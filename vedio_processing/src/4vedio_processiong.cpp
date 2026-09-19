
// 4vedio_processiong.cpp
/* 编译命令:
g++ -std=c++17 -O2 -o stream_gst 4vedio_processiong.cpp \
    $(pkg-config --cflags gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0 opencv4) \
    -I/usr/local/include \
    -I/home/cat/Orbbec/include/libobsensor/h \
    $(pkg-config --libs gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0 opencv4) \
    -lOrbbecSDK -lpthread -ldl -lrt -lm */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <ObSensor.hpp>
#include <ObTypes.h>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/video/video.h>
#include <opencv2/opencv.hpp>
#include <signal.h>
#include <iostream>
#include <memory>
#include <cstring>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <algorithm>
#include <ctime>
#include <fstream>
#include <mutex>
#include <iomanip>
#include <sstream>
#include <fcntl.h>

// ===== OBFormat 枚举转字符串名称（来自 ObTypes.h）=====
static const char* formatName(int format) {
    switch (format) {
        case -1:  return "UNKNOWN";
        case  0:  return "YUYV";
        case  1:  return "YUY2";
        case  2:  return "UYVY";
        case  3:  return "NV12";
        case  4:  return "NV21";
        case  5:  return "MJPG";
        case  6:  return "H264";
        case  7:  return "H265";
        case  8:  return "Y16";
        case  9:  return "Y8";
        case 10:  return "Y10";
        case 11:  return "Y11";
        case 12:  return "Y12";
        case 13:  return "GRAY";
        case 14:  return "HEVC";
        case 15:  return "I420";
        case 16:  return "ACCEL";
        case 17:  return "GYRO";
        case 19:  return "POINT";
        case 20:  return "RGB_POINT";
        case 21:  return "RLE";
        case 22:  return "RGB";
        case 23:  return "BGR";
        case 24:  return "Y14";
        case 25:  return "BGRA";
        case 26:  return "COMPRESSED";
        case 27:  return "RVL";
        case 28:  return "Z16";
        case 29:  return "YV12";
        case 30:  return "BA81";
        case 31:  return "RGBA";
        case 32:  return "BYR2";
        case 33:  return "RW16";
        case 34:  return "Y12C4";
        default:  return "UNKNOWN";
    }
}

// ===== OBStreamType 枚举转字符串名称 =====
static const char* streamTypeName(int type) {
    switch (type) {
        case -1: return "UNKNOWN";
        case  0: return "VIDEO";
        case  1: return "IR";
        case  2: return "COLOR";
        case  3: return "DEPTH";
        case  4: return "ACCEL";
        case  5: return "GYRO";
        case  6: return "IR_LEFT";
        case  7: return "IR_RIGHT";
        case  8: return "RAW_PHASE";
        case  9: return "CONFIDENCE";
        default: return "UNKNOWN";
    }
}

// ===== OBSensorType 枚举转字符串名称 =====
static const char* sensorTypeName(int type) {
    switch (type) {
        case  0: return "UNKNOWN";
        case  1: return "IR";
        case  2: return "COLOR";
        case  3: return "DEPTH";
        case  4: return "ACCEL";
        case  5: return "GYRO";
        case  6: return "IR_LEFT";
        case  7: return "IR_RIGHT";
        case  8: return "RAW_PHASE";
        case  9: return "CONFIDENCE";
        default: return "UNKNOWN";
    }
}


static volatile sig_atomic_t g_running = 1;

// ===== 全局日志 =====
static std::ofstream g_ts_log;
static std::mutex g_ts_log_mutex;
static uint64_t g_start_time_ns = 0;

// ===== UDP 时间戳全局变量 =====
static int g_ts_sock = -1;
static struct sockaddr_in g_ts_addr;
static std::mutex g_ts_sock_mutex;

// ===== UDP 数据包结构 =====
struct TimestampPacket {
    uint64_t pts_ns;        // Unix 时间戳（纳秒）
    uint64_t send_time_ns;  // Unix 时间戳（纳秒）
    char id[8];
};

void signalHandler(int sig) {
    (void)sig;
    g_running = 0;
}

// ===== 初始化时间戳 UDP socket =====
void init_timestamp_socket(const char* ip) {
    g_ts_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_ts_sock < 0) {
        std::cerr << "[TS] Failed to create UDP socket" << std::endl;
        return;
    }
    memset(&g_ts_addr, 0, sizeof(g_ts_addr));
    g_ts_addr.sin_family = AF_INET;
    g_ts_addr.sin_port = htons(6001);
    if (inet_pton(AF_INET, ip, &g_ts_addr.sin_addr) <= 0) {
        std::cerr << "[TS] Invalid IP: " << ip << std::endl;
        close(g_ts_sock);
        g_ts_sock = -1;
        return;
    }
    std::cout << "[TS] Time sync target: " << ip << ":6001" << std::endl;
}

// ===== 发送时间戳（线程安全）=====
void send_timestamp_udp(uint64_t pts_ns, uint64_t send_time_ns, const char* name) {
    std::lock_guard<std::mutex> lock(g_ts_sock_mutex);
    if (g_ts_sock < 0) return;
    TimestampPacket pkt;
    pkt.pts_ns = pts_ns;
    pkt.send_time_ns = send_time_ns;
    memset(pkt.id, 0, 8);
    strncpy(pkt.id, name, 7);
    sendto(g_ts_sock, &pkt, sizeof(pkt), 0,
           (struct sockaddr*)&g_ts_addr, sizeof(g_ts_addr));
}

// ===== 时间戳转人类可读字符串 =====
std::string timestampToHuman(uint64_t timestamp_ns) {
    time_t sec = timestamp_ns / 1000000000ULL;
    uint64_t nsec = timestamp_ns % 1000000000ULL;
    struct tm* tm_info = localtime(&sec);
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
    char result[80];
    snprintf(result, sizeof(result), "%s.%09lu", buffer, nsec);
    return std::string(result);
}

// ===== 记录时间戳到日志 =====
void logTimestamp(const char* stage, const char* stream, uint64_t timestamp_ns, const char* extra = "") {
    std::lock_guard<std::mutex> lock(g_ts_log_mutex);
    if (g_ts_log.is_open()) {
        uint64_t relative_ns = (g_start_time_ns > 0) ? (timestamp_ns - g_start_time_ns) : 0;
        g_ts_log << stage << "," << stream << "," 
                 << timestamp_ns << "," << relative_ns << ","
                 << timestampToHuman(timestamp_ns) << "," << extra << "\n";
        g_ts_log.flush();
    }
}

// =============================================
// ===== GStreamer Pad Probe 用于 v4l2 流时间戳 =====
// =============================================

struct V4l2StreamData {
    std::string name;
    uint64_t last_pts_ns;
    int frame_count;
    uint64_t base_offset_ns;   // GStreamer PTS -> Unix 时间戳的偏移
    bool base_offset_set;
};

static V4l2StreamData g_v4l2_data[2] = {
    {"cam1", 0, 0, 0, false},
    {"cam2", 0, 0, 0, false}
};

// Pad Probe 回调函数
static GstPadProbeReturn v4l2_probe_cb(GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {
    (void)pad;
    V4l2StreamData* data = static_cast<V4l2StreamData*>(user_data);
    if (!data) return GST_PAD_PROBE_OK;
    
    GstBuffer *buffer = GST_PAD_PROBE_INFO_BUFFER(info);
    if (!buffer) return GST_PAD_PROBE_OK;
    
    // 获取 GStreamer PTS（纳秒，从系统启动开始）
    uint64_t gst_pts_ns = GST_BUFFER_PTS(buffer);
    if (gst_pts_ns == GST_CLOCK_TIME_NONE) {
        return GST_PAD_PROBE_OK;
    }
    
    // 获取当前系统时间（Unix 时间戳）
    uint64_t now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                         std::chrono::system_clock::now().time_since_epoch()).count();
    
    // ===== 建立基准转换：GStreamer PTS -> Unix 时间戳 =====
    if (!data->base_offset_set) {
        data->base_offset_ns = now_ns - gst_pts_ns;
        data->base_offset_set = true;
        std::cout << "[TS] " << data->name << " base offset: " << data->base_offset_ns << " ns" << std::endl;
    }
    
    // 将 GStreamer PTS 转换为 Unix 时间戳
    uint64_t unix_pts_ns = gst_pts_ns + data->base_offset_ns;
    
    // 去重
    if (unix_pts_ns == data->last_pts_ns) {
        return GST_PAD_PROBE_OK;
    }
    data->last_pts_ns = unix_pts_ns;
    
    // 发送时间戳 UDP
    send_timestamp_udp(unix_pts_ns, now_ns, data->name.c_str());
    
    data->frame_count++;
    if (data->frame_count % 30 == 0) {
        std::cout << "[TS] " << data->name << ": PTS=" << timestampToHuman(unix_pts_ns) 
                  << ", send=" << timestampToHuman(now_ns) << std::endl;
    }
    
    if (data->frame_count % 10 == 0) {
        logTimestamp("sender", data->name.c_str(), unix_pts_ns, "v4l2_probe");
    }
    
    return GST_PAD_PROBE_OK;
}

// ===== 为 v4l2 流添加 probe =====
void attach_v4l2_probe(GstElement *pipeline, const char* v4l2_src_name, V4l2StreamData* data) {
    GstElement *v4l2src = gst_bin_get_by_name(GST_BIN(pipeline), v4l2_src_name);
    if (!v4l2src) {
        std::cerr << "[TS] Warning: Cannot find " << v4l2_src_name << std::endl;
        return;
    }
    
    GstPad *src_pad = gst_element_get_static_pad(v4l2src, "src");
    if (!src_pad) {
        std::cerr << "[TS] Warning: Cannot get src pad from " << v4l2_src_name << std::endl;
        gst_object_unref(v4l2src);
        return;
    }
    
    gst_pad_add_probe(src_pad, GST_PAD_PROBE_TYPE_BUFFER, v4l2_probe_cb, data, NULL);
    
    std::cout << "[TS] Attached probe to " << v4l2_src_name << std::endl;
    gst_object_unref(src_pad);
    gst_object_unref(v4l2src);
}

// =============================================
// ===== 基础格式转换辅助函数 =====
// =============================================

void convertY16ToYUV420SP(const uint16_t *depth_data, uint8_t *yuv_data, int width, int height, uint16_t max_depth = 4096) {
    int y_size = width * height;
    for (int i = 0; i < y_size; ++i) {
        uint16_t d = depth_data[i];
        yuv_data[i] = (d == 0 || d > max_depth) ? 0 : static_cast<uint8_t>(255 - (d * 255 / max_depth));
    }
    memset(yuv_data + y_size, 128, y_size / 2);
}

void convertYUYVToNV12(const uint8_t *yuyv, uint8_t *nv12, int width, int height) {
    uint8_t *y = nv12;
    uint8_t *uv = nv12 + width * height;
    for (int i = 0; i < height; ++i) {
        const uint8_t *src = yuyv + i * width * 2;
        uint8_t *y_row = y + i * width;
        uint8_t *uv_row = (i % 2 == 0) ? uv + (i / 2) * width : nullptr;
        for (int j = 0; j < width; j += 2) {
            uint8_t y0 = src[j * 2];
            uint8_t u = src[j * 2 + 1];
            uint8_t y1 = (j + 1 < width) ? src[j * 2 + 2] : y0;
            uint8_t v = src[j * 2 + 3];
            y_row[j] = y0;
            if (j + 1 < width) y_row[j + 1] = y1;
            if (uv_row) {
                uv_row[j] = u;
                uv_row[j + 1] = v;
            }
        }
    }
}

void convertI420ToNV12(const uint8_t* i420_data, uint8_t* nv12_data, int width, int height) {
    int y_size = width * height;
    int uv_size = y_size / 2;
    memcpy(nv12_data, i420_data, y_size);
    const uint8_t* src_u = i420_data + y_size;
    const uint8_t* src_v = src_u + uv_size / 2;
    uint8_t* dst_uv = nv12_data + y_size;
    for (int i = 0; i < uv_size / 2; ++i) {
        dst_uv[i * 2] = src_u[i];
        dst_uv[i * 2 + 1] = src_v[i];
    }
}

void convertUYVYToNV12(const uint8_t *uyvy, uint8_t *nv12, int width, int height) {
    uint8_t *y = nv12;
    uint8_t *uv = nv12 + width * height;
    for (int i = 0; i < height; ++i) {
        const uint8_t *src = uyvy + i * width * 2;
        uint8_t *y_row = y + i * width;
        uint8_t *uv_row = (i % 2 == 0) ? uv + (i / 2) * width : nullptr;
        for (int j = 0; j < width; j += 2) {
            uint8_t u = src[j * 2];
            uint8_t y0 = src[j * 2 + 1];
            uint8_t v = src[j * 2 + 2];
            uint8_t y1 = src[j * 2 + 3];
            y_row[j] = y0;
            if (j + 1 < width) y_row[j + 1] = y1;
            if (uv_row) {
                uv_row[j] = u;
                uv_row[j + 1] = v;
            }
        }
    }
}

// ===== 统一格式转 NV12 =====
bool convertFrameToNV12(const std::shared_ptr<ob::Frame>& frame, std::vector<uint8_t>& nv12_buf, int width, int height) {
    auto format = frame->format();
    const uint8_t* data = static_cast<const uint8_t*>(frame->data());
    size_t dataSize = frame->dataSize();
    int y_size = width * height;
    int uv_size = y_size / 2;
    int total_size = y_size + uv_size;

    if (nv12_buf.size() != static_cast<size_t>(total_size)) {
        nv12_buf.resize(total_size);
    }

    switch (format) {
        case OB_FORMAT_NV12:
            if (dataSize >= static_cast<size_t>(total_size)) {
                memcpy(nv12_buf.data(), data, total_size);
                return true;
            }
            break;
        case OB_FORMAT_YUYV:
        case OB_FORMAT_YUY2:
            convertYUYVToNV12(data, nv12_buf.data(), width, height);
            return true;
        case OB_FORMAT_UYVY:
            convertUYVYToNV12(data, nv12_buf.data(), width, height);
            return true;
        case OB_FORMAT_NV21: {
            memcpy(nv12_buf.data(), data, y_size);
            const uint8_t* src_uv = data + y_size;
            uint8_t* dst_uv = nv12_buf.data() + y_size;
            for (int i = 0; i < uv_size; i += 2) {
                dst_uv[i] = src_uv[i + 1];
                dst_uv[i + 1] = src_uv[i];
            }
            return true;
        }
        case OB_FORMAT_I420:
            convertI420ToNV12(data, nv12_buf.data(), width, height);
            return true;
        case OB_FORMAT_YV12: {
            memcpy(nv12_buf.data(), data, y_size);
            const uint8_t* src_v = data + y_size;
            const uint8_t* src_u = src_v + uv_size / 2;
            uint8_t* dst_uv = nv12_buf.data() + y_size;
            for (int i = 0; i < uv_size / 2; ++i) {
                dst_uv[i * 2] = src_u[i];
                dst_uv[i * 2 + 1] = src_v[i];
            }
            return true;
        }
        case OB_FORMAT_Y8:
        case OB_FORMAT_GRAY:
        case OB_FORMAT_BA81: 
            memcpy(nv12_buf.data(), data, y_size);
            memset(nv12_buf.data() + y_size, 128, uv_size);
            return true;
        case OB_FORMAT_Y16:
        case OB_FORMAT_Z16:
            convertY16ToYUV420SP(reinterpret_cast<const uint16_t*>(data), nv12_buf.data(), width, height);
            return true;
        case OB_FORMAT_RGB: {
            cv::Mat src(height, width, CV_8UC3, (void*)data);
            cv::Mat i420_mat;
            cv::cvtColor(src, i420_mat, cv::COLOR_RGB2YUV_I420);
            convertI420ToNV12(i420_mat.data, nv12_buf.data(), width, height);
            return true;
        }
        case OB_FORMAT_BGR: {
            cv::Mat src(height, width, CV_8UC3, (void*)data);
            cv::Mat i420_mat;
            cv::cvtColor(src, i420_mat, cv::COLOR_BGR2YUV_I420);
            convertI420ToNV12(i420_mat.data, nv12_buf.data(), width, height);
            return true;
        }
        default:
            std::cerr << "[Warning] Unsupported format (" << format << ")." << std::endl;
            return false;
    }
    return false;
}

// ===== 推送帧 =====
bool pushFrame(GstElement *appsrc, void *data, size_t size, uint64_t capture_time_ns) {
    GstBuffer *buffer = gst_buffer_new_allocate(nullptr, size, nullptr);
    gst_buffer_fill(buffer, 0, data, size);
    GST_BUFFER_PTS(buffer) = capture_time_ns;
    GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale(1, GST_SECOND, 30);
    GstFlowReturn ret;
    g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);
    gst_buffer_unref(buffer);
    return (ret == GST_FLOW_OK);
}

// =============================================
// ===== main =====
// =============================================

int main(int argc, char *argv[]) {
    signal(SIGINT, signalHandler);
    gst_init(&argc, &argv);

    // ===== 初始化时间戳日志 =====
    g_ts_log.open("timestamp_sender.csv", std::ios::out | std::ios::trunc);
    if (g_ts_log.is_open()) {
        g_ts_log << "stage,stream,timestamp_ns,relative_ns,timestamp_human,extra\n";
        std::cout << "[TS] Sender log: timestamp_sender.csv" << std::endl;
    }

    g_start_time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                         std::chrono::system_clock::now().time_since_epoch()).count();

    const char* RECEIVER_IP = "192.168.7.2"; 
    init_timestamp_socket(RECEIVER_IP);

    const int DEPTH_WIDTH = 1280;
    const int DEPTH_HEIGHT = 720;
    int color_width = 1280;
    int color_height = 720;

    GstElement *pipeline = nullptr;
    GstElement *depth_appsrc = nullptr;
    GstElement *color_appsrc = nullptr;

    try {
        ob::Context ctx;
        auto deviceList = ctx.queryDeviceList();
        if (deviceList->deviceCount() == 0) {
            std::cerr << "No Orbbec device found!" << std::endl;
            return -1;
        }

        auto device = deviceList->getDevice(0);
        std::cout << "Using Orbbec: " << device->getDeviceInfo()->name() << std::endl;

        auto pipeline_ob = std::make_shared<ob::Pipeline>(device);
        
        auto depthProfiles = pipeline_ob->getStreamProfileList(OB_SENSOR_DEPTH);

        // ===== 打印深度相机所有可用配置（格式名称） =====
        std::cout << "\n=== Depth Camera Available Profiles ===" << std::endl;
        for (uint32_t i = 0; i < depthProfiles->count(); ++i) {
            auto profile = depthProfiles->getProfile(i);
            auto videoProfile = profile->as<ob::VideoStreamProfile>();
            if (videoProfile) {
                std::cout << "  [" << i << "] "
                          << videoProfile->width() << "x" << videoProfile->height()
                          << " @ " << videoProfile->fps() << "fps"
                          << " Format: " << formatName(videoProfile->format())
                          << std::endl;
            } else {
                std::cout << "  [" << i << "] (not a video profile)" << std::endl;
            }
        }
        std::cout << "==========================================\n" << std::endl;

        auto depthProfile = depthProfiles->getVideoStreamProfile(DEPTH_WIDTH, DEPTH_HEIGHT, OB_FORMAT_Y16, 30);
        if (!depthProfile) {
            depthProfile = depthProfiles->getVideoStreamProfile(DEPTH_WIDTH, DEPTH_HEIGHT, OB_FORMAT_ANY, 30);
        }
        if (!depthProfile) {
             if (depthProfiles->count() > 0) {
                 depthProfile = depthProfiles->getProfile(0)->as<ob::VideoStreamProfile>();
             } else {
                 std::cerr << "No depth profiles available!" << std::endl;
                 return -1;
             }
        }
        std::cout << "Selected Depth: " << depthProfile->width() << "x" << depthProfile->height() 
                  << " Format: " << formatName(depthProfile->format()) << std::endl;

        auto colorProfiles = pipeline_ob->getStreamProfileList(OB_SENSOR_COLOR);

        // ===== 打印彩色相机所有可用配置（格式名称） =====
        std::cout << "\n=== Color Camera Available Profiles ===" << std::endl;
        for (uint32_t i = 0; i < colorProfiles->count(); ++i) {
            auto profile = colorProfiles->getProfile(i);
            auto videoProfile = profile->as<ob::VideoStreamProfile>();
            if (videoProfile) {
                std::cout << "  [" << i << "] "
                          << videoProfile->width() << "x" << videoProfile->height()
                          << " @ " << videoProfile->fps() << "fps"
                          << " Format: " << formatName(videoProfile->format())
                          << std::endl;
            } else {
                std::cout << "  [" << i << "] (not a video profile)" << std::endl;
            }
        }
        std::cout << "==========================================\n" << std::endl;

        std::shared_ptr<ob::VideoStreamProfile> colorProfile = nullptr;
        
        std::vector<std::pair<int, int>> preferred_resolutions = {
            {1920, 1080}, {1280, 800}, {1280, 720}, {640, 480}
        };

        for (const auto& res : preferred_resolutions) {
            for (size_t i = 0; i < colorProfiles->count(); ++i) {
                auto p = colorProfiles->getProfile(i)->as<ob::VideoStreamProfile>();
                if (p->width() == static_cast<uint32_t>(res.first) && p->height() == static_cast<uint32_t>(res.second) && p->fps() == 30) {
                    if (p->format() == OB_FORMAT_YUYV || p->format() == OB_FORMAT_NV12 || p->format() == OB_FORMAT_UYVY) {
                        colorProfile = p;
                        color_width = res.first;
                        color_height = res.second;
                        break;
                    }
                }
            }
            if (colorProfile) break;
        }

        if (!colorProfile) {
            for (size_t i = 0; i < colorProfiles->count(); ++i) {
                auto p = colorProfiles->getProfile(i)->as<ob::VideoStreamProfile>();
                if (p->fps() == 30) {
                    colorProfile = p;
                    color_width = p->width();
                    color_height = p->height();
                    break;
                }
            }
        }

        if (!colorProfile) {
            std::cerr << "No suitable Color profile found!" << std::endl;
            return -1;
        }

        std::cout << "Selected Color: " << colorProfile->width() << "x" << colorProfile->height() 
                  << " Format: " << formatName(colorProfile->format()) << " @ " << colorProfile->fps() << " FPS" << std::endl;

        // ===== 构建 GStreamer Pipeline =====
        std::string depth_caps = "video/x-raw,format=Y16,width=" + std::to_string(depthProfile->width()) + 
                                 ",height=" + std::to_string(depthProfile->height()) + ",framerate=30/1";
        // 获取彩色流实际格式名称用于caps
        std::string color_format_str;
        switch (colorProfile->format()) {
            case OB_FORMAT_YUYV:
            case OB_FORMAT_YUY2: color_format_str = "YUYV"; break;
            case OB_FORMAT_NV12: color_format_str = "NV12"; break;
            case OB_FORMAT_UYVY: color_format_str = "UYVY"; break;
            default: color_format_str = "NV12"; break;
        }
        std::cout << "Color actual format: " << formatName(colorProfile->format()) << std::endl;

        std::string color_caps = "video/x-raw,format=" + color_format_str + ",width=" + std::to_string(color_width) + 
                                 ",height=" + std::to_string(color_height) + ",framerate=30/1";

        std::string pipeline_str =
            "appsrc name=depth_src is-live=true format=time ! " +
            depth_caps + " ! queue ! videoconvert ! video/x-raw,format=NV12 ! mpph264enc b-frames=0 ! " +
            "rtph264pay config-interval=1 pt=96 ! udpsink host=" + std::string(RECEIVER_IP) + " port=9000 sync=false async=false " +
            
            "appsrc name=color_src is-live=true format=time ! " +
            color_caps + " ! queue ! videoconvert ! video/x-raw,format=NV12 ! mpph264enc b-frames=0 ! " +
            "rtph264pay config-interval=1 pt=96 ! udpsink host=" + std::string(RECEIVER_IP) + " port=9001 sync=false async=false ";

        // v4l2 流 1
        if (access("/dev/video22", F_OK) == 0) {
            pipeline_str +=
                "v4l2src name=v4l2_src1 device=/dev/video22 do-timestamp=true ! "
                "video/x-raw,format=NV12,width=1280,height=720,framerate=30/1 ! "
                "queue ! mpph264enc b-frames=0 ! "
                "rtph264pay config-interval=1 pt=97 ! udpsink host=" + std::string(RECEIVER_IP) + " port=9002 sync=false async=false ";
        }

        // v4l2 流 2
        if (access("/dev/video31", F_OK) == 0) {
            pipeline_str +=
                "v4l2src name=v4l2_src2 device=/dev/video31 do-timestamp=true ! "
                "video/x-raw,format=NV12,width=1280,height=720,framerate=30/1 ! "
                "queue ! mpph264enc b-frames=0 ! "
                "rtph264pay config-interval=1 pt=98 ! udpsink host=" + std::string(RECEIVER_IP) + " port=9003 sync=false async=false ";
        }

        std::cout << "Pipeline:\n" << pipeline_str << "\n";

        GError *error = nullptr;
        pipeline = gst_parse_launch(pipeline_str.c_str(), &error);
        if (!pipeline) {
            std::cerr << "GStreamer error: " << (error ? error->message : "unknown") << std::endl;
            if (error) g_error_free(error);
            return -1;
        }

        // ===== 为 v4l2 流添加 Pad Probe =====
        if (access("/dev/video22", F_OK) == 0) {
            attach_v4l2_probe(pipeline, "v4l2_src1", &g_v4l2_data[0]);
        }
        if (access("/dev/video31", F_OK) == 0) {
            attach_v4l2_probe(pipeline, "v4l2_src2", &g_v4l2_data[1]);
        }

        depth_appsrc = gst_bin_get_by_name(GST_BIN(pipeline), "depth_src");
        color_appsrc = gst_bin_get_by_name(GST_BIN(pipeline), "color_src");

        gst_element_set_state(pipeline, GST_STATE_PLAYING);

        auto config = std::make_shared<ob::Config>();
        config->enableStream(depthProfile);
        config->enableStream(colorProfile);

        pipeline_ob->start(config);
        
        std::cout << "\n=== Streaming Started ===" << std::endl;
        std::cout << "  Depth:   port 9000 (timestamp sent)" << std::endl;
        std::cout << "  Color:   port 9001 (timestamp sent)" << std::endl;
        std::cout << "  Cam1:    port 9002 (timestamp sent via probe)" << std::endl;
        std::cout << "  Cam2:    port 9003 (timestamp sent via probe)" << std::endl;
        std::cout << "  UDP timestamp: " << RECEIVER_IP << ":6001 (PTS + send_time)" << std::endl;
        std::cout << "  Press Ctrl+C to stop" << std::endl;
        std::cout << "=========================\n" << std::endl;



        int depth_frame_count = 0;
        int color_frame_count = 0;

        while (g_running) {
            auto frameSet = pipeline_ob->waitForFrames(100);
            if (!frameSet) continue;

            // 使用系统时间（Unix 时间戳）
            uint64_t capture_time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                           std::chrono::system_clock::now().time_since_epoch()).count();

            auto depthFrame = frameSet->depthFrame();
            if (depthFrame) {
                depth_frame_count++;
                // 直接推送Y16原始帧，不做CPU格式转换，由videoconvert硬件转NV12
                uint64_t send_time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                            std::chrono::system_clock::now().time_since_epoch()).count();
                if (pushFrame(depth_appsrc, depthFrame->data(), depthFrame->dataSize(), capture_time_ns)) {
                    send_timestamp_udp(capture_time_ns, send_time_ns, "depth");
                    static int depth_print = 0;
                    if (++depth_print % 30 == 0) {
                        std::cout << "[Sender] depth: PTS=" << timestampToHuman(capture_time_ns) 
                                  << ", send=" << timestampToHuman(send_time_ns) << std::endl;
                    }
                }
            }

            auto colorFrame = frameSet->colorFrame();
            if (colorFrame) {
                color_frame_count++;
                // 直接推送原始帧(YUYV/NV12)，不做CPU格式转换，由videoconvert硬件转NV12
                uint64_t send_time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                            std::chrono::system_clock::now().time_since_epoch()).count();
                if (pushFrame(color_appsrc, colorFrame->data(), colorFrame->dataSize(), capture_time_ns)) {
                    send_timestamp_udp(capture_time_ns, send_time_ns, "color");
                    static int color_print = 0;
                    if (++color_print % 30 == 0) {
                        std::cout << "[Sender] color: PTS=" << timestampToHuman(capture_time_ns) 
                                  << ", send=" << timestampToHuman(send_time_ns) << std::endl;
                    }
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        pipeline_ob->stop();
    }
    catch (const ob::Error &e) {
        std::cerr << "Orbbec Error: " << e.getMessage() << std::endl;
    }
    catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    // ===== 清理 =====
    if (g_ts_sock >= 0) close(g_ts_sock);
    
    if (g_ts_log.is_open()) {
        g_ts_log.close();
        std::cout << "\n[TS] Log saved to timestamp_sender.csv" << std::endl;
    }

    if (pipeline) {
        gst_element_send_event(pipeline, gst_event_new_eos());
        GstBus *bus = gst_element_get_bus(pipeline);
        gst_bus_timed_pop_filtered(bus, 5 * GST_SECOND, (GstMessageType)(GST_MESSAGE_EOS | GST_MESSAGE_ERROR));
        gst_object_unref(bus);
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
    }

    std::cout << "All streams stopped." << std::endl;
    return 0;
}