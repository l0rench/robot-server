// // //
// // // The MIT License (MIT)
// // //
// // // Copyright (c) 2022 Livox. All rights reserved.
// // //
// // // Permission is hereby granted, free of charge, to any person obtaining a copy
// // // of this software and associated documentation files (the "Software"), to deal
// // // in the Software without restriction, including without limitation the rights
// // // to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// // // copies of the Software, and to permit persons to whom the Software is
// // // furnished to do so, subject to the following conditions:
// // //
// // // The above copyright notice and this permission notice shall be included in
// // // all copies or substantial portions of the Software.
// // //
// // // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// // // IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// // // FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// // // AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// // // LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// // // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// // // SOFTWARE.
// // //

// // #include "livox_lidar_def.h"
// // #include "livox_lidar_api.h"

// // #ifdef _WIN32
// // #include <winsock2.h>
// // #else
// // #include <unistd.h>
// // #include <arpa/inet.h>
// // #endif

// // #include <stdio.h>
// // #include <stdlib.h>
// // #include <string.h>
// // #include <thread>
// // #include <chrono>
// // #include <iostream>

// // void PointCloudCallback(uint32_t handle, const uint8_t dev_type, LivoxLidarEthernetPacket* data, void* client_data) {
// //   if (data == nullptr) {
// //     return;
// //   }
// //   printf("point cloud handle: %u, data_num: %d, data_type: %d, length: %d, frame_counter: %d\n",
// //       handle, data->dot_num, data->data_type, data->length, data->frame_cnt);

// //   if (data->data_type == kLivoxLidarCartesianCoordinateHighData) {
// //     LivoxLidarCartesianHighRawPoint *p_point_data = (LivoxLidarCartesianHighRawPoint *)data->data;
// //     for (uint32_t i = 0; i < data->dot_num; i++) {
// //       //p_point_data[i].x;
// //       //p_point_data[i].y;
// //       //p_point_data[i].z;
// //     }
// //   }
// //   else if (data->data_type == kLivoxLidarCartesianCoordinateLowData) {
// //     LivoxLidarCartesianLowRawPoint *p_point_data = (LivoxLidarCartesianLowRawPoint *)data->data;
// //   } else if (data->data_type == kLivoxLidarSphericalCoordinateData) {
// //     LivoxLidarSpherPoint* p_point_data = (LivoxLidarSpherPoint *)data->data;
// //   }
// // }

// // void ImuDataCallback(uint32_t handle, const uint8_t dev_type,  LivoxLidarEthernetPacket* data, void* client_data) {
// //   if (data == nullptr) {
// //     return;
// //   } 
// //   printf("Imu data callback handle:%u, data_num:%u, data_type:%u, length:%u, frame_counter:%u.\n",
// //       handle, data->dot_num, data->data_type, data->length, data->frame_cnt);
// // }

// // // void OnLidarSetIpCallback(livox_vehicle_status status, uint32_t handle, uint8_t ret_code, void*) {
// // //   if (status == kVehicleStatusSuccess) {
// // //     printf("lidar set ip slot: %d, ret_code: %d\n",
// // //       slot, ret_code);
// // //   } else if (status == kVehicleStatusTimeout) {
// // //     printf("lidar set ip number timeout\n");
// // //   }
// // // }

// // void WorkModeCallback(livox_status status, uint32_t handle,LivoxLidarAsyncControlResponse *response, void *client_data) {
// //   if (response == nullptr) {
// //     return;
// //   }
// //   printf("WorkModeCallack, status:%u, handle:%u, ret_code:%u, error_key:%u",
// //       status, handle, response->ret_code, response->error_key);

// // }

// // void RebootCallback(livox_status status, uint32_t handle, LivoxLidarRebootResponse* response, void* client_data) {
// //   if (response == nullptr) {
// //     return;
// //   }
// //   printf("RebootCallback, status:%u, handle:%u, ret_code:%u",
// //       status, handle, response->ret_code);
// // }

// // void SetIpInfoCallback(livox_status status, uint32_t handle, LivoxLidarAsyncControlResponse *response, void *client_data) {
// //   if (response == nullptr) {
// //     return;
// //   }
// //   printf("LivoxLidarIpInfoCallback, status:%u, handle:%u, ret_code:%u, error_key:%u",
// //       status, handle, response->ret_code, response->error_key);

// //   if (response->ret_code == 0 && response->error_key == 0) {
// //     LivoxLidarRequestReboot(handle, RebootCallback, nullptr);
// //   }
// // }

// // void QueryInternalInfoCallback(livox_status status, uint32_t handle, 
// //     LivoxLidarDiagInternalInfoResponse* response, void* client_data) {
// //   if (status != kLivoxLidarStatusSuccess) {
// //     printf("Query lidar internal info failed.\n");
// //     QueryLivoxLidarInternalInfo(handle, QueryInternalInfoCallback, nullptr);
// //     return;
// //   }

// //   if (response == nullptr) {
// //     return;
// //   }

// //   uint8_t host_point_ipaddr[4] {0};
// //   uint16_t host_point_port = 0;
// //   uint16_t lidar_point_port = 0;

// //   uint8_t host_imu_ipaddr[4] {0};
// //   uint16_t host_imu_data_port = 0;
// //   uint16_t lidar_imu_data_port = 0;

// //   uint16_t off = 0;
// //   for (uint8_t i = 0; i < response->param_num; ++i) {
// //     LivoxLidarKeyValueParam* kv = (LivoxLidarKeyValueParam*)&response->data[off];
// //     if (kv->key == kKeyLidarPointDataHostIpCfg) {
// //       memcpy(host_point_ipaddr, &(kv->value[0]), sizeof(uint8_t) * 4);
// //       memcpy(&(host_point_port), &(kv->value[4]), sizeof(uint16_t));
// //       memcpy(&(lidar_point_port), &(kv->value[6]), sizeof(uint16_t));
// //     } else if (kv->key == kKeyLidarImuHostIpCfg) {
// //       memcpy(host_imu_ipaddr, &(kv->value[0]), sizeof(uint8_t) * 4);
// //       memcpy(&(host_imu_data_port), &(kv->value[4]), sizeof(uint16_t));
// //       memcpy(&(lidar_imu_data_port), &(kv->value[6]), sizeof(uint16_t));
// //     }
// //     off += sizeof(uint16_t) * 2;
// //     off += kv->length;
// //   }

// //   printf("Host point cloud ip addr:%u.%u.%u.%u, host point cloud port:%u, lidar point cloud port:%u.\n",
// //       host_point_ipaddr[0], host_point_ipaddr[1], host_point_ipaddr[2], host_point_ipaddr[3], host_point_port, lidar_point_port);

// //   printf("Host imu ip addr:%u.%u.%u.%u, host imu port:%u, lidar imu port:%u.\n",
// //     host_imu_ipaddr[0], host_imu_ipaddr[1], host_imu_ipaddr[2], host_imu_ipaddr[3], host_imu_data_port, lidar_imu_data_port);

// // }

// // void LidarInfoChangeCallback(const uint32_t handle, const LivoxLidarInfo* info, void* client_data) {
// //   if (info == nullptr) {
// //     printf("lidar info change callback failed, the info is nullptr.\n");
// //     return;
// //   } 
// //   printf("LidarInfoChangeCallback Lidar handle: %u SN: %s\n", handle, info->sn);
  
// //   // set the work mode to kLivoxLidarNormal, namely start the lidar
// //   SetLivoxLidarWorkMode(handle, kLivoxLidarNormal, WorkModeCallback, nullptr);

// //   QueryLivoxLidarInternalInfo(handle, QueryInternalInfoCallback, nullptr);

// //   // LivoxLidarIpInfo lidar_ip_info;
// //   // strcpy(lidar_ip_info.ip_addr, "192.168.1.10");
// //   // strcpy(lidar_ip_info.net_mask, "255.255.255.0");
// //   // strcpy(lidar_ip_info.gw_addr, "192.168.1.1");
// //   // SetLivoxLidarLidarIp(handle, &lidar_ip_info, SetIpInfoCallback, nullptr);
// // }

// // void LivoxLidarPushMsgCallback(const uint32_t handle, const uint8_t dev_type, const char* info, void* client_data) {
// //   struct in_addr tmp_addr;
// //   tmp_addr.s_addr = handle;  
// //   std::cout << "handle: " << handle << ", ip: " << inet_ntoa(tmp_addr) << ", push msg info: " << std::endl;
// //   std::cout << info << std::endl;
// //   return;
// // }

// // int main(int argc, const char *argv[]) {
// //   if (argc != 2) {
// //     printf("Params Invalid, must input config path.\n");
// //     return -1;
// //   }
// //   const std::string path = argv[1];

// //   // REQUIRED, to init Livox SDK2
// //   if (!LivoxLidarSdkInit(path.c_str())) {
// //     printf("Livox Init Failed\n");
// //     LivoxLidarSdkUninit();
// //     return -1;
// //   }
  
// //   // REQUIRED, to get point cloud data via 'PointCloudCallback'
// //   SetLivoxLidarPointCloudCallBack(PointCloudCallback, nullptr);
  
// //   // OPTIONAL, to get imu data via 'ImuDataCallback'
// //   // some lidar types DO NOT contain an imu component
// //   SetLivoxLidarImuDataCallback(ImuDataCallback, nullptr);
  
// //   SetLivoxLidarInfoCallback(LivoxLidarPushMsgCallback, nullptr);
  
// //   // REQUIRED, to get a handle to targeted lidar and set its work mode to NORMAL
// //   SetLivoxLidarInfoChangeCallback(LidarInfoChangeCallback, nullptr);

// // #ifdef WIN32
// //   Sleep(300000);
// // #else
// //   sleep(300);
// // #endif
// //   LivoxLidarSdkUninit();
// //   printf("Livox Quick Start Demo End!\n");
// //   return 0;
// // }



// //
// // The MIT License (MIT)
// //
// // Copyright (c) 2022 Livox. All rights reserved.
// //
// // Permission is hereby granted, free of charge, to any person obtaining a copy
// // of this software and associated documentation files (the "Software"), to deal
// // in the Software without restriction, including without limitation the rights
// // to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// // copies of the Software, and to permit persons to whom the Software is
// // furnished to do so, subject to the following conditions:
// //
// // The above copyright notice and this permission notice shall be included in
// // all copies or substantial portions of the Software.
// //
// // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// // IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// // FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// // AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// // LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// // SOFTWARE.
// //

// #include "livox_lidar_def.h"
// #include "livox_lidar_api.h"

// #ifdef _WIN32
// #include <winsock2.h>
// #else
// #include <unistd.h>
// #include <arpa/inet.h>
// #include <sys/time.h>
// #endif

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <thread>
// #include <chrono>
// #include <iostream>
// #include <fstream>
// #include <iomanip>
// #include <vector>
// #include <mutex>
// #include <cmath>
// #include <algorithm>
// #include <deque>
// #include <ctime>

// // ==================== 数据结构定义 ====================

// // 点云帧结构
// struct PointCloudFrame {
//     uint32_t handle;
//     uint32_t frame_counter;
//     uint32_t dot_num;
//     uint8_t data_type;
//     uint8_t time_type;              // 0=本地, 1=PTP, 2=GPS
    
//     uint64_t radar_timestamp_ns;     // 雷达原始纳秒时间戳
//     uint64_t radar_timestamp_sec;    // 雷达时间戳（秒）
//     uint64_t radar_timestamp_us;     // 雷达时间戳（微秒）
//     uint64_t receive_timestamp_us;   // 系统接收时间（微秒）
//     uint64_t latency_us;             // 延迟（微秒）
    
//     std::string radar_time_str;      // 雷达时间（人类可读）
//     std::string receive_time_str;    // 系统接收时间（人类可读）
    
//     std::vector<float> x, y, z;
// };

// // 延迟统计
// struct LatencyStats {
//     uint64_t min_latency_us = UINT64_MAX;
//     uint64_t max_latency_us = 0;
//     double avg_latency_us = 0.0;
//     uint32_t sample_count = 0;
//     uint64_t total_latency_us = 0;
    
//     std::deque<uint64_t> last_100_latencies;
    
//     void add_sample(uint64_t latency_us) {
//         min_latency_us = std::min(min_latency_us, latency_us);
//         max_latency_us = std::max(max_latency_us, latency_us);
//         total_latency_us += latency_us;
//         sample_count++;
//         avg_latency_us = (double)total_latency_us / sample_count;
        
//         last_100_latencies.push_back(latency_us);
//         if (last_100_latencies.size() > 100) {
//             last_100_latencies.pop_front();
//         }
//     }
    
//     double get_jitter_us() {
//         if (last_100_latencies.size() < 2) return 0.0;
//         double mean = 0.0;
//         for (auto val : last_100_latencies) mean += val;
//         mean /= last_100_latencies.size();
//         double variance = 0.0;
//         for (auto val : last_100_latencies) {
//             variance += (val - mean) * (val - mean);
//         }
//         variance /= last_100_latencies.size();
//         return sqrt(variance);
//     }
    
//     void print() {
//         printf("\n========== Latency Statistics ==========\n");
//         printf("Total samples:     %u\n", sample_count);
//         printf("Min latency:       %lu us (%.3f ms)\n", min_latency_us, min_latency_us / 1000.0);
//         printf("Max latency:       %lu us (%.3f ms)\n", max_latency_us, max_latency_us / 1000.0);
//         printf("Avg latency:       %.2f us (%.3f ms)\n", avg_latency_us, avg_latency_us / 1000.0);
//         printf("Jitter (stddev):   %.2f us (%.3f ms)\n", get_jitter_us(), get_jitter_us() / 1000.0);
//         printf("========================================\n");
//     }
// };

// // ==================== 全局变量 ====================

// static std::mutex g_data_mutex;
// static std::vector<PointCloudFrame> g_pointcloud_frames;
// static LatencyStats g_latency_stats;

// static uint64_t g_last_frame_time_us = 0;
// static double g_current_fps = 0.0;
// static uint32_t g_frame_count = 0;
// static bool g_ptp_synced = false;

// // ==================== 工具函数 ====================

// uint64_t GetCurrentTimestampUs() {
//     struct timeval tv;
//     gettimeofday(&tv, nullptr);
//     return (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
// }

// // 将Unix时间戳（秒）转换为人类可读字符串
// std::string UnixTimeToHuman(uint64_t timestamp_sec) {
//     time_t rawtime = (time_t)timestamp_sec;
//     struct tm* timeinfo = localtime(&rawtime);
//     char buffer[64];
//     strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
//     return std::string(buffer);
// }

// // 将Unix时间戳（微秒）转换为人类可读字符串（含微秒）
// std::string UnixTimeUsToHuman(uint64_t timestamp_us) {
//     time_t rawtime = timestamp_us / 1000000;
//     struct tm* timeinfo = localtime(&rawtime);
//     char buffer[64];
//     strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
//     char us_str[16];
//     snprintf(us_str, sizeof(us_str), ".%06d", (int)(timestamp_us % 1000000));
//     return std::string(buffer) + us_str;
// }

// // 从 uint8_t[8] 数组中提取 uint64_t 时间戳（纳秒，小端序）
// uint64_t ExtractTimestampNs(const uint8_t* timestamp_array) {
//     uint64_t timestamp = 0;
//     for (int i = 0; i < 8; i++) {
//         timestamp |= ((uint64_t)timestamp_array[i]) << (i * 8);
//     }
//     return timestamp;
// }

// // ==================== 回调函数 ====================

// void PointCloudCallback(uint32_t handle, const uint8_t dev_type, 
//                         LivoxLidarEthernetPacket* data, void* client_data) {
//     if (data == nullptr) {
//         return;
//     }
    
//     uint64_t receive_time_us = GetCurrentTimestampUs();
    
//     // 帧率计算
//     if (g_last_frame_time_us > 0) {
//         double dt = (receive_time_us - g_last_frame_time_us) / 1000000.0;
//         if (dt > 0 && dt < 1.0) {
//             g_current_fps = 1.0 / dt;
//         }
//     }
//     g_last_frame_time_us = receive_time_us;
//     g_frame_count++;
    
//     PointCloudFrame frame;
//     frame.handle = handle;
//     frame.frame_counter = data->frame_cnt;
//     frame.dot_num = data->dot_num;
//     frame.data_type = data->data_type;
//     frame.time_type = data->time_type;
//     frame.receive_timestamp_us = receive_time_us;
//     frame.receive_time_str = UnixTimeUsToHuman(receive_time_us);
    
//     // 提取雷达时间戳（纳秒）
//     frame.radar_timestamp_ns = ExtractTimestampNs(data->timestamp);
//     frame.radar_timestamp_sec = frame.radar_timestamp_ns / 1000000000ULL;
//     frame.radar_timestamp_us = frame.radar_timestamp_ns / 1000;
    
//     // 转换雷达时间为人类可读
//     if (frame.radar_timestamp_sec > 0 && frame.radar_timestamp_sec < 3000000000ULL) {
//         frame.radar_time_str = UnixTimeToHuman(frame.radar_timestamp_sec);
//         // 加上微秒部分
//         char us_suffix[16];
//         snprintf(us_suffix, sizeof(us_suffix), ".%06d", 
//                  (int)(frame.radar_timestamp_ns % 1000000000ULL) / 1000);
//         frame.radar_time_str += us_suffix;
//     } else {
//         frame.radar_time_str = "INVALID_TIMESTAMP";
//     }
    
//     // ====== 计算延迟 ======
//     if (data->time_type == 1 && frame.radar_timestamp_sec > 0 && 
//         frame.radar_timestamp_sec < 3000000000ULL) {
//         // PTP同步：使用雷达时间戳计算延迟
//         g_ptp_synced = true;
        
//         // 延迟 = 系统接收时间（微秒）- 雷达时间戳（微秒）
//         if (receive_time_us >= frame.radar_timestamp_us) {
//             frame.latency_us = receive_time_us - frame.radar_timestamp_us;
//         } else {
//             // 如果雷达时间比系统时间晚（可能时间基准有微小差异）
//             frame.latency_us = 0;
//         }
        
//         // 限制异常值
//         if (frame.latency_us > 500000) {
//             frame.latency_us = 50000;  // 异常时设为50ms
//         }
//     } else {
//         // 未同步：使用帧间隔估算
//         static uint64_t last_time_us = 0;
//         static uint32_t last_counter = 0;
        
//         if (last_counter > 0 && data->frame_cnt > last_counter) {
//             uint32_t frame_diff = data->frame_cnt - last_counter;
//             uint64_t time_diff = receive_time_us - last_time_us;
//             if (frame_diff > 0 && time_diff > 0 && time_diff < 500000) {
//                 frame.latency_us = time_diff / frame_diff;
//             } else {
//                 frame.latency_us = 100000;
//             }
//         } else {
//             frame.latency_us = 100000;
//         }
//         last_time_us = receive_time_us;
//         last_counter = data->frame_cnt;
//         frame.radar_time_str = "NOT_SYNCED";
//     }
    
//     // 提取点云数据
//     frame.x.reserve(data->dot_num);
//     frame.y.reserve(data->dot_num);
//     frame.z.reserve(data->dot_num);
    
//     if (data->data_type == kLivoxLidarCartesianCoordinateHighData) {
//         LivoxLidarCartesianHighRawPoint* p_point_data = 
//             (LivoxLidarCartesianHighRawPoint*)data->data;
//         for (uint32_t i = 0; i < data->dot_num; i++) {
//             frame.x.push_back(p_point_data[i].x / 1000.0f);
//             frame.y.push_back(p_point_data[i].y / 1000.0f);
//             frame.z.push_back(p_point_data[i].z / 1000.0f);
//         }
//     } else if (data->data_type == kLivoxLidarCartesianCoordinateLowData) {
//         LivoxLidarCartesianLowRawPoint* p_point_data = 
//             (LivoxLidarCartesianLowRawPoint*)data->data;
//         for (uint32_t i = 0; i < data->dot_num; i++) {
//             frame.x.push_back(p_point_data[i].x / 1000.0f);
//             frame.y.push_back(p_point_data[i].y / 1000.0f);
//             frame.z.push_back(p_point_data[i].z / 1000.0f);
//         }
//     } else if (data->data_type == kLivoxLidarSphericalCoordinateData) {
//         LivoxLidarSpherPoint* p_point_data = (LivoxLidarSpherPoint*)data->data;
//         for (uint32_t i = 0; i < data->dot_num; i++) {
//             float depth = p_point_data[i].depth / 1000.0f;
//             float theta = p_point_data[i].theta / 1000.0f * M_PI / 180.0f;
//             float phi = p_point_data[i].phi / 1000.0f * M_PI / 180.0f;
//             frame.x.push_back(depth * sin(theta) * cos(phi));
//             frame.y.push_back(depth * sin(theta) * sin(phi));
//             frame.z.push_back(depth * cos(theta));
//         }
//     }
    
//     // 存储帧
//     {
//         std::lock_guard<std::mutex> lock(g_data_mutex);
//         g_pointcloud_frames.push_back(frame);
        
//         if (frame.latency_us > 0 && frame.latency_us < 500000) {
//             g_latency_stats.add_sample(frame.latency_us);
//         }
        
//         if (g_pointcloud_frames.size() > 10000) {
//             g_pointcloud_frames.erase(g_pointcloud_frames.begin());
//         }
//     }
    
//     // ====== 详细打印（每帧都打印前20帧，之后每10帧打印） ======
//     static int print_counter = 0;
//     print_counter++;
    
//     if (print_counter <= 20 || print_counter % 10 == 0) {
//         const char* sync_type = "LOCAL";
//         if (data->time_type == 1) sync_type = "PTP";
//         else if (data->time_type == 2) sync_type = "GPS";
        
//         printf("\n========================================\n");
//         printf("[Frame %u] %u points, Sync: %s\n", 
//                frame.frame_counter, frame.dot_num, sync_type);
//         printf("  Radar Time:   %s\n", frame.radar_time_str.c_str());
//         printf("  System Time:  %s\n", frame.receive_time_str.c_str());
//         printf("  Latency:      %lu us (%.3f ms)\n", 
//                frame.latency_us, frame.latency_us / 1000.0);
//         printf("  FPS:          %.1f\n", g_current_fps);
//         printf("========================================\n");
//     }
// }

// // IMU数据回调
// void ImuDataCallback(uint32_t handle, const uint8_t dev_type, 
//                      LivoxLidarEthernetPacket* data, void* client_data) {
//     if (data == nullptr) return;
//     static int imu_counter = 0;
//     if (++imu_counter % 100 == 0) {
//         printf("[IMU] handle=%u, frame_cnt=%u\n", handle, data->frame_cnt);
//     }
// }

// // 工作模式回调
// void WorkModeCallback(livox_status status, uint32_t handle,
//                       LivoxLidarAsyncControlResponse* response, void* client_data) {
//     if (response == nullptr) return;
//     printf("[WorkMode] status:%u, handle:%u, ret_code:%u, error_key:%u\n",
//            status, handle, response->ret_code, response->error_key);
// }

// // 雷达信息变化回调
// void LidarInfoChangeCallback(const uint32_t handle, const LivoxLidarInfo* info, 
//                              void* client_data) {
//     if (info == nullptr) {
//         printf("[Error] lidar info change callback failed\n");
//         return;
//     }
    
//     printf("[LidarInfo] handle=%u, SN=%s\n", handle, info->sn);
//     printf("[LidarInfo] Waiting for PTP sync...\n");
    
//     SetLivoxLidarWorkMode(handle, kLivoxLidarNormal, WorkModeCallback, nullptr);
// }

// // 推送消息回调
// void LivoxLidarPushMsgCallback(const uint32_t handle, const uint8_t dev_type, 
//                                const char* info, void* client_data) {
//     // 减少打印
// }

// // ==================== 数据保存 ====================

// void SavePointCloudToFile(const std::string& filename) {
//     std::lock_guard<std::mutex> lock(g_data_mutex);
    
//     std::ofstream file(filename);
//     if (!file.is_open()) {
//         printf("[Error] Failed to open file: %s\n", filename.c_str());
//         return;
//     }
    
//     file << "# PointCloud with Latency Information\n";
//     file << "# Generated: " << UnixTimeUsToHuman(GetCurrentTimestampUs()) << "\n";
//     file << "# Total Frames: " << g_pointcloud_frames.size() << "\n";
//     file << "# PTP Synced: " << (g_ptp_synced ? "YES" : "NO") << "\n";
//     file << "# Latency Stats (us): Min=" << g_latency_stats.min_latency_us 
//          << " Max=" << g_latency_stats.max_latency_us 
//          << " Avg=" << g_latency_stats.avg_latency_us << "\n";
//     file << "# \n";
    
//     file << "ply\n";
//     file << "format ascii 1.0\n";
    
//     int total_points = 0;
//     for (const auto& frame : g_pointcloud_frames) {
//         total_points += frame.dot_num;
//     }
    
//     file << "element vertex " << total_points << "\n";
//     file << "property float x\n";
//     file << "property float y\n";
//     file << "property float z\n";
//     file << "property uint32 frame_id\n";
//     file << "property uint64 latency_us\n";
//     file << "end_header\n";
    
//     for (const auto& frame : g_pointcloud_frames) {
//         for (size_t i = 0; i < frame.x.size(); i++) {
//             file << frame.x[i] << " " << frame.y[i] << " " << frame.z[i] 
//                  << " " << frame.frame_counter
//                  << " " << frame.latency_us << "\n";
//         }
//     }
    
//     file.close();
//     printf("\n[Save] Point cloud saved to: %s\n", filename.c_str());
//     printf("[Save] Total frames: %zu, Total points: %d\n", 
//            g_pointcloud_frames.size(), total_points);
// }

// // ==================== 主函数 ====================

// int main(int argc, const char* argv[]) {
//     if (argc != 2) {
//         printf("Usage: %s <config_file_path>\n", argv[0]);
//         printf("Example: %s ../config.json\n", argv[0]);
//         return -1;
//     }
    
//     const std::string path = argv[1];
    
//     printf("\n");
//     printf("╔══════════════════════════════════════════════════════════════╗\n");
//     printf("║   Livox LiDAR - PTP Latency Measurement                    ║\n");
//     printf("║   Radar timestamp (Unix epoch) vs System receive time      ║\n");
//     printf("╚══════════════════════════════════════════════════════════════╝\n");
//     printf("\n");
    
//     if (!LivoxLidarSdkInit(path.c_str())) {
//         printf("[Init] ✗ Livox SDK Init Failed\n");
//         LivoxLidarSdkUninit();
//         return -1;
//     }
//     printf("[Init] ✓ Livox SDK Init Success!\n");
    
//     SetLivoxLidarPointCloudCallBack(PointCloudCallback, nullptr);
//     SetLivoxLidarImuDataCallback(ImuDataCallback, nullptr);
//     SetLivoxLidarInfoCallback(LivoxLidarPushMsgCallback, nullptr);
//     SetLivoxLidarInfoChangeCallback(LidarInfoChangeCallback, nullptr);
    
//     printf("\n┌─────────────────────────────────────────────────────────────┐\n");
//     printf("│  System Running...                                         │\n");
//     printf("│  Press Ctrl+C to stop                                      │\n");
//     printf("└─────────────────────────────────────────────────────────────┘\n");
//     printf("\n");
    
//     int run_seconds = 300;
//     for (int i = 0; i < run_seconds; i++) {
//         sleep(1);
        
//         if (i % 10 == 0 && i > 0) {
//             std::lock_guard<std::mutex> lock(g_data_mutex);
//             size_t total_points = 0;
//             for (const auto& frame : g_pointcloud_frames) {
//                 total_points += frame.x.size();
//             }
//             printf("[Status] Frames: %4zu, Points: %6zu, PTP: %s, FPS: %.1f\n", 
//                    g_pointcloud_frames.size(), total_points,
//                    g_ptp_synced ? "SYNCED" : "WAITING", g_current_fps);
            
//             if (g_latency_stats.sample_count > 0) {
//                 printf("[Latency] Min: %5lu us, Max: %5lu us, Avg: %7.2f us, Jitter: %6.2f us\n",
//                        g_latency_stats.min_latency_us,
//                        g_latency_stats.max_latency_us,
//                        g_latency_stats.avg_latency_us,
//                        g_latency_stats.get_jitter_us());
//             }
//         }
//     }
    
//     SavePointCloudToFile("pointcloud_with_latency.ply");
//     printf("\n");
//     g_latency_stats.print();
    
//     LivoxLidarSdkUninit();
//     printf("\n[End] Done!\n");
    
//     return 0;
// }


//
// The MIT License (MIT)
//
// Copyright (c) 2022 Livox. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include "livox_lidar_def.h"
#include "livox_lidar_api.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <mutex>
#include <cmath>
#include <algorithm>
#include <deque>
#include <ctime>

// ==================== UDP 发送配置 ====================
#define UDP_PORT 9999
#define TARGET_IP "192.168.7.2"

static int udp_socket = -1;
static struct sockaddr_in target_addr;

// 发送数据结构（精简版，只传关键信息）
struct __attribute__((packed)) PointCloudPacket {
    uint32_t frame_counter;
    uint32_t dot_num;
    uint64_t radar_timestamp_us;    // 雷达采集时间（微秒）
    uint64_t send_timestamp_us;     // 发送时间（微秒）
    uint64_t latency_us;            // 发送端计算的延迟
    uint32_t point_count;           // 实际点数
    // 后面跟着点云数据：float x, y, z 依次排列
};

// ==================== 数据结构定义 ====================

// 点云帧结构
struct PointCloudFrame {
    uint32_t handle;
    uint32_t frame_counter;
    uint32_t dot_num;
    uint8_t data_type;
    uint8_t time_type;              // 0=本地, 1=PTP, 2=GPS
    
    uint64_t radar_timestamp_ns;     // 雷达原始纳秒时间戳
    uint64_t radar_timestamp_sec;    // 雷达时间戳（秒）
    uint64_t radar_timestamp_us;     // 雷达时间戳（微秒）
    uint64_t receive_timestamp_us;   // 系统接收时间（微秒）
    uint64_t latency_us;             // 延迟（微秒）
    
    std::string radar_time_str;      // 雷达时间（人类可读）
    std::string receive_time_str;    // 系统接收时间（人类可读）
    
    std::vector<float> x, y, z;
};

// 延迟统计
struct LatencyStats {
    uint64_t min_latency_us = UINT64_MAX;
    uint64_t max_latency_us = 0;
    double avg_latency_us = 0.0;
    uint32_t sample_count = 0;
    uint64_t total_latency_us = 0;
    
    std::deque<uint64_t> last_100_latencies;
    
    void add_sample(uint64_t latency_us) {
        min_latency_us = std::min(min_latency_us, latency_us);
        max_latency_us = std::max(max_latency_us, latency_us);
        total_latency_us += latency_us;
        sample_count++;
        avg_latency_us = (double)total_latency_us / sample_count;
        
        last_100_latencies.push_back(latency_us);
        if (last_100_latencies.size() > 100) {
            last_100_latencies.pop_front();
        }
    }
    
    double get_jitter_us() {
        if (last_100_latencies.size() < 2) return 0.0;
        double mean = 0.0;
        for (auto val : last_100_latencies) mean += val;
        mean /= last_100_latencies.size();
        double variance = 0.0;
        for (auto val : last_100_latencies) {
            variance += (val - mean) * (val - mean);
        }
        variance /= last_100_latencies.size();
        return sqrt(variance);
    }
    
    void print() {
        printf("\n========== Latency Statistics ==========\n");
        printf("Total samples:     %u\n", sample_count);
        printf("Min latency:       %lu us (%.3f ms)\n", min_latency_us, min_latency_us / 1000.0);
        printf("Max latency:       %lu us (%.3f ms)\n", max_latency_us, max_latency_us / 1000.0);
        printf("Avg latency:       %.2f us (%.3f ms)\n", avg_latency_us, avg_latency_us / 1000.0);
        printf("Jitter (stddev):   %.2f us (%.3f ms)\n", get_jitter_us(), get_jitter_us() / 1000.0);
        printf("========================================\n");
    }
};

// ==================== 全局变量 ====================

static std::mutex g_data_mutex;
static std::vector<PointCloudFrame> g_pointcloud_frames;
static LatencyStats g_latency_stats;

static uint64_t g_last_frame_time_us = 0;
static double g_current_fps = 0.0;
static uint32_t g_frame_count = 0;
static bool g_ptp_synced = false;


uint64_t GetCurrentTimestampUs() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

std::string UnixTimeToHuman(uint64_t timestamp_sec) {
    time_t rawtime = (time_t)timestamp_sec;
    struct tm* timeinfo = localtime(&rawtime);
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return std::string(buffer);
}

std::string UnixTimeUsToHuman(uint64_t timestamp_us) {
    time_t rawtime = timestamp_us / 1000000;
    struct tm* timeinfo = localtime(&rawtime);
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    char us_str[16];
    snprintf(us_str, sizeof(us_str), ".%06d", (int)(timestamp_us % 1000000));
    return std::string(buffer) + us_str;
}

uint64_t ExtractTimestampNs(const uint8_t* timestamp_array) {
    uint64_t timestamp = 0;
    for (int i = 0; i < 8; i++) {
        timestamp |= ((uint64_t)timestamp_array[i]) << (i * 8);
    }
    return timestamp;
}
// ==================== UDP 发送函数 ====================

bool InitUdpSender() {
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0) {
        printf("[UDP] Failed to create socket: %s\n", strerror(errno));
        return false;
    }
    
    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(UDP_PORT);
    if (inet_pton(AF_INET, TARGET_IP, &target_addr.sin_addr) <= 0) {
        printf("[UDP] Invalid target IP: %s\n", TARGET_IP);
        close(udp_socket);
        udp_socket = -1;
        return false;
    }
    
    printf("[UDP] Sender initialized, target: %s:%d\n", TARGET_IP, UDP_PORT);
    return true;
}

void SendPointCloud(const PointCloudFrame& frame) {
    if (udp_socket < 0) return;
    
    size_t point_count = frame.x.size();
    if (point_count == 0) return;
    
    size_t data_size = sizeof(PointCloudPacket) + point_count * 3 * sizeof(float);
    if (data_size > 65535) {
        // UDP 包最大 65535，如果太大就只发送部分
        printf("[UDP] Warning: packet too large (%zu bytes), truncating\n", data_size);
        point_count = (65535 - sizeof(PointCloudPacket)) / (3 * sizeof(float));
        data_size = sizeof(PointCloudPacket) + point_count * 3 * sizeof(float);
    }
    
    std::vector<uint8_t> buffer(data_size);
    PointCloudPacket* pkt = (PointCloudPacket*)buffer.data();
    
    pkt->frame_counter = frame.frame_counter;
    pkt->dot_num = frame.dot_num;
    pkt->radar_timestamp_us = frame.radar_timestamp_us;
    pkt->send_timestamp_us = GetCurrentTimestampUs();
    pkt->latency_us = frame.latency_us;
    pkt->point_count = point_count;
    
    float* data_ptr = (float*)(pkt + 1);
    for (size_t i = 0; i < point_count; i++) {
        data_ptr[i * 3 + 0] = frame.x[i];
        data_ptr[i * 3 + 1] = frame.y[i];
        data_ptr[i * 3 + 2] = frame.z[i];
    }
    
    ssize_t sent = sendto(udp_socket, buffer.data(), data_size, 0,
                          (struct sockaddr*)&target_addr, sizeof(target_addr));
    
    if (sent < 0) {
        printf("[UDP] Send failed: %s\n", strerror(errno));
    }
}

// ==================== 工具函数 ====================



// ==================== 回调函数 ====================

// void PointCloudCallback(uint32_t handle, const uint8_t dev_type, 
//                         LivoxLidarEthernetPacket* data, void* client_data) {
//     if (data == nullptr) return;
    
//     uint64_t receive_time_us = GetCurrentTimestampUs();
    
//     if (g_last_frame_time_us > 0) {
//         double dt = (receive_time_us - g_last_frame_time_us) / 1000000.0;
//         if (dt > 0 && dt < 1.0) {
//             g_current_fps = 1.0 / dt;
//         }
//     }
//     g_last_frame_time_us = receive_time_us;
//     g_frame_count++;
    
//     PointCloudFrame frame;
//     frame.handle = handle;
//     frame.frame_counter = data->frame_cnt;
//     frame.dot_num = data->dot_num;
//     frame.data_type = data->data_type;
//     frame.time_type = data->time_type;
//     frame.receive_timestamp_us = receive_time_us;
//     frame.receive_time_str = UnixTimeUsToHuman(receive_time_us);
    
//     frame.radar_timestamp_ns = ExtractTimestampNs(data->timestamp);
//     frame.radar_timestamp_sec = frame.radar_timestamp_ns / 1000000000ULL;
//     frame.radar_timestamp_us = frame.radar_timestamp_ns / 1000;
    
//     if (frame.radar_timestamp_sec > 0 && frame.radar_timestamp_sec < 3000000000ULL) {
//         frame.radar_time_str = UnixTimeToHuman(frame.radar_timestamp_sec);
//         char us_suffix[16];
//         snprintf(us_suffix, sizeof(us_suffix), ".%06d", 
//                  (int)(frame.radar_timestamp_ns % 1000000000ULL) / 1000);
//         frame.radar_time_str += us_suffix;
//     } else {
//         frame.radar_time_str = "INVALID_TIMESTAMP";
//     }
    
//     // ====== 延迟计算（去掉截断） ======
//     if (data->time_type == 1 && frame.radar_timestamp_sec > 0 && 
//         frame.radar_timestamp_sec < 3000000000ULL) {
//         g_ptp_synced = true;
        
//         int64_t latency = (int64_t)receive_time_us - (int64_t)frame.radar_timestamp_us;
        
//         if (latency >= 0) {
//             frame.latency_us = (uint64_t)latency;
//         } else {
//             // 延迟为负，使用帧间隔估算
//             static uint64_t last_time_us = 0;
//             static uint32_t last_counter = 0;
//             if (last_counter > 0 && data->frame_cnt > last_counter) {
//                 uint32_t frame_diff = data->frame_cnt - last_counter;
//                 uint64_t time_diff = receive_time_us - last_time_us;
//                 if (frame_diff > 0 && time_diff > 0 && time_diff < 5000000) {
//                     frame.latency_us = time_diff / frame_diff;
//                 } else {
//                     frame.latency_us = 0;
//                 }
//             } else {
//                 frame.latency_us = 0;
//             }
//             last_time_us = receive_time_us;
//             last_counter = data->frame_cnt;
//             // 保留雷达时间字符串，但添加标记
//             frame.radar_time_str += " (EST)";
//         }
//     } else {
//         // 非 PTP 模式，使用帧间隔估算
//         static uint64_t last_time_us = 0;
//         static uint32_t last_counter = 0;
        
//         if (last_counter > 0 && data->frame_cnt > last_counter) {
//             uint32_t frame_diff = data->frame_cnt - last_counter;
//             uint64_t time_diff = receive_time_us - last_time_us;
//             if (frame_diff > 0 && time_diff > 0 && time_diff < 5000000) {
//                 frame.latency_us = time_diff / frame_diff;
//             } else {
//                 frame.latency_us = 0;
//             }
//         } else {
//             frame.latency_us = 0;
//         }
//         last_time_us = receive_time_us;
//         last_counter = data->frame_cnt;
//         if (frame.radar_time_str != "INVALID_TIMESTAMP") {
//             frame.radar_time_str += " (EST)";
//         } else {
//             frame.radar_time_str = "ESTIMATED";
//         }
//     }
    
//     // 提取点云数据（保持不变）...
//     frame.x.reserve(data->dot_num);
//     frame.y.reserve(data->dot_num);
//     frame.z.reserve(data->dot_num);
    
//     if (data->data_type == kLivoxLidarCartesianCoordinateHighData) {
//         LivoxLidarCartesianHighRawPoint* p_point_data = 
//             (LivoxLidarCartesianHighRawPoint*)data->data;
//         for (uint32_t i = 0; i < data->dot_num; i++) {
//             frame.x.push_back(p_point_data[i].x / 1000.0f);
//             frame.y.push_back(p_point_data[i].y / 1000.0f);
//             frame.z.push_back(p_point_data[i].z / 1000.0f);
//         }
//     } else if (data->data_type == kLivoxLidarCartesianCoordinateLowData) {
//         LivoxLidarCartesianLowRawPoint* p_point_data = 
//             (LivoxLidarCartesianLowRawPoint*)data->data;
//         for (uint32_t i = 0; i < data->dot_num; i++) {
//             frame.x.push_back(p_point_data[i].x / 1000.0f);
//             frame.y.push_back(p_point_data[i].y / 1000.0f);
//             frame.z.push_back(p_point_data[i].z / 1000.0f);
//         }
//     } else if (data->data_type == kLivoxLidarSphericalCoordinateData) {
//         LivoxLidarSpherPoint* p_point_data = (LivoxLidarSpherPoint*)data->data;
//         for (uint32_t i = 0; i < data->dot_num; i++) {
//             float depth = p_point_data[i].depth / 1000.0f;
//             float theta = p_point_data[i].theta / 1000.0f * M_PI / 180.0f;
//             float phi = p_point_data[i].phi / 1000.0f * M_PI / 180.0f;
//             frame.x.push_back(depth * sin(theta) * cos(phi));
//             frame.y.push_back(depth * sin(theta) * sin(phi));
//             frame.z.push_back(depth * cos(theta));
//         }
//     }
    
//     // 存储帧
//     {
//         std::lock_guard<std::mutex> lock(g_data_mutex);
//         g_pointcloud_frames.push_back(frame);
        
//         if (frame.latency_us > 0 && frame.latency_us < 500000) {
//             g_latency_stats.add_sample(frame.latency_us);
//         }
        
//         if (g_pointcloud_frames.size() > 10000) {
//             g_pointcloud_frames.erase(g_pointcloud_frames.begin());
//         }
//     }
    
//     // 发送 UDP
//      SendPointCloud(frame);
//     // ====== 降低 UDP 发送频率 ======
// // static int udp_send_counter = 0;
// // udp_send_counter++;

// // // 每 3 帧发送一次（可调整：2, 3, 5, 10）
// // const int SEND_INTERVAL = 10;

// // if (udp_send_counter % SEND_INTERVAL == 0) {
// //     SendPointCloud(frame);
// // }
// //     static int print_counter = 0;
// //     print_counter++;
    
//     // if (print_counter <= 20 || print_counter % 10 == 0) {
//     //     const char* sync_type = "LOCAL";
//     //     if (data->time_type == 1) sync_type = "PTP";
//     //     else if (data->time_type == 2) sync_type = "GPS";
        
//     //     printf("\n========================================\n");
//     //     printf("[Frame %u] %u points, Sync: %s\n", 
//     //            frame.frame_counter, frame.dot_num, sync_type);
//     //     printf("  Radar Time:   %s\n", frame.radar_time_str.c_str());
//     //     printf("  System Time:  %s\n", frame.receive_time_str.c_str());
//     //     printf("  Latency:      %lu us (%.3f ms)\n", 
//     //            frame.latency_us, frame.latency_us / 1000.0);
//     //     printf("  FPS:          %.1f\n", g_current_fps);
//     //     printf("  [UDP] Sent to %s:%d\n", TARGET_IP, UDP_PORT);
//     //     printf("========================================\n");
//     // }
// }


void PointCloudCallback(uint32_t handle, const uint8_t dev_type, 
                        LivoxLidarEthernetPacket* data, void* client_data) {
    if (data == nullptr) return;
    
    uint64_t receive_time_us = GetCurrentTimestampUs();
    
    // ====== 直接转发原始数据，不做任何解析 ======
    // 构造转发的 UDP 包：只包含原始数据 + 接收时间戳
    struct ForwardPacket {
        uint64_t receive_time_us;           // 接收时间（微秒）
        uint32_t frame_counter;              // 帧计数器
        uint32_t data_length;                // 原始数据长度
        uint8_t raw_data[1];                 // 原始数据（柔性数组）
    };
    
    size_t total_size = sizeof(ForwardPacket) - 1 + data->length;
    std::vector<uint8_t> buffer(total_size);
    ForwardPacket* pkt = (ForwardPacket*)buffer.data();
    
    pkt->receive_time_us = receive_time_us;
    pkt->frame_counter = data->frame_cnt;
    pkt->data_length = data->length;
    memcpy(pkt->raw_data, data->data, data->length);
    
    // 发送到 7.2
    if (udp_socket >= 0) {
        ssize_t sent = sendto(udp_socket, buffer.data(), total_size, 0,
                              (struct sockaddr*)&target_addr, sizeof(target_addr));
    }
    
    // 不再解析点云，不再计算延迟
}


void ImuDataCallback(uint32_t handle, const uint8_t dev_type, 
                     LivoxLidarEthernetPacket* data, void* client_data) {
    if (data == nullptr) return;
    static int imu_counter = 0;
    // if (++imu_counter % 100 == 0) {
    //     printf("[IMU] handle=%u, frame_cnt=%u\n", handle, data->frame_cnt);
    // }
}

void WorkModeCallback(livox_status status, uint32_t handle,
                      LivoxLidarAsyncControlResponse* response, void* client_data) {
    if (response == nullptr) return;
    printf("[WorkMode] status:%u, handle:%u, ret_code:%u, error_key:%u\n",
           status, handle, response->ret_code, response->error_key);
}

void LidarInfoChangeCallback(const uint32_t handle, const LivoxLidarInfo* info, 
                             void* client_data) {
    if (info == nullptr) {
        printf("[Error] lidar info change callback failed\n");
        return;
    }
    
    printf("[LidarInfo] handle=%u, SN=%s\n", handle, info->sn);
    printf("[LidarInfo] Waiting for PTP sync...\n");
    
    SetLivoxLidarWorkMode(handle, kLivoxLidarNormal, WorkModeCallback, nullptr);
}

void LivoxLidarPushMsgCallback(const uint32_t handle, const uint8_t dev_type, 
                               const char* info, void* client_data) {
    // 减少打印
}

void SavePointCloudToFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(g_data_mutex);
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        printf("[Error] Failed to open file: %s\n", filename.c_str());
        return;
    }
    
    file << "# PointCloud with Latency Information\n";
    file << "# Generated: " << UnixTimeUsToHuman(GetCurrentTimestampUs()) << "\n";
    file << "# Total Frames: " << g_pointcloud_frames.size() << "\n";
    file << "# PTP Synced: " << (g_ptp_synced ? "YES" : "NO") << "\n";
    file << "# Latency Stats (us): Min=" << g_latency_stats.min_latency_us 
         << " Max=" << g_latency_stats.max_latency_us 
         << " Avg=" << g_latency_stats.avg_latency_us << "\n";
    file << "# \n";
    
    file << "ply\n";
    file << "format ascii 1.0\n";
    
    int total_points = 0;
    for (const auto& frame : g_pointcloud_frames) {
        total_points += frame.dot_num;
    }
    
    file << "element vertex " << total_points << "\n";
    file << "property float x\n";
    file << "property float y\n";
    file << "property float z\n";
    file << "property uint32 frame_id\n";
    file << "property uint64 latency_us\n";
    file << "end_header\n";
    
    for (const auto& frame : g_pointcloud_frames) {
        for (size_t i = 0; i < frame.x.size(); i++) {
            file << frame.x[i] << " " << frame.y[i] << " " << frame.z[i] 
                 << " " << frame.frame_counter
                 << " " << frame.latency_us << "\n";
        }
    }
    
    file.close();
    printf("\n[Save] Point cloud saved to: %s\n", filename.c_str());
    printf("[Save] Total frames: %zu, Total points: %d\n", 
           g_pointcloud_frames.size(), total_points);
}

// ==================== 主函数 ====================

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <config_file_path>\n", argv[0]);
        printf("Example: %s ../config.json\n", argv[0]);
        return -1;
    }
    
    const std::string path = argv[1];
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   Livox LiDAR - PTP Latency Measurement (Sender)           ║\n");
    printf("║   Sending point cloud to 192.168.7.2:9999                  ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    if (!InitUdpSender()) {
        printf("[Warning] UDP sender init failed, continuing without sending\n");
    }
    
    if (!LivoxLidarSdkInit(path.c_str())) {
        printf("[Init] ✗ Livox SDK Init Failed\n");
        LivoxLidarSdkUninit();
        return -1;
    }
    printf("[Init] ✓ Livox SDK Init Success!\n");
    
    SetLivoxLidarPointCloudCallBack(PointCloudCallback, nullptr);
    SetLivoxLidarImuDataCallback(ImuDataCallback, nullptr);
    SetLivoxLidarInfoCallback(LivoxLidarPushMsgCallback, nullptr);
    SetLivoxLidarInfoChangeCallback(LidarInfoChangeCallback, nullptr);
    
    printf("\n┌─────────────────────────────────────────────────────────────┐\n");
    printf("│  System Running...                                         │\n");
    printf("│  Press Ctrl+C to stop                                      │\n");
    printf("└─────────────────────────────────────────────────────────────┘\n");
    printf("\n");
    
    int run_seconds = 300;
    for (int i = 0; i < run_seconds; i++) {
        sleep(1);
        
        if (i % 10 == 0 && i > 0) {
            std::lock_guard<std::mutex> lock(g_data_mutex);
            size_t total_points = 0;
            for (const auto& frame : g_pointcloud_frames) {
                total_points += frame.x.size();
            }
            printf("[Status] Frames: %4zu, Points: %6zu, PTP: %s, FPS: %.1f\n", 
                   g_pointcloud_frames.size(), total_points,
                   g_ptp_synced ? "SYNCED" : "WAITING", g_current_fps);
            
            if (g_latency_stats.sample_count > 0) {
                printf("[Latency] Min: %5lu us, Max: %5lu us, Avg: %7.2f us, Jitter: %6.2f us\n",
                       g_latency_stats.min_latency_us,
                       g_latency_stats.max_latency_us,
                       g_latency_stats.avg_latency_us,
                       g_latency_stats.get_jitter_us());
            }
        }
    }
    
    SavePointCloudToFile("pointcloud_with_latency.ply");
    printf("\n");
    g_latency_stats.print();
    
    if (udp_socket >= 0) {
        close(udp_socket);
    }
    
    LivoxLidarSdkUninit();
    printf("\n[End] Done!\n");
    
    return 0;
}