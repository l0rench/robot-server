// ============================================================
// imu_config.cpp - 原极IMU参数配置模块
// 基于原极IMU通信协议手册_V1.3
// 功能：设置/读取IMU各项参数（频率、波特率、滤波器、坐标系等）
// ============================================================

#include "imu_config.h"
#include "serial_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

// ============================================================
// 1. 基本配置函数
// ============================================================

/**
 * 设置输出频率 (ODR)
 * @param fd 串口文件描述符
 * @param frequency 输出频率 (1, 10, 50, 100, 200, 500, 1000 Hz)
 * @return 0=成功, -1=失败
 */
int imu_set_odr(int fd, uint32_t frequency) {
    if (fd < 0) return -1;
    
    // 验证频率是否支持
    uint32_t valid_freq[] = {1, 10, 50, 100, 200, 500, 1000};
    int valid = 0;
    for (int i = 0; i < 7; i++) {
        if (frequency == valid_freq[i]) { valid = 1; break; }
    }
    if (!valid) {
        fprintf(stderr, "Error: Unsupported frequency %d. Supported: 1,10,50,100,200,500,1000\n", frequency);
        return -1;
    }
    
    printf("Setting ODR to %d Hz...\n", frequency);
    
    // CMD=14, param1=频率, param3=21(ODR索引)
    Send_CMD_LONG(14, (float)frequency, 0, PARAM_INDEX_ODR, 0, 0, 0);
    if (write(fd, &data_cmd_long, sizeof(data_cmd_long)) < 0) {
        perror("Write failed");
        return -1;
    }
    usleep(20000);
    
    return 0;
}

/**
 * 读取当前输出频率
 */
int imu_get_odr(int fd) {
    if (fd < 0) return -1;
    
    printf("Reading current ODR...\n");
    
    Send_CMD_LONG(6, 0, 0, PARAM_INDEX_ODR, 0, 0, 0);
    if (write(fd, &data_cmd_long, sizeof(data_cmd_long)) < 0) {
        perror("Write failed");
        return -1;
    }
    usleep(20000);
    
    printf("Please check response for ODR value (index 21)\n");
    return 0;
}

/**
 * 设置串口波特率
 */
int imu_set_baudrate(int fd, uint32_t baudrate) {
    if (fd < 0) return -1;
    
    uint32_t valid_baud[] = {115200, 230400, 460800, 921600, 1500000};
    int valid = 0;
    for (int i = 0; i < 5; i++) {
        if (baudrate == valid_baud[i]) { valid = 1; break; }
    }
    if (!valid) {
        fprintf(stderr, "Error: Unsupported baudrate %d\n", baudrate);
        return -1;
    }
    
    printf("Setting baudrate to %d bps...\n", baudrate);
    
    Send_CMD_LONG(14, (float)baudrate, 0, PARAM_INDEX_BAUDRATE, 0, 0, 0);
    if (write(fd, &data_cmd_long, sizeof(data_cmd_long)) < 0) {
        perror("Write failed");
        return -1;
    }
    usleep(20000);
    
    return 0;
}

/**
 * 设置滤波器
 */
int imu_set_filter(int fd, uint32_t filter_value) {
    if (fd < 0) return -1;
    
    printf("Setting filter to 0x%02X...\n", filter_value);
    
    Send_CMD_LONG(14, (float)filter_value, 0, PARAM_INDEX_FILTER, 0, 0, 0);
    if (write(fd, &data_cmd_long, sizeof(data_cmd_long)) < 0) {
        perror("Write failed");
        return -1;
    }
    usleep(20000);
    
    return 0;
}

/**
 * 设置坐标系朝向
 */
int imu_set_orientation(int fd, uint32_t orientation) {
    if (fd < 0) return -1;
    
    if (orientation < 101 || orientation > 124) {
        fprintf(stderr, "Error: Orientation must be between 101 and 124\n");
        return -1;
    }
    
    printf("Setting orientation to %d...\n", orientation);
    
    Send_CMD_LONG(14, (float)orientation, 0, PARAM_INDEX_ORIENTATION, 0, 0, 0);
    if (write(fd, &data_cmd_long, sizeof(data_cmd_long)) < 0) {
        perror("Write failed");
        return -1;
    }
    usleep(20000);
    
    return 0;
}

/**
 * 设置输出模式
 */
int imu_set_output_mode(int fd, uint32_t mode) {
    if (fd < 0) return -1;
    
    const char* mode_names[] = {"AHRS", "AHRS Extended", "Quaternion", "Command"};
    const char* name = "Unknown";
    if (mode == MODE_AHRS) name = mode_names[0];
    else if (mode == MODE_AHRS_EXTENDED) name = mode_names[1];
    else if (mode == MODE_QUATERNION) name = mode_names[2];
    else if (mode == MODE_COMMAND) name = mode_names[3];
    
    printf("Setting output mode to %s (%d)...\n", name, mode);
    
    Send_CMD_LONG(3, (float)mode, 0, 0, 0, 0, 0);
    if (write(fd, &data_cmd_long, sizeof(data_cmd_long)) < 0) {
        perror("Write failed");
        return -1;
    }
    usleep(20000);
    
    return 0;
}

/**
 * 保存参数到FLASH
 */
int imu_save_params(int fd) {
    if (fd < 0) return -1;
    
    printf("Saving parameters to FLASH...\n");
    
    Send_CMD_LONG(5, 0, 0, 0, 0, 0, 0);
    if (write(fd, &data_cmd_long, sizeof(data_cmd_long)) < 0) {
        perror("Write failed");
        return -1;
    }
    usleep(50000);
    
    return 0;
}

/**
 * 软件复位
 */
int imu_soft_reset(int fd) {
    if (fd < 0) return -1;
    
    printf("Software reset...\n");
    
    Send_CMD_LONG(9, 0, 0, 0, 0, 0, 0);
    if (write(fd, &data_cmd_long, sizeof(data_cmd_long)) < 0) {
        perror("Write failed");
        return -1;
    }
    usleep(50000);
    
    return 0;
}

/**
 * 触发获取AHRS数据（命令模式）
 */
int imu_trigger_ahrs(int fd) {
    if (fd < 0) return -1;
    
    Send_CMD_LONG(2, 0, 0, 0, 0, 0, 0);
    if (write(fd, &data_cmd_long, sizeof(data_cmd_long)) < 0) {
        perror("Write failed");
        return -1;
    }
    usleep(10000);
    
    return 0;
}

// ============================================================
// 2. 高级配置函数
// ============================================================

/**
 * 完整的IMU配置流程
 */
int imu_full_config(int fd, uint32_t odr, uint32_t filter, 
                    uint32_t orientation, uint32_t mode) {
    if (fd < 0) return -1;
    
    printf("\n========== IMU Full Configuration ==========\n");
    printf("ODR: %d Hz\n", odr);
    printf("Filter: 0x%02X\n", filter);
    printf("Orientation: %d\n", orientation);
    printf("Mode: %d\n", mode);
    printf("============================================\n\n");
    
    // 1. 停止数据流（进入命令模式）
    printf("Step 1: Stopping data stream...\n");
    if (imu_set_output_mode(fd, MODE_COMMAND) < 0) return -1;
    usleep(50000);
    
    // 2. 设置ODR
    printf("Step 2: Setting ODR...\n");
    if (imu_set_odr(fd, odr) < 0) return -1;
    usleep(20000);
    
    // 3. 设置滤波器
    printf("Step 3: Setting filter...\n");
    if (imu_set_filter(fd, filter) < 0) return -1;
    usleep(20000);
    
    // 4. 设置坐标系
    printf("Step 4: Setting orientation...\n");
    if (imu_set_orientation(fd, orientation) < 0) return -1;
    usleep(20000);
    
    // 5. 保存参数
    printf("Step 5: Saving parameters...\n");
    if (imu_save_params(fd) < 0) return -1;
    usleep(50000);
    
    // 6. 软件复位（使参数生效）
    printf("Step 6: Software reset...\n");
    if (imu_soft_reset(fd) < 0) return -1;
    usleep(100000);
    
    // 7. 设置输出模式
    printf("Step 7: Setting output mode...\n");
    if (imu_set_output_mode(fd, mode) < 0) return -1;
    usleep(50000);
    
    printf("\n========== Configuration Complete ==========\n");
    printf("IMU is now running at %d Hz\n", odr);
    printf("============================================\n");
    
    return 0;
}

// ============================================================
// 3. 快捷配置函数
// ============================================================

/**
 * 快速配置1000Hz输出（需要921600波特率）
 */
int imu_config_1000hz(int fd) {
    return imu_full_config(fd, 1000, FILTER_NO, 101, MODE_AHRS);
}

/**
 * 快速配置200Hz输出
 */
int imu_config_200hz(int fd) {
    return imu_full_config(fd, 200, FILTER_NO, 101, MODE_AHRS);
}

/**
 * 快速配置100Hz输出
 */
int imu_config_100hz(int fd) {
    return imu_full_config(fd, 100, FILTER_NO, 101, MODE_AHRS);
}

/**
 * 快速配置50Hz输出
 */
int imu_config_50hz(int fd) {
    return imu_full_config(fd, 50, FILTER_10HZ, 101, MODE_AHRS);
}

// ============================================================
// 4. 验证工具
// ============================================================

static int _frame_count = 0;
static time_t _last_check = 0;
static int _last_rate = 0;

/**
 * 验证输出频率（在数据接收循环中调用）
 */
void imu_verify_rate(void) {
    _frame_count++;
    
    time_t now = time(NULL);
    if (now - _last_check >= 1) {
        _last_rate = _frame_count;
        printf("[Rate Check] %d Hz\n", _frame_count);
        _frame_count = 0;
        _last_check = now;
    }
}

/**
 * 重置帧计数器
 */
void imu_reset_rate_counter(void) {
    _frame_count = 0;
    _last_check = time(NULL);
    _last_rate = 0;
}