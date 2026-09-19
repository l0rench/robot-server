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
#include <fcntl.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <mutex>
#include <sched.h>

// ==================== UDP 发送配置 ====================
#define UDP_PORT 9999
#define TARGET_IP "192.168.7.2"
#define MAX_UDP_PACKET_SIZE 65536

static int udp_socket = -1;
static struct sockaddr_in target_addr;
static uint8_t send_buffer[MAX_UDP_PACKET_SIZE];

// ==================== 工具函数 ====================

uint64_t GetCurrentTimestampUs() {
struct timespec ts_spec;
clock_gettime(CLOCK_REALTIME, &ts_spec);
uint64_t ts = (uint64_t)ts_spec.tv_sec * 1000000 + ts_spec.tv_nsec / 1000;
return ts;  // ← 加上这行
}

uint64_t ExtractTimestampNs(const uint8_t* timestamp_array) {
    uint64_t timestamp = 0;
    for (int i = 0; i < 8; i++) {
        timestamp |= ((uint64_t)timestamp_array[i]) << (i * 8);
    }
    return timestamp;
}

// ==================== UDP 发送初始化 ====================

bool InitUdpSender() {
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0) {
        return false;
    }

    // 1. 非阻塞
    int flags = fcntl(udp_socket, F_GETFL, 0);
    fcntl(udp_socket, F_SETFL, flags | O_NONBLOCK);

    // 2. 增大发送缓冲区
    int send_buf_size = 8 * 1024 * 1024;
    setsockopt(udp_socket, SOL_SOCKET, SO_SNDBUF,
               &send_buf_size, sizeof(send_buf_size));

    // 3. 发送超时 1ms
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    setsockopt(udp_socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    // 4. 禁用 UDP 校验和（可选，提高速度）
    int val = 1;
    setsockopt(udp_socket, SOL_SOCKET, SO_NO_CHECK, &val, sizeof(val));

    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(UDP_PORT);
    if (inet_pton(AF_INET, TARGET_IP, &target_addr.sin_addr) <= 0) {
        close(udp_socket);
        udp_socket = -1;
        return false;
    }

    return true;
}

// ==================== 转发数据结构 ====================
struct __attribute__((packed)) ForwardPacket {
    uint64_t radar_timestamp_us;
    uint64_t receive_time_us;
    uint64_t send_time_us;
    uint32_t frame_counter;
    uint32_t data_length;
    uint8_t raw_data[1];
};

// ==================== 回调函数 ====================

void PointCloudCallback(uint32_t handle, const uint8_t dev_type,
                        LivoxLidarEthernetPacket* data, void* client_data) {
    if (data == nullptr) return;

    uint64_t current_time_us = GetCurrentTimestampUs();
    uint64_t radar_timestamp_us = ExtractTimestampNs(data->timestamp) / 1000;

    size_t total_size = sizeof(ForwardPacket) - 1 + data->length;
    if (total_size > MAX_UDP_PACKET_SIZE) {
        return;
    }

    ForwardPacket* pkt = (ForwardPacket*)send_buffer;
    pkt->radar_timestamp_us = radar_timestamp_us;
    pkt->receive_time_us = current_time_us;
    pkt->send_time_us = current_time_us;
    pkt->frame_counter = data->frame_cnt;
    pkt->data_length = data->length;
    memcpy(pkt->raw_data, data->data, data->length);

    if (udp_socket >= 0) {
        // 使用 sendto（简单，足够快）
        sendto(udp_socket, send_buffer, total_size, 0,
               (struct sockaddr*)&target_addr, sizeof(target_addr));
    }
}

void WorkModeCallback(livox_status status, uint32_t handle,
                      LivoxLidarAsyncControlResponse* response, void* client_data) {}

void LidarInfoChangeCallback(const uint32_t handle, const LivoxLidarInfo* info,
                             void* client_data) {
    if (info == nullptr) return;
    SetLivoxLidarWorkMode(handle, kLivoxLidarNormal, WorkModeCallback, nullptr);
}

void LivoxLidarPushMsgCallback(const uint32_t handle, const uint8_t dev_type,
                               const char* info, void* client_data) {}

// ==================== 主函数 ====================

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <config_file_path>\n", argv[0]);
        return -1;
    }
    // 绑定到核心 2
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(2, &cpuset);
    sched_setaffinity(0, sizeof(cpuset), &cpuset);

    // 设置 FIFO 优先级

    // 实时调度优先级
    struct sched_param param;
    param.sched_priority = 80;
    if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
        perror("sched_setscheduler");
    }

    const std::string path = argv[1];

    if (!InitUdpSender()) {
        printf("[UDP] Failed to init sender\n");
        return -1;
    }
    printf("[UDP] Sender initialized, target: %s:%d\n", TARGET_IP, UDP_PORT);

    if (!LivoxLidarSdkInit(path.c_str())) {
        printf("[Init] Livox SDK Init Failed\n");
        LivoxLidarSdkUninit();
        return -1;
    }
    printf("[Init] Livox SDK Init Success!\n");

    DisableLivoxSdkConsoleLogger();

    SetLivoxLidarPointCloudCallBack(PointCloudCallback, nullptr);
    SetLivoxLidarInfoCallback(LivoxLidarPushMsgCallback, nullptr);
    SetLivoxLidarInfoChangeCallback(LidarInfoChangeCallback, nullptr);

    printf("Forwarding point cloud data to %s:%d\n", TARGET_IP, UDP_PORT);
    printf("Press Ctrl+C to stop\n");

    while (true) {
        sleep(1);
    }

    if (udp_socket >= 0) {
        close(udp_socket);
    }

    LivoxLidarSdkUninit();
    return 0;
}