// motor_api.h - 统一电机API接口
#ifndef MOTOR_API_H
#define MOTOR_API_H

#include "motor.h"

// 简化的API接口
typedef struct {
    uint32_t id;            // 电机ID
    char name[32];          // 电机名称
    bool enabled;           // 使能状态
    bool initialized;       // 初始化状态
} MotorDevice;

// 初始化电机系统
int motor_system_init(void);

// 初始化单个电机
int motor_init_device(uint32_t id, const char* name);

// 获取电机状态
int motor_get_status(uint32_t id, float* speed_rpm, float* current_ma, 
                     float* position_deg, float* voltage, uint32_t* alarm);

// 控制电机运动
int motor_move_to_angle(uint32_t id, float angle_deg, float speed_rpm);
int motor_set_speed(uint32_t id, float speed_rpm);
int motor_set_current(uint32_t id, float current_ma);

// 电机使能控制
int motor_enable_device(uint32_t id, bool enable);
int motor_stop_device(uint32_t id);

// 设置保护参数
int motor_set_limits(uint32_t id, float max_current_ma, float max_speed_rpm);

// 获取错误信息
const char* motor_get_error_string(int error_code);

// 双电机控制
int motor_dual_move_angles(float angle1, float angle2, float speed_rpm);

// 清理
void motor_system_cleanup(void);

#endif // MOTOR_API_H