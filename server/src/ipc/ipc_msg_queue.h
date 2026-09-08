// // ipc_msg_queue.h
// #ifndef IPC_MSG_QUEUE_H
// #define IPC_MSG_QUEUE_H

// #include <mqueue.h>
// #include <pthread.h>
// #include <stdint.h>
// #include <stdbool.h>
// #include <time.h>

// // POSIX消息队列名称
// #define MQ_ENV_NAME     "/env_mq"
// #define MQ_MOTOR_NAME   "/motor_mq"
// #define MQ_IMU_NAME     "/imu_mq"
// #define MQ_CMD_NAME     "/cmd_mq"

// // 最大消息数量和大小
// #define MAX_MSGS        100
// #define MAX_MSG_SIZE    1024

// // 消息类型定义
// typedef enum {
//     MSG_TYPE_ENV_DATA = 1,     // 环境传感器数据
//     MSG_TYPE_MOTOR_DATA,       // 电机状态数据  
//     MSG_TYPE_IMU_DATA,         // IMU数据
//     MSG_TYPE_MOTOR_CMD,        // 电机控制命令
//     MSG_TYPE_SYS_CMD           // 系统命令
// } MsgType;

// // 环境传感器消息
// typedef struct {
//     MsgType msg_type;          // 消息类型
//     float temperature;         // 温度 (°C)
//     float humidity;            // 湿度 (%)
//     float pressure;            // 气压 (hPa)
//     float gas_resistance;      // 气体电阻 (Ω)
//     uint64_t timestamp;        // 时间戳(微秒)
// } EnvMsg;

// // IMU消息
// typedef struct {
//     MsgType msg_type;
//     float accel_x, accel_y, accel_z;    // 加速度 (m/s²)
//     float gyro_x, gyro_y, gyro_z;       // 陀螺仪 (rad/s)
//     uint64_t timestamp;
// } ImuMsg;

// // 电机状态消息（支持双电机）
// typedef struct {
//     MsgType msg_type;
//     // 电机1数据
//     float speed_rpm_1;        // 转速 (RPM)
//     float current_ma_1;       // 电流 (mA)
//     float position_deg_1;     // 位置 (°)
//     float voltage_1;          // 电压 (V)
    
//     // 电机2数据
//     float speed_rpm_2;        // 转速 (RPM)
//     float current_ma_2;       // 电流 (mA)
//     float position_deg_2;     // 位置 (°)
//     float voltage_2;          // 电压 (V)
    
//     // 通用数据
//     uint32_t alarm_code_1;    // 电机1告警码
//     uint32_t alarm_code_2;    // 电机2告警码
//     uint64_t timestamp;
// } MotorMsg;

// // 电机控制命令消息
// typedef struct {
//     MsgType msg_type;
//     // 电机1控制
//     float target_speed_1;     // 目标速度 (RPM)
//     float target_angle_1;     // 目标角度 (°)
//     bool enable_1;            // 使能状态
//     bool stop_1;              // 停止运行
//     // int direction_1;          // 方向 (0:顺时针, 1:逆时针)
    
//     // 电机2控制
//     float target_speed_2;     // 目标速度 (RPM)
//     float target_angle_2;     // 目标角度 (°)
//     bool enable_2;            // 使能状态
//     bool stop_2;              // 停止运行
//     // int direction_2;          // 方向 (0:顺时针, 1:逆时针)
    
//     // 通用命令
//     uint8_t control_mode;     // 控制模式 (1:位置, 3:速度, 4:电流)
// } MotorCmdMsg;

// // 系统命令消息
// typedef struct {
//     MsgType msg_type;
//     char cmd[64];             // 命令字符串
//     int param;                // 参数
// } SysCmdMsg;

// // POSIX消息队列句柄
// typedef struct {
//     mqd_t env_mq;
//     mqd_t motor_mq;
//     mqd_t imu_mq;
//     mqd_t cmd_mq;
// } MessageQueues;

// // 函数声明
// int msg_queue_init(MessageQueues* mqs);
// int msg_queue_send_env(mqd_t mq, const EnvMsg* msg);
// int msg_queue_send_motor(mqd_t mq, const MotorMsg* msg);
// int msg_queue_send_imu(mqd_t mq, const ImuMsg* msg);
// int msg_queue_send_cmd(mqd_t mq, const MotorCmdMsg* msg);
// void msg_queue_cleanup(MessageQueues* mqs);
// int msg_queue_get_count(mqd_t mq);
// // ipc_msg_queue.h - 修改部分
// // 通用接收函数声明
// int msg_queue_receive(mqd_t mq, void* buf, size_t buf_size, unsigned int* prio, bool block);
// int msg_queue_receive_env(mqd_t mq, EnvMsg* msg, bool block);
// int msg_queue_receive_motor(mqd_t mq, MotorMsg* msg, bool block);
// int msg_queue_receive_imu(mqd_t mq, ImuMsg* msg, bool block);
// int msg_queue_receive_cmd(mqd_t mq, MotorCmdMsg* msg, bool block);
// #endif // IPC_MSG_QUEUE_H


// ipc_msg_queue.h
#ifndef IPC_MSG_QUEUE_H
#define IPC_MSG_QUEUE_H

#include <mqueue.h>
#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

// POSIX消息队列名称
#define MQ_ENV_NAME     "/env_mq"
#define MQ_MOTOR_NAME   "/motor_mq"
#define MQ_IMU_NAME     "/imu_mq"
#define MQ_CMD_NAME     "/cmd_mq"
#define MQ_LOG_NAME     "/log_mq"      // <-- 新增：日志消息队列名称

// 最大消息数量和大小
#define MAX_MSGS        100
#define MAX_MSG_SIZE    1024

// 消息类型定义
typedef enum {
    MSG_TYPE_ENV_DATA = 1,     // 环境传感器数据
    MSG_TYPE_MOTOR_DATA,       // 电机状态数据  
    MSG_TYPE_IMU_DATA,         // IMU数据
    MSG_TYPE_MOTOR_CMD,        // 电机控制命令
    MSG_TYPE_SYS_CMD,          // 系统命令
    MSG_TYPE_LOG_MSG           // 日志消息（新增）
} MsgType;

// 环境传感器消息
typedef struct {
    MsgType msg_type;          // 消息类型
    float temperature;         // 温度 (°C)
    float humidity;            // 湿度 (%)
    float pressure;            // 气压 (hPa)
    float gas_resistance;      // 气体电阻 (Ω)
    uint64_t timestamp;        // 时间戳(微秒)
} EnvMsg;

// IMU消息
typedef struct {
    MsgType msg_type;
    float accel_x, accel_y, accel_z;    // 加速度 (m/s²)
    float gyro_x, gyro_y, gyro_z;       // 陀螺仪 (rad/s)
    uint64_t timestamp;
} ImuMsg;

// 电机状态消息（支持双电机）
typedef struct {
    MsgType msg_type;
    // 电机1数据
    float speed_rpm_1;        // 转速 (RPM)
    float current_ma_1;       // 电流 (mA)
    float position_deg_1;     // 位置 (°)
    float voltage_1;          // 电压 (V)
    
    // 电机2数据
    float speed_rpm_2;        // 转速 (RPM)
    float current_ma_2;       // 电流 (mA)
    float position_deg_2;     // 位置 (°)
    float voltage_2;          // 电压 (V)
    
    // 通用数据
    uint32_t alarm_code_1;    // 电机1告警码
    uint32_t alarm_code_2;    // 电机2告警码
    uint64_t timestamp;
} MotorMsg;

// 电机控制命令消息
typedef struct {
    MsgType msg_type;
    // 电机1控制
    float target_speed_1;     // 目标速度 (RPM)
    float target_angle_1;     // 目标角度 (°)
    bool enable_1;            // 使能状态
    bool stop_1;              // 停止运行
    
    // 电机2控制
    float target_speed_2;     // 目标速度 (RPM)
    float target_angle_2;     // 目标角度 (°)
    bool enable_2;            // 使能状态
    bool stop_2;              // 停止运行
    
    // 通用命令
    uint8_t control_mode;     // 控制模式 (1:位置, 3:速度, 4:电流)
} MotorCmdMsg;

// 系统命令消息
typedef struct {
    MsgType msg_type;
    char cmd[64];             // 命令字符串
    int param;                // 参数
} SysCmdMsg;

// >>>>>>>>>>>>>>>>>>>>>>>>>> 新增：日志消息结构 <<<<<<<<<<<<<<<<<<<<<<<<<<
typedef struct {
    MsgType msg_type;         // 必须为 MSG_TYPE_LOG_MSG
    char message[256];        // 日志内容
    uint64_t timestamp;       // 时间戳（可选）
} LogMsg;

// POSIX消息队列句柄
typedef struct {
    mqd_t env_mq;
    mqd_t motor_mq;
    mqd_t imu_mq;
    mqd_t cmd_mq;
    mqd_t log_mq;             // <-- 新增：日志消息队列句柄
} MessageQueues;

// 函数声明
int msg_queue_init(MessageQueues* mqs);
int msg_queue_send_env(mqd_t mq, const EnvMsg* msg);
int msg_queue_send_motor(mqd_t mq, const MotorMsg* msg);
int msg_queue_send_imu(mqd_t mq, const ImuMsg* msg);
int msg_queue_send_cmd(mqd_t mq, const MotorCmdMsg* msg);
int msg_queue_send_log(mqd_t mq, const LogMsg* msg);  // <-- 新增

void msg_queue_cleanup(MessageQueues* mqs);
int msg_queue_get_count(mqd_t mq);

// 通用接收函数声明
int msg_queue_receive(mqd_t mq, void* buf, size_t buf_size, unsigned int* prio, bool block);
int msg_queue_receive_env(mqd_t mq, EnvMsg* msg, bool block);
int msg_queue_receive_motor(mqd_t mq, MotorMsg* msg, bool block);
int msg_queue_receive_imu(mqd_t mq, ImuMsg* msg, bool block);
int msg_queue_receive_cmd(mqd_t mq, MotorCmdMsg* msg, bool block);
int msg_queue_receive_log(mqd_t mq, LogMsg* msg, bool block);  // <-- 新增

#endif // IPC_MSG_QUEUE_H