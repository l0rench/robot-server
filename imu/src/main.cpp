#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <math.h>
#include "serial_port.h"
#include "imu_config.h"

// ========== UDP 配置 ==========
#define TARGET_IP   "192.168.7.2"
#define TARGET_PORT 9005

static int udp_sock = -1;
static struct sockaddr_in target_addr;
static unsigned long long frame_seq = 0;

// ========== 全局 AHRS 状态 ==========
typedef struct {
    double timestamp;
    float roll, pitch, yaw;
    float gx, gy, gz;
    float ax, ay, az;
    volatile int valid;
} ahrs_frame_t;

static ahrs_frame_t latest_frame = {0};
static pthread_mutex_t frame_mutex = PTHREAD_MUTEX_INITIALIZER;

// ========== 发送UDP数据 ==========
static void send_udp_data(const ahrs_frame_t* f) {
    if (udp_sock < 0) return;
    
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    unsigned long long send_time_us = ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000;
    
    unsigned char packet[64];
    int offset = 0;
    
    memcpy(packet + offset, &frame_seq, 8);
    offset += 8;
    frame_seq++;
    
    memcpy(packet + offset, &send_time_us, 8);
    offset += 8;
    
    memcpy(packet + offset, &f->roll, 4);
    offset += 4;
    memcpy(packet + offset, &f->pitch, 4);
    offset += 4;
    memcpy(packet + offset, &f->yaw, 4);
    offset += 4;
    
    memcpy(packet + offset, &f->gx, 4);
    offset += 4;
    memcpy(packet + offset, &f->gy, 4);
    offset += 4;
    memcpy(packet + offset, &f->gz, 4);
    offset += 4;
    
    memcpy(packet + offset, &f->ax, 4);
    offset += 4;
    memcpy(packet + offset, &f->ay, 4);
    offset += 4;
    memcpy(packet + offset, &f->az, 4);
    offset += 4;
    
    sendto(udp_sock, packet, offset, 0,
           (struct sockaddr*)&target_addr, sizeof(target_addr));
}

// ========== AHRS 更新回调 ==========
extern "C" void on_ahrs_update(float roll, float pitch, float yaw,
                               float gx, float gy, float gz,
                               float ax, float ay, float az) {
    pthread_mutex_lock(&frame_mutex);
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    latest_frame.timestamp = ts.tv_sec + ts.tv_nsec / 1e9;
    latest_frame.roll = roll;
    latest_frame.pitch = pitch;
    latest_frame.yaw = yaw;
    latest_frame.gx = gx;
    latest_frame.gy = gy;
    latest_frame.gz = gz;
    latest_frame.ax = ax;
    latest_frame.ay = ay;
    latest_frame.az = az;
    latest_frame.valid = 1;
    
    ahrs_frame_t copy = latest_frame;
    pthread_mutex_unlock(&frame_mutex);
    
    send_udp_data(&copy);
}

// ========== UDP 初始化 ==========
static int init_udp_sender(const char* ip, int port) {
    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock < 0) {
        perror("UDP socket create failed");
        return -1;
    }
    
    int sendbuf_size = 1024 * 1024;
    setsockopt(udp_sock, SOL_SOCKET, SO_SNDBUF, &sendbuf_size, sizeof(sendbuf_size));
    
    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &target_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid target IP: %s\n", ip);
        close(udp_sock);
        udp_sock = -1;
        return -1;
    }
    
    printf("UDP sender ready: %s:%d\n", ip, port);
    return 0;
}

// ============================================================
// 主函数
// ============================================================
int main(int argc, char** argv) {
    const char* port = (argc > 1) ? argv[1] : "ttyS5";
    int baud = 921600;
    
    const char* target_ip = TARGET_IP;
    int target_port = TARGET_PORT;
    if (argc > 3) {
        target_ip = argv[2];
        target_port = atoi(argv[3]);
    }
    
    if (init_udp_sender(target_ip, target_port) < 0) {
        fprintf(stderr, "Failed to initialize UDP sender\n");
        exit(1);
    }
    
    int fd = serial_port_open(port);
    if (fd < 0) {
        fprintf(stderr, "Failed to open serial port %s\n", port);
        exit(1);
    }
    
    if (serial_port_configure(fd, baud) < 0) {
        fprintf(stderr, "Failed to configure serial port\n");
        serial_port_close(fd);
        exit(1);
    }
    printf("Serial port %s opened at %d baud.\n", port, baud);
    
    // ============================================================
    // 配置 IMU 为 1000Hz（首次运行取消注释）
    // ============================================================
    // printf("\n========== Configuring IMU to 1000Hz ==========\n");
    // imu_config_1000hz(fd);
    // printf("========== Configuration Complete ==========\n");
    // sleep(3);
    
    printf("Forwarding IMU data to %s:%d\n", target_ip, target_port);
    printf("Press Ctrl+C to exit.\n\n");
    
    // ============================================================
    // 批量读取 - 提高效率
    // ============================================================
    uint8_t buffer[512];
    
    while (1) {
        int n = read(fd, buffer, sizeof(buffer));
        if (n > 0) {
            for (int i = 0; i < n; i++) {
                imu_rx(buffer[i]);
            }
        }
        // 极短延迟，让CPU有时间处理其他任务
        usleep(1);
    }
    
    close(udp_sock);
    serial_port_close(fd);
    return 0;
}