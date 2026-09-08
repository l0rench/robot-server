// shared_data.h - 修改为使用消息队列
#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include "motor_api.h"
#include "ipc_msg_queue.h"

// 系统配置
typedef struct {
    char server_ip[16];       // 服务器IP
    int server_port;          // 服务器端口
    
    // 电机保护参数
    float max_current_1;      // 电机1最大电流(mA)
    float max_current_2;      // 电机2最大电流(mA)
    float max_speed_1;        // 电机1最大速度(RPM)
    float max_speed_2;        // 电机2最大速度(RPM)
    
    // 默认运动参数
    float default_accel;      // 默认加速度(RPM/s²)
    float default_decel;      // 默认减速度(RPM/s²)
    
    // 系统配置
    int debug_mode;           // 调试模式
    int can_baudrate;         // CAN总线波特率
} SystemConfig;

// 全局数据
typedef struct {
    volatile bool system_running;     // 系统运行标志
    bool motors_initialized;          // 电机初始化标志
    bool tcp_connected;               // TCP连接状态
    
    SystemConfig config;              // 系统配置
    MessageQueues msg_queues;         // POSIX消息队列
    
    // 线程同步
    pthread_mutex_t cmd_mutex;        // 命令互斥锁
    pthread_cond_t cmd_cond;          // 命令条件变量
    MotorCmdMsg last_motor_cmd;       // 最新的电机命令
    
    // 统计数据
    uint64_t env_samples;             // 环境数据采样次数
    uint64_t imu_samples;             // IMU数据采样次数
    uint64_t motor_samples;           // 电机数据采样次数
} GlobalData;


// 线程函数声明
void* env_sensor_thread(void* arg);
void* imu_collector_thread(void* arg);
void* motor_status_thread(void* arg);
void* display_thread(void* arg);
void* tcp_report_thread(void* arg);
void* motor_control_thread(void* arg);
void parse_motor_command(const char* cmd, void* g_data_ptr);

// 工具函数

void init_default_config(SystemConfig* config);
#endif // SHARED_DATA_H