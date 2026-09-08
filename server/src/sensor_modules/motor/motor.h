// sensor_modules/motor/motor.h
#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>
#include <stdbool.h>

// 基础配置
#define CAN_IF "can0"
#define BAUD "1000000"
#define SERVO_ID_1 0x001
#define SERVO_ID_2 0x002
#define SERVO_ID_3 0x003
// 协议指令定义
#define CMD_WRITE       0x01
#define CMD_WRITE_ACK   0x02
#define CMD_READ        0x03
#define CMD_READ_ACK    0x04

// 写指令返回状态
#define WRITE_SUCCESS       0x01
#define WRITE_FAIL          0x00
#define WRITE_DATA_ERR      0x03
#define WRITE_OUT_OF_RANGE  0x04
#define WRITE_CORRECTED     0x05

// 寄存器地址
#define ADDR_CURRENT        0x05  // 当前电流 (mA)
#define ADDR_SPEED          0x06  // 当前速度 (脉冲/秒)
#define ADDR_POSITION       0x07  // 当前位置 (脉冲)
#define ADDR_TARGET_CURRENT 0x08  // 目标电流 (mA)
#define ADDR_TARGET_SPEED   0x09  // 目标速度 (脉冲/秒)
#define ADDR_TARGET_POS     0x0A  // 目标位置 (脉冲)
#define ADDR_ACCEL          0x0B  // 加速度 (脉冲/秒²)
#define ADDR_DECEL          0x0C  // 减速度 (脉冲/秒²)
#define ADDR_WORK_MODE      0x0F  // 工作模式
#define ADDR_ENABLE         0x10  // 使能/失能
#define ADDR_STOP_RUN       0x11  // 停止运行
#define ADDR_POSITION_OFFSET 0x12 // 位置偏移
#define ADDR_ALARM_CODE     0x15  // 告警代码
#define ADDR_BUS_VOLTAGE    0x1B  // 母线电压
#define ADDR_MAX_CURRENT    0x30  // 最大电流限制 (mA)
#define ADDR_MAX_SPEED      0x31  // 最大速度限制 (脉冲/秒)
#define ADDR_CAN_ID         0x4B  // CAN ID

// 工作模式定义
#define MODE_POSITION       0x01  // 位置模式
#define MODE_SPEED          0x03  // 速度模式
#define MODE_CURRENT        0x04  // 电流模式

// 角度范围
#define MIN_ANGLE          -180.0f
#define MAX_ANGLE          180.0f
#define PULSES_PER_REVOLUTION 65536  // 16位编码器

// 函数声明 - 修正类型
int motor_can_init(void);
int motor_can_send_cmd(uint32_t id, uint8_t cmd, uint8_t addr, uint32_t data);
int motor_fast_read(uint32_t id, uint8_t addr, uint32_t* result);
int motor_batch_read(uint32_t id, uint8_t* addrs, uint32_t* results, int count);
int motor_batch_write(uint32_t id, uint8_t* addrs, uint32_t* values, int count);

// 设备信息
void motor_read_device_info(uint32_t id);
void motor_read_real_time_status(uint32_t id);
int motor_get_current_position(uint32_t id, int32_t* pulse, float* angle);

// 运动控制
int motor_position_mode(uint32_t id, float target_degree, float speed_rpm, 
                       float accel_rpm_s, float decel_rpm_s);
int motor_speed_mode(uint32_t id, float speed_rpm, float accel_rpm_s, float decel_rpm_s);
int motor_current_mode(uint32_t id, int32_t current_ma);
int motor_force_position_mode(uint32_t id, float target_degree, float speed_rpm, 
                             int32_t max_current_ma);
int motor_enable(uint32_t id, int enable);
int motor_stop(uint32_t id);
int motor_set_protection(uint32_t id, uint32_t max_current_ma, uint32_t max_speed_pulse);

// 运动序列
int motor_execute_sequence(uint32_t id, float* angles, float* speeds, 
                          float* accels, int count, int delay_ms);

// 双电机控制
int motor_dual_move(float angle1, float angle2, float speed_rpm);

// 告警处理
int motor_check_alarm(uint32_t id);

// 转换函数
int32_t motor_degree_to_pulse(float degree);
float motor_pulse_to_degree(int32_t pulse);
uint32_t motor_rpm_to_pulse_per_sec(float rpm);
uint32_t motor_accel_rpm_to_pulse(float accel_rpm_s);

// 调试
void motor_set_debug_mode(int enable);

// 添加这些函数声明（motor.c 中已实现但可能未在头文件中声明）
int motor_position_mode(uint32_t id, float target_degree, float speed_rpm, 
                       float accel_rpm_s, float decel_rpm_s);
int motor_speed_mode(uint32_t id, float speed_rpm, float accel_rpm_s, float decel_rpm_s);
int motor_current_mode(uint32_t id, int32_t current_ma);
int motor_enable(uint32_t id, int enable);
int motor_stop(uint32_t id);
int motor_set_protection(uint32_t id, uint32_t max_current_ma, uint32_t max_speed_pulse);
int motor_dual_move(float angle1, float angle2, float speed_rpm);

#endif // MOTOR_H