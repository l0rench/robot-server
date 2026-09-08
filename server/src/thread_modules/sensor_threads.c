// #include "shared_data.h"
// #include "motor_api.h"
// #include "bme68x.h"
// #include "bme68x_defs.h"
// #include "ipc_msg_queue.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <time.h>
// #include <string.h>
// #include <fcntl.h>
// #include <linux/i2c-dev.h>
// #include <sys/ioctl.h>
// #include <errno.h>
// #include <math.h>
// #include <mqueue.h>
// #include "serial_port.h"

// // I2C读写函数（与你的main.c一致）
// static void user_delay_us(uint32_t period, void *intf_ptr) {
//     usleep(period);
// }

// static int8_t user_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr) {
//     int fd = *((int*)intf_ptr);
//     if (write(fd, &reg_addr, 1) != 1) {
//         return -1;
//     }
//     if (read(fd, data, len) != (ssize_t)len) {
//         return -1;
//     }
//     return 0;
// }

// static int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr) {
//     int fd = *((int*)intf_ptr);
//     uint8_t buf[len + 1];
//     buf[0] = reg_addr;
//     memcpy(buf + 1, data, len);
    
//     if (write(fd, buf, len + 1) != (ssize_t)(len + 1)) {
//         return -1;
//     }
//     return 0;
// }

// // sensor_threads.c - 保持 get_timestamp_us 为 static
// uint64_t get_timestamp_us(void) {
//     struct timespec ts;
//     clock_gettime(CLOCK_MONOTONIC, &ts);
//     return (uint64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
// }

// // 辅助函数：安全发送并打印详细错误-add by fxh
// static void log_mq_error(const char* thread_name, const char* msg_type) {
//     fprintf(stderr, "[%s] 发送 %s 失败: errno=%d (%s)\n",
//             thread_name, msg_type, errno, strerror(errno));
// }

// // 环境传感器线程 - 使用真实BME68X传感器
// void* env_sensor_thread(void* arg) {
//     GlobalData* g_data = (GlobalData*)arg;
    
//     printf("[ENV_THREAD] 环境传感器线程启动\n");
    
//     if (g_data == NULL) {
//         printf("[ENV_THREAD] 错误：未传递有效的全局数据\n");
//         return NULL;
//     }
    
//     // 1. 初始化BME68X传感器
//     struct bme68x_dev dev;
//     static uint8_t dev_addr = BME68X_I2C_ADDR_HIGH;  // 0x77
//     int8_t rslt = BME68X_OK;
    
//     // 打开I2C设备
//     int i2c_fd = -1;
//     const char* i2c_device = "/dev/i2c-6";  // 根据实际情况调整
    
//     if ((i2c_fd = open(i2c_device, O_RDWR)) < 0) {
//         printf("[ENV_THREAD] 错误：无法打开I2C设备: %s (%s)\n", 
//                i2c_device, strerror(errno));
//         printf("[ENV_THREAD] BME68X传感器未连接，线程退出\n");
//         return NULL;  // 直接退出，不尝试模拟数据
//     }
    
//     if (ioctl(i2c_fd, I2C_SLAVE, dev_addr) < 0) {
//         printf("[ENV_THREAD] 错误：无法设置I2C地址: 0x%02X (%s)\n", 
//                dev_addr, strerror(errno));
//         close(i2c_fd);
//         printf("[ENV_THREAD] BME68X传感器未响应，线程退出\n");
//         return NULL;  // 直接退出
//     }
    
//     // 初始化设备结构
//     dev.intf_ptr = &i2c_fd;  // 传递文件描述符
//     dev.intf = BME68X_I2C_INTF;
//     dev.read = user_i2c_read;
//     dev.write = user_i2c_write;
//     dev.delay_us = user_delay_us;
    
//     // 初始化传感器
//     rslt = bme68x_init(&dev);
//     if (rslt != BME68X_OK) {
//         printf("[ENV_THREAD] 错误：BME68X传感器初始化失败: %d\n", rslt);
//         close(i2c_fd);
//         printf("[ENV_THREAD] BME68X传感器初始化失败，线程退出\n");
//         return NULL;  // 直接退出
//     }
    
//     // printf("[ENV_THREAD] BME68X传感器初始化成功\n");
    
//     // 2. 传感器配置（使用室内导航推荐配置）
//     struct bme68x_conf conf;
//     struct bme68x_heatr_conf heatr_conf;
    
//     conf.os_hum = BME68X_OS_1X;
//     conf.os_pres = BME68X_OS_16X;
//     conf.os_temp = BME68X_OS_2X;
//     conf.filter = BME68X_FILTER_SIZE_15;
    
//     heatr_conf.enable = BME68X_ENABLE;
//     heatr_conf.heatr_temp = 300;
//     heatr_conf.heatr_dur = 100;
    
//     // 设置配置
//     if (bme68x_set_conf(&conf, &dev) != BME68X_OK) {
//         printf("[ENV_THREAD] 错误：传感器配置失败\n");
//         close(i2c_fd);
//         printf("[ENV_THREAD] 线程退出\n");
//         return NULL;
//     }
    
//     if (bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &dev) != BME68X_OK) {
//         printf("[ENV_THREAD] 错误：加热器配置失败\n");
//         close(i2c_fd);
//         printf("[ENV_THREAD] 线程退出\n");
//         return NULL;
//     }
    
//     // printf("[ENV_THREAD] 开始采集环境传感器数据...\n");
    
//     // 3. 主循环：读取传感器数据并发送到消息队列
//     while (g_data->system_running) {
//         struct bme68x_data sensor_data;
//         uint8_t n_fields = 0;
//         uint32_t del_period;
        
//         // 设置为强制模式
//         if (bme68x_set_op_mode(BME68X_FORCED_MODE, &dev) != BME68X_OK) {
//             printf("[ENV_THREAD] 设置强制模式失败\n");
//             sleep(1);
//             continue;
//         }
        
//         // 计算并等待测量完成
//         del_period = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &dev) + (heatr_conf.heatr_dur * 1000);
//         dev.delay_us(del_period, dev.intf_ptr);
        
//         // 读取数据
//         rslt = bme68x_get_data(BME68X_FORCED_MODE, &sensor_data, &n_fields, &dev);
//         if (rslt == BME68X_OK && n_fields > 0) {
//             EnvMsg env_msg = {0}; // 清零整个结构体
//             env_msg.msg_type = MSG_TYPE_ENV_DATA;
            
//             #ifdef BME68X_USE_FPU
//             env_msg.temperature = sensor_data.temperature;
//             env_msg.humidity = sensor_data.humidity;
//             env_msg.pressure = sensor_data.pressure / 100.0f;
//             env_msg.gas_resistance = sensor_data.gas_resistance;
//             #else
//             env_msg.temperature = (float)sensor_data.temperature / 100.0f;
//             env_msg.humidity = (float)sensor_data.humidity / 1000.0f;
//             env_msg.pressure = (float)sensor_data.pressure / 100.0f;
//             env_msg.gas_resistance = (float)sensor_data.gas_resistance;
//             #endif
//             env_msg.timestamp = get_timestamp_us();

//             printf("[ENV_THREAD] 真实数据: %.1f°C, %.1f%%, %.1fhPa, %.0fΩ\n",
//                    env_msg.temperature, env_msg.humidity, 
//                    env_msg.pressure, env_msg.gas_resistance);
//             if (msg_queue_send_env(g_data->msg_queues.env_mq, &env_msg) != 0) {
//                 log_mq_error("ENV_THREAD", "环境数据");
//             }
//         } else {
//             printf("[ENV_THREAD] 读取传感器数据失败: rslt=%d, n_fields=%d\n", rslt, n_fields);
//         }
        
//         sleep(1);
//     }
    
//     // 清理资源
//     close(i2c_fd);
//     // printf("[ENV_THREAD] 环境传感器线程退出\n");
//     return NULL;
// }

// // IMU采集线程（使用POSIX消息队列）
// void* imu_collector_thread(void* arg) {
//     GlobalData* g_data = (GlobalData*)arg;
    
//     printf("[IMU_THREAD] IMU采集线程启动\n");
    
//     if (g_data == NULL) {
//         // printf("[IMU_THREAD] 错误：未传递有效的全局数据\n");
//         return NULL;
//     }
//     // 检查队列是否有效
//     if (g_data->msg_queues.imu_mq == (mqd_t)-1) {
//         printf("[IMU_THREAD] 错误：IMU消息队列未初始化！\n");
//         return NULL;
//     }
//     // 1. 初始化IMU传感器
//     const char* imu_port = "/dev/ttyS5";  // 根据你的系统调整
//     int baud_rate = 115200;
    
//     if (imu_init(imu_port, baud_rate) != 0) {
//         // printf("[IMU_THREAD] IMU初始化失败，使用模拟数据\n");
//         goto use_simulated_data;
//     }
    
//     // printf("[IMU_THREAD] IMU传感器初始化成功\n");
    
//     // 2. 主循环：读取IMU数据并发送到消息队列
//     IMUData imu_raw_data;
    
//     while (g_data->system_running) {
//         // 读取IMU数据
//         int result = imu_read(&imu_raw_data);
        
//         if (result > 0 && imu_raw_data.data_valid) {
//             // 创建IMU消息
//             ImuMsg imu_msg = {0};
//             imu_msg.msg_type = MSG_TYPE_IMU_DATA;
            
//             // 填充真实传感器数据
//             imu_msg.accel_x = imu_raw_data.accel_x;
//             imu_msg.accel_y = imu_raw_data.accel_y;
//             imu_msg.accel_z = imu_raw_data.accel_z;
            
//             imu_msg.gyro_x = imu_raw_data.gyro_x;
//             imu_msg.gyro_y = imu_raw_data.gyro_y;
//             imu_msg.gyro_z = imu_raw_data.gyro_z;
            
//             imu_msg.timestamp = imu_raw_data.timestamp;
            
//             // 调试输出
//             // printf("[IMU_THREAD] 真实数据: "
//             //        "Accel(%.2f,%.2f,%.2f) m/s², "
//             //        "Gyro(%.2f,%.2f,%.2f) rad/s\n",
//             //        imu_msg.accel_x, imu_msg.accel_y, imu_msg.accel_z,
//             //        imu_msg.gyro_x, imu_msg.gyro_y, imu_msg.gyro_z);
            
//             // 发送到POSIX消息队列
//             if (msg_queue_send_imu(g_data->msg_queues.imu_mq, &imu_msg) != 0) {
//                 log_mq_error("IMU_THREAD", "IMU数据");
//             }
//         } else if (result < 0) {
//             // IMU读取错误
//             printf("[IMU_THREAD] IMU读取错误，切换到模拟数据\n");
//             // imu_close();
//             // goto use_simulated_data;
//         }
        
//         // 采集频率：100Hz（根据需要调整）
//         usleep(10000);  // 10ms
//     }
    
//     // 3. 清理资源
//     imu_close();
//     // printf("[IMU_THREAD] IMU采集线程退出\n");
//     return NULL;
    
// use_simulated_data:
//     // 4. 备选方案：使用模拟数据
//     // printf("[IMU_THREAD] 使用模拟数据\n");
    
//     srand(time(NULL));
    
//     while (g_data->system_running) {
//         ImuMsg imu_msg;
//         imu_msg.msg_type = MSG_TYPE_IMU_DATA;
        
//         static int count = 0;
        
//         // 模拟轻微震动
//         count++;
//         float vibration = sinf(count * 0.1f) * 0.05f;
        
//         imu_msg.accel_x = ((rand() % 100) - 50) * 0.01f + vibration;
//         imu_msg.accel_y = ((rand() % 100) - 50) * 0.01f + vibration;
//         imu_msg.accel_z = 9.8f + ((rand() % 100) - 50) * 0.01f;  // 重力加速度
        
//         imu_msg.gyro_x = ((rand() % 100) - 50) * 0.01f;
//         imu_msg.gyro_y = ((rand() % 100) - 50) * 0.01f;
//         imu_msg.gyro_z = ((rand() % 100) - 50) * 0.01f;
        
//         imu_msg.timestamp = get_timestamp_us();
        
//         // 发送到POSIX消息队列
//         if (msg_queue_send_imu(g_data->msg_queues.imu_mq, &imu_msg) != 0) {
//             // printf("[IMU_THREAD] 发送IMU数据失败\n");
//         }
        
//         // 采集频率：10Hz
//         // usleep(100000);
//         usleep(500000);
//     }
    
//     // printf("[IMU_THREAD] IMU采集线程退出\n");
//     return NULL;
// }

// // 电机状态线程（使用POSIX消息队列）
// void* motor_status_thread(void* arg) {
//     GlobalData* g_data = (GlobalData*)arg;
    
//     // printf("[MOTOR_THREAD] 电机状态线程启动\n");
    
//     if (g_data == NULL) {
//         // printf("[MOTOR_THREAD] 错误：未传递有效的全局数据\n");
//         return NULL;
//     }
    
//     srand(time(NULL));
    
//     while (g_data->system_running) {
//         MotorMsg motor_msg;
//         motor_msg.msg_type = MSG_TYPE_MOTOR_DATA;
        
//         // 尝试读取真实电机状态
//         if (g_data->motors_initialized) {
//             // 读取真实数据
//             float speed1 = 0, current1 = 0, pos1 = 0, volt1 = 0;
//             uint32_t alarm1 = 0;
            
//             float speed2 = 0, current2 = 0, pos2 = 0, volt2 = 0;
//             uint32_t alarm2 = 0;
            
//             if (motor_get_status(SERVO_ID_1, &speed1, &current1, &pos1, &volt1, &alarm1) == 0 &&
//                 motor_get_status(SERVO_ID_2, &speed2, &current2, &pos2, &volt2, &alarm2) == 0) {
                
//                 motor_msg.speed_rpm_1 = speed1;
//                 motor_msg.current_ma_1 = current1;
//                 motor_msg.position_deg_1 = pos1;
//                 motor_msg.voltage_1 = volt1;
//                 motor_msg.alarm_code_1 = alarm1;
                
//                 motor_msg.speed_rpm_2 = speed2;
//                 motor_msg.current_ma_2 = current2;
//                 motor_msg.position_deg_2 = pos2;
//                 motor_msg.voltage_2 = volt2;
//                 motor_msg.alarm_code_2 = alarm2;
                
//             } else {
//                 goto use_simulated_data;
//             }
//         } else {
// use_simulated_data:
//             // 使用模拟数据
//             static float sim_speed1 = 1500.0f;
//             static float sim_speed2 = 1200.0f;
            
//             sim_speed1 += ((rand() % 200) - 100);
//             sim_speed2 += ((rand() % 200) - 100);
            
//             sim_speed1 = fmaxf(0.0f, fminf(3000.0f, sim_speed1));
//             sim_speed2 = fmaxf(0.0f, fminf(3000.0f, sim_speed2));
            
//             motor_msg.speed_rpm_1 = sim_speed1;
//             motor_msg.speed_rpm_2 = sim_speed2;
//             motor_msg.current_ma_1 = 1000.0f + (rand() % 500);
//             motor_msg.current_ma_2 = 800.0f + (rand() % 400);
//             motor_msg.position_deg_1 = (float)(rand() % 3600) / 10.0f;
//             motor_msg.position_deg_2 = (float)(rand() % 3600) / 10.0f;
//             motor_msg.voltage_1 = 48.0f + (rand() % 20) * 0.1f;
//             motor_msg.voltage_2 = 48.0f + (rand() % 20) * 0.1f;
//             motor_msg.alarm_code_1 = (rand() % 100 == 0) ? 0x01 : 0;
//             motor_msg.alarm_code_2 = (rand() % 100 == 0) ? 0x01 : 0;
//         }
        
//         motor_msg.timestamp = get_timestamp_us();
        
//         // 发送到POSIX消息队列
//         if (msg_queue_send_motor(g_data->msg_queues.motor_mq, &motor_msg) != 0) {
//             // printf("[MOTOR_THREAD] 发送电机数据失败\n");
//         }
        
//         // 更新频率：2Hz
//         usleep(500000);
//     }
    
//     // printf("[MOTOR_THREAD] 电机状态线程退出\n");
//     return NULL;
// }

#include "shared_data.h"
#include "motor_api.h"
#include "bme68x.h"
#include "bme68x_defs.h"
#include "ipc_msg_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <math.h>
#include <mqueue.h>
#include "serial_port.h"

// I2C读写函数
static void user_delay_us(uint32_t period, void *intf_ptr) {
    usleep(period);
}

static int8_t user_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr) {
    int fd = *((int*)intf_ptr);
    if (write(fd, &reg_addr, 1) != 1) {
        return -1;
    }
    if (read(fd, data, len) != (ssize_t)len) {
        return -1;
    }
    return 0;
}

static int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr) {
    int fd = *((int*)intf_ptr);
    uint8_t buf[len + 1];
    buf[0] = reg_addr;
    memcpy(buf + 1, data, len);
    
    if (write(fd, buf, len + 1) != (ssize_t)(len + 1)) {
        return -1;
    }
    return 0;
}

// 获取微秒时间戳
uint64_t get_timestamp_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

// 辅助函数：安全发送并打印详细错误
static void log_mq_error(const char* thread_name, const char* msg_type) {
    fprintf(stderr, "[%s] 发送 %s 失败: errno=%d (%s)\n",
            thread_name, msg_type, errno, strerror(errno));
}

// 环境传感器线程 - 使用真实BME68X传感器
void* env_sensor_thread(void* arg) {
    GlobalData* g_data = (GlobalData*)arg;
    
    printf("[ENV_THREAD] 环境传感器线程启动\n");
    
    if (g_data == NULL) {
        printf("[ENV_THREAD] 错误：未传递有效的全局数据\n");
        return NULL;
    }
    
    // 1. 初始化BME68X传感器
    struct bme68x_dev dev;
    static uint8_t dev_addr = BME68X_I2C_ADDR_HIGH;  // 0x77
    int8_t rslt = BME68X_OK;
    
    // 打开I2C设备
    int i2c_fd = -1;
    const char* i2c_device = "/dev/i2c-6";  // 根据实际情况调整
    
    if ((i2c_fd = open(i2c_device, O_RDWR)) < 0) {
        printf("[ENV_THREAD] 错误：无法打开I2C设备: %s (%s)\n", 
               i2c_device, strerror(errno));
        printf("[ENV_THREAD] BME68X传感器未连接，线程退出\n");
        return NULL;
    }
    
    if (ioctl(i2c_fd, I2C_SLAVE, dev_addr) < 0) {
        printf("[ENV_THREAD] 错误：无法设置I2C地址: 0x%02X (%s)\n", 
               dev_addr, strerror(errno));
        close(i2c_fd);
        printf("[ENV_THREAD] BME68X传感器未响应，线程退出\n");
        return NULL;
    }
    
    // 初始化设备结构
    dev.intf_ptr = &i2c_fd;
    dev.intf = BME68X_I2C_INTF;
    dev.read = user_i2c_read;
    dev.write = user_i2c_write;
    dev.delay_us = user_delay_us;
    
    // 初始化传感器
    rslt = bme68x_init(&dev);
    if (rslt != BME68X_OK) {
        printf("[ENV_THREAD] 错误：BME68X传感器初始化失败: %d\n", rslt);
        close(i2c_fd);
        printf("[ENV_THREAD] BME68X传感器初始化失败，线程退出\n");
        return NULL;
    }
    
    // 2. 传感器配置（使用室内导航推荐配置）
    struct bme68x_conf conf;
    struct bme68x_heatr_conf heatr_conf;
    
    conf.os_hum = BME68X_OS_1X;
    conf.os_pres = BME68X_OS_16X;
    conf.os_temp = BME68X_OS_2X;
    conf.filter = BME68X_FILTER_SIZE_15;
    
    heatr_conf.enable = BME68X_ENABLE;
    heatr_conf.heatr_temp = 300;
    heatr_conf.heatr_dur = 100;
    
    if (bme68x_set_conf(&conf, &dev) != BME68X_OK) {
        printf("[ENV_THREAD] 错误：传感器配置失败\n");
        close(i2c_fd);
        return NULL;
    }
    
    if (bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &dev) != BME68X_OK) {
        printf("[ENV_THREAD] 错误：加热器配置失败\n");
        close(i2c_fd);
        return NULL;
    }
    
    // 3. 主循环：读取传感器数据并发送到消息队列
    while (g_data->system_running) {
        struct bme68x_data sensor_data;
        uint8_t n_fields = 0;
        uint32_t del_period;
        
        if (bme68x_set_op_mode(BME68X_FORCED_MODE, &dev) != BME68X_OK) {
            printf("[ENV_THREAD] 设置强制模式失败\n");
            sleep(1);
            continue;
        }
        
        del_period = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &dev) + (heatr_conf.heatr_dur * 1000);
        dev.delay_us(del_period, dev.intf_ptr);
        
        rslt = bme68x_get_data(BME68X_FORCED_MODE, &sensor_data, &n_fields, &dev);
        if (rslt == BME68X_OK && n_fields > 0) {
            EnvMsg env_msg = {0};
            env_msg.msg_type = MSG_TYPE_ENV_DATA;
            
#ifdef BME68X_USE_FPU
            env_msg.temperature = sensor_data.temperature;
            env_msg.humidity = sensor_data.humidity;
            env_msg.pressure = sensor_data.pressure / 100.0f;
            env_msg.gas_resistance = sensor_data.gas_resistance;
#else
            env_msg.temperature = (float)sensor_data.temperature / 100.0f;
            env_msg.humidity = (float)sensor_data.humidity / 1000.0f;
            env_msg.pressure = (float)sensor_data.pressure / 100.0f;
            env_msg.gas_resistance = (float)sensor_data.gas_resistance;
#endif
            env_msg.timestamp = get_timestamp_us();

            printf("[ENV_THREAD] 真实数据: %.1f°C, %.1f%%, %.1fhPa, %.0fΩ\n",
                   env_msg.temperature, env_msg.humidity, 
                   env_msg.pressure, env_msg.gas_resistance);
                   
            if (msg_queue_send_env(g_data->msg_queues.env_mq, &env_msg) != 0) {
                log_mq_error("ENV_THREAD", "环境数据");
            }
        } else {
            printf("[ENV_THREAD] 读取传感器数据失败: rslt=%d, n_fields=%d\n", rslt, n_fields);
        }
        
        sleep(1);
    }
    
    // 清理资源
    close(i2c_fd);
    printf("[ENV_THREAD] 环境传感器线程退出\n");
    return NULL;
}

// IMU采集线程（仅使用真实IMU）
void* imu_collector_thread(void* arg) {
    GlobalData* g_data = (GlobalData*)arg;
    
    printf("[IMU_THREAD] IMU采集线程启动\n");
    
    if (g_data == NULL) {
        printf("[IMU_THREAD] 错误：未传递有效的全局数据\n");
        return NULL;
    }
    
    if (g_data->msg_queues.imu_mq == (mqd_t)-1) {
        printf("[IMU_THREAD] 错误：IMU消息队列未初始化！\n");
        return NULL;
    }
    
    // 1. 初始化IMU传感器
    const char* imu_port = "/dev/ttyS5";
    int baud_rate = 115200;
    
    if (imu_init(imu_port, baud_rate) != 0) {
        printf("[IMU_THREAD] IMU初始化失败，线程退出\n");
        return NULL;
    }
    
    printf("[IMU_THREAD] IMU传感器初始化成功\n");
    
    // 2. 主循环：读取IMU数据并发送到消息队列
    IMUData imu_raw_data;
    
    while (g_data->system_running) {
        int result = imu_read(&imu_raw_data);
        
        if (result > 0 && imu_raw_data.data_valid) {
            ImuMsg imu_msg = {0};
            imu_msg.msg_type = MSG_TYPE_IMU_DATA;
            
            imu_msg.accel_x = imu_raw_data.accel_x;
            imu_msg.accel_y = imu_raw_data.accel_y;
            imu_msg.accel_z = imu_raw_data.accel_z;
            
            imu_msg.gyro_x = imu_raw_data.gyro_x;
            imu_msg.gyro_y = imu_raw_data.gyro_y;
            imu_msg.gyro_z = imu_raw_data.gyro_z;
            
            imu_msg.timestamp = imu_raw_data.timestamp;
            
            if (msg_queue_send_imu(g_data->msg_queues.imu_mq, &imu_msg) != 0) {
                log_mq_error("IMU_THREAD", "IMU数据");
            }
        } else if (result < 0) {
            printf("[IMU_THREAD] IMU读取错误，线程退出\n");
            break;
        }
        
        usleep(10000);  // 10ms → ~100Hz
    }
    
    imu_close();
    printf("[IMU_THREAD] IMU采集线程退出\n");
    return NULL;
}

// 电机状态线程（仅使用真实电机状态）
void* motor_status_thread(void* arg) {
    GlobalData* g_data = (GlobalData*)arg;
    
    printf("[MOTOR_THREAD] 电机状态线程启动\n");
    
    if (g_data == NULL) {
        printf("[MOTOR_THREAD] 错误：未传递有效的全局数据\n");
        return NULL;
    }
    
    if (!g_data->motors_initialized) {
        printf("[MOTOR_THREAD] 电机未初始化，线程退出\n");
        return NULL;
    }
    
    while (g_data->system_running) {
        MotorMsg motor_msg = {0};
        motor_msg.msg_type = MSG_TYPE_MOTOR_DATA;
        
        float speed1 = 0, current1 = 0, pos1 = 0, volt1 = 0;
        uint32_t alarm1 = 0;
        float speed2 = 0, current2 = 0, pos2 = 0, volt2 = 0;
        uint32_t alarm2 = 0;
        
        if (motor_get_status(SERVO_ID_1, &speed1, &current1, &pos1, &volt1, &alarm1) != 0 ||
            motor_get_status(SERVO_ID_2, &speed2, &current2, &pos2, &volt2, &alarm2) != 0) {
            printf("[MOTOR_THREAD] 读取电机状态失败，线程退出\n");
            break;
        }
        
        motor_msg.speed_rpm_1 = speed1;
        motor_msg.current_ma_1 = current1;
        motor_msg.position_deg_1 = pos1;
        motor_msg.voltage_1 = volt1;
        motor_msg.alarm_code_1 = alarm1;
        
        motor_msg.speed_rpm_2 = speed2;
        motor_msg.current_ma_2 = current2;
        motor_msg.position_deg_2 = pos2;
        motor_msg.voltage_2 = volt2;
        motor_msg.alarm_code_2 = alarm2;
        
        motor_msg.timestamp = get_timestamp_us();
        
        if (msg_queue_send_motor(g_data->msg_queues.motor_mq, &motor_msg) != 0) {
            log_mq_error("MOTOR_THREAD", "电机数据");
        }
        
        usleep(500000);  // 2Hz
    }
    
    printf("[MOTOR_THREAD] 电机状态线程退出\n");
    return NULL;
}