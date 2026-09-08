// ============================================================
// imu_config.h - 原极IMU参数配置模块头文件
// 基于原极IMU通信协议手册_V1.3
// ============================================================

#ifndef IMU_CONFIG_H
#define IMU_CONFIG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// 参数索引（参考手册 1.1.2.2 节）
// ============================================================
#define PARAM_INDEX_BAUDRATE      3   // 串口波特率
#define PARAM_INDEX_ORIENTATION   4   // 坐标系朝向
#define PARAM_INDEX_GYRO_X_OFF    8   // X轴陀螺零偏
#define PARAM_INDEX_GYRO_Y_OFF    9   // Y轴陀螺零偏
#define PARAM_INDEX_GYRO_Z_OFF    10  // Z轴陀螺零偏
#define PARAM_INDEX_ODR           21  // AHRS输出频率 (Hz)
#define PARAM_INDEX_FILTER        31  // 内部滤波器配置

// ============================================================
// 输出模式（参考手册 1.5 节）
// ============================================================
#define MODE_AHRS                 1   // AHRS数据流
#define MODE_AHRS_EXTENDED        51  // AHRS扩展模式
#define MODE_QUATERNION           52  // 四元数模式
#define MODE_COMMAND              100 // 命令模式（停止数据流）

// ============================================================
// 滤波器配置（参考手册 1.6 节）
// ============================================================
#define FILTER_1HZ                0x11
#define FILTER_2HZ                0x22
#define FILTER_5HZ                0x33
#define FILTER_10HZ               0x44
#define FILTER_15HZ               0x55
#define FILTER_20HZ               0x66
#define FILTER_25HZ               0x77
#define FILTER_30HZ               0x88
#define FILTER_35HZ               0x99
#define FILTER_40HZ               0xAA
#define FILTER_NO                 0xBB

// ============================================================
// 波特率定义
// ============================================================
#define BAUD_115200               115200
#define BAUD_230400               230400
#define BAUD_460800               460800
#define BAUD_921600               921600
#define BAUD_1500000              1500000

// ============================================================
// 函数声明
// ============================================================

// 基本配置函数
int imu_set_odr(int fd, uint32_t frequency);
int imu_get_odr(int fd);
int imu_set_baudrate(int fd, uint32_t baudrate);
int imu_set_filter(int fd, uint32_t filter_value);
int imu_set_orientation(int fd, uint32_t orientation);
int imu_set_output_mode(int fd, uint32_t mode);
int imu_save_params(int fd);
int imu_soft_reset(int fd);
int imu_trigger_ahrs(int fd);

// 高级配置
int imu_full_config(int fd, uint32_t odr, uint32_t filter, 
                    uint32_t orientation, uint32_t mode);

// 快捷配置
int imu_config_200hz(int fd);
int imu_config_1000hz(int fd);
int imu_config_100hz(int fd);
int imu_config_50hz(int fd);

// 验证工具
void imu_verify_rate(void);
void imu_reset_rate_counter(void);

#ifdef __cplusplus
}
#endif

#endif // IMU_CONFIG_H