// sensor_modules/motor/motor_api.c
#include "motor_api.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "motor.h"

static MotorDevice g_motors[2] = {
    {SERVO_ID_1, "电机1", false, false},
    {SERVO_ID_2, "电机2", false, false},
    {SERVO_ID_3, "电机3", false, false}
};

static pthread_mutex_t g_motor_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool g_system_initialized = false;

// 初始化电机系统
int motor_system_init(void) {
    if (g_system_initialized) {
        return 0;
    }
    
    pthread_mutex_lock(&g_motor_mutex);
    
    printf("[MOTOR] 初始化电机系统...\n");
    
    // 初始化CAN总线
    if (motor_can_init() != 0) {
        printf("[MOTOR] CAN总线初始化失败\n");
        pthread_mutex_unlock(&g_motor_mutex);
        return -1;
    }
    
    // 设置保护参数
    for (int i = 0; i < 2; i++) {
        float max_current = (i == 0) ? 1400 : 500;
        float max_speed_rpm = (i == 0) ? 200 : 150;
        
        motor_set_protection(g_motors[i].id, 
                           (uint32_t)max_current,
                           motor_rpm_to_pulse_per_sec(max_speed_rpm));
    }
    
    g_system_initialized = true;
    printf("[MOTOR] 电机系统初始化完成\n");
    
    pthread_mutex_unlock(&g_motor_mutex);
    return 0;
}

// 初始化单个电机
int motor_init_device(uint32_t id, const char* name) {
    int motor_index = -1;
    
    // 查找电机
    for (int i = 0; i < 2; i++) {
        if (g_motors[i].id == id) {
            motor_index = i;
            break;
        }
    }
    
    if (motor_index == -1) {
        printf("[MOTOR] 无效的电机ID: 0x%X\n", id);
        return -1;
    }
    
    pthread_mutex_lock(&g_motor_mutex);
    
    if (name) {
        strncpy(g_motors[motor_index].name, name, sizeof(g_motors[motor_index].name) - 1);
    }
    
    // 读取设备信息确认连接
    printf("[MOTOR] 初始化 %s (ID: 0x%X)\n", g_motors[motor_index].name, id);
    motor_read_device_info(id);
    
    // 读取当前状态
    motor_read_real_time_status(id);
    
    g_motors[motor_index].initialized = true;
    
    pthread_mutex_unlock(&g_motor_mutex);
    
    return 0;
}

// 获取电机状态
int motor_get_status(uint32_t id, float* speed_rpm, float* current_ma, 
                     float* position_deg, float* voltage, uint32_t* alarm) {
    int motor_index = -1;
    
    for (int i = 0; i < 2; i++) {
        if (g_motors[i].id == id) {
            motor_index = i;
            break;
        }
    }
    
    if (motor_index == -1) {
        return -1;
    }
    
    pthread_mutex_lock(&g_motor_mutex);
    
    // 读取实时状态
    uint8_t addrs[] = {
        ADDR_CURRENT,
        ADDR_SPEED,
        ADDR_POSITION,
        ADDR_BUS_VOLTAGE,
        ADDR_ALARM_CODE
    };
    
    uint32_t results[5];
    
    if (motor_batch_read(id, addrs, results, 5) != 0) {
        pthread_mutex_unlock(&g_motor_mutex);
        return -1;
    }
    
    // 填充返回数据
    if (current_ma) *current_ma = (float)((int32_t)results[0]);
    if (speed_rpm) *speed_rpm = ((float)((int32_t)results[1]) * 60.0f) / PULSES_PER_REVOLUTION;
    if (position_deg) *position_deg = motor_pulse_to_degree((int32_t)results[2]);
    if (voltage) *voltage = results[3] * 0.1f;
    if (alarm) *alarm = results[4];
    
    pthread_mutex_unlock(&g_motor_mutex);
    
    return 0;
}

// 控制电机运动
int motor_move_to_angle(uint32_t id, float angle_deg, float speed_rpm) {
    if (angle_deg < MIN_ANGLE || angle_deg > MAX_ANGLE) {
        printf("[MOTOR] 角度 %.1f 超出范围 (%.0f~%.0f)\n", angle_deg, MIN_ANGLE, MAX_ANGLE);
        return -1;
    }
    
    if (speed_rpm <= 0) {
        printf("[MOTOR] 速度必须为正数\n");
        return -1;
    }
    
    // 使用默认加速度
    float default_accel = 1000.0f;
    
    pthread_mutex_lock(&g_motor_mutex);
    int result = motor_position_mode(id, angle_deg, speed_rpm, default_accel, default_accel);
    pthread_mutex_unlock(&g_motor_mutex);
    
    return result;
}

int motor_set_speed(uint32_t id, float speed_rpm) {
    float default_accel = 1000.0f;
    
    pthread_mutex_lock(&g_motor_mutex);
    int result = motor_speed_mode(id, speed_rpm, default_accel, default_accel);
    pthread_mutex_unlock(&g_motor_mutex);
    
    return result;
}

int motor_set_current(uint32_t id, float current_ma) {
    pthread_mutex_lock(&g_motor_mutex);
    int result = motor_current_mode(id, (int32_t)current_ma);
    pthread_mutex_unlock(&g_motor_mutex);
    
    return result;
}

// 电机使能控制
int motor_enable_device(uint32_t id, bool enable) {
    pthread_mutex_lock(&g_motor_mutex);
    int result = motor_enable(id, enable ? 1 : 0);
    
    if (result == 0) {
        // 更新状态
        for (int i = 0; i < 2; i++) {
            if (g_motors[i].id == id) {
                g_motors[i].enabled = enable;
                break;
            }
        }
    }
    
    pthread_mutex_unlock(&g_motor_mutex);
    return result;
}

int motor_stop_device(uint32_t id) {
    pthread_mutex_lock(&g_motor_mutex);
    int result = motor_stop(id);
    pthread_mutex_unlock(&g_motor_mutex);
    return result;
}

// 设置保护参数
int motor_set_limits(uint32_t id, float max_current_ma, float max_speed_rpm) {
    pthread_mutex_lock(&g_motor_mutex);
    int result = motor_set_protection(id, 
                                    (uint32_t)max_current_ma,
                                    motor_rpm_to_pulse_per_sec(max_speed_rpm));
    pthread_mutex_unlock(&g_motor_mutex);
    return result;
}

// 双电机控制
int motor_dual_move_angles(float angle1, float angle2, float speed_rpm) {
    pthread_mutex_lock(&g_motor_mutex);
    int result = motor_dual_move(angle1, angle2, speed_rpm);
    pthread_mutex_unlock(&g_motor_mutex);
    return result;
}

// 清理
void motor_system_cleanup(void) {
    pthread_mutex_lock(&g_motor_mutex);
    
    printf("[MOTOR] 清理电机系统...\n");
    
    // 停止并失能所有电机
    for (int i = 0; i < 2; i++) {
        if (g_motors[i].initialized && g_motors[i].enabled) {
            motor_stop(g_motors[i].id);
            motor_enable(g_motors[i].id, 0);
            g_motors[i].enabled = false;
        }
    }
    
    g_system_initialized = false;
    
    pthread_mutex_unlock(&g_motor_mutex);
}

// 获取错误信息
const char* motor_get_error_string(int error_code) {
    switch (error_code) {
        case 0: return "成功";
        case -1: return "通用错误";
        case -2: return "CAN通信错误";
        case -3: return "电机未初始化";
        case -4: return "参数错误";
        case -5: return "超时";
        default: return "未知错误";
    }
}