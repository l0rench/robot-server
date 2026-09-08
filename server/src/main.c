// // main.c - 修复函数声明
// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <pthread.h>
// #include <signal.h>
// #include <string.h>
// #include <time.h>
// #include "shared_data.h"
// #include "motor_api.h"
// #include "bme68x.h"
// #include "bme68x_defs.h"
// #include "ipc_msg_queue.h"
// #include "motor.h"
// #include <sys/types.h>
// #include <sys/wait.h>
// #include <stdint.h>
// #include <stdarg.h>
// #include <math.h>
// #include <signal.h>
// #include <time.h>
// #include <fcntl.h>
// #include <errno.h>
// #include "serial_port.h"
// // 声明包装函数
// int motor_batch_read(uint32_t id, uint8_t* addrs, uint32_t* results, int count);
// int motor_position_mode(uint32_t id, float target_degree, float speed_rpm, 
//                        float accel_rpm_s, float decel_rpm_s);
// int motor_speed_mode(uint32_t id, float speed_rpm, float accel_rpm_s, float decel_rpm_s);
// int motor_current_mode(uint32_t id, int32_t current_ma);
// int motor_enable(uint32_t id, int enable);
// int motor_stop(uint32_t id);
// int motor_dual_move(float angle1, float angle2, float speed_rpm);
// // 声明所有线程函数
// void* env_sensor_thread(void* arg);
// void* imu_collector_thread(void* arg);
// void* motor_status_thread(void* arg);
// void* display_thread(void* arg);
// void* tcp_report_thread(void* arg);
// void* motor_control_thread(void* arg);

// // 全局变量
// GlobalData g_data;
// volatile sig_atomic_t keep_running = 1;

// // 信号处理函数
// void signal_handler(int sig) {
//     // printf("\n收到信号 %d，正在关闭...\n", sig);
//     keep_running = 0;
//     g_data.system_running = false;
// }


// // 连接服务器 - 声明（在 tcp_client.c 中实现）
// bool connect_to_server(int* sockfd);

// // 初始化默认配置
// void init_default_config(SystemConfig* config) {
//     memset(config, 0, sizeof(SystemConfig));
    
//     // 网络配置
//     strcpy(config->server_ip, "127.0.0.1");
//     config->server_port = 8080;
    
//     // 电机保护参数
//     config->max_current_1 = 1400;  // 电机1最大电流 1400mA
//     config->max_current_2 = 1400;   // 电机2最大电流 500mA
//     config->max_speed_1 = 200;     // 电机1最大速度 200RPM
//     config->max_speed_2 = 200;     // 电机2最大速度 150RPM
    
//     // 默认运动参数
//     config->default_accel = 1000.0f;  // 1000 RPM/s²
//     config->default_decel = 1000.0f;  // 1000 RPM/s²
    
//     // 系统配置
//     config->debug_mode = 0;
//     config->can_baudrate = 1000000;  // 1Mbps
// }

// // 初始化消息队列系统
// void init_msg_queue_system(GlobalData* g_data) {
//     // printf("[MAIN] 初始化消息队列系统...\n");
    
//     // 使用POSIX消息队列初始化
//     if (msg_queue_init(&g_data->msg_queues) != 0) {
//         printf("[MAIN] 消息队列初始化失败\n");
//         exit(1);
//     }
    
//     printf("[MAIN] 消息队列系统初始化完成\n");
//     // printf("  环境队列: %d\n", g_data->msg_queues.env_mq);
//     // printf("  电机队列: %d\n", g_data->msg_queues.motor_mq);
//     // printf("  IMU队列: %d\n", g_data->msg_queues.imu_mq);
//     // printf("  命令队列: %d\n", g_data->msg_queues.cmd_mq);
// }

// // 初始化电机系统
// void init_motor_system(GlobalData* g_data) {
//     // printf("[MAIN] 初始化电机系统...\n");
    
//     // 初始化电机API
//     if (motor_system_init() != 0) {
//         // printf("[MAIN] 电机系统初始化失败\n");
//         return;
//     }
    
//     // 初始化电机1
//     if (motor_init_device(SERVO_ID_1, "左电机") == 0) {
//         // printf("[MAIN] 电机1初始化成功\n");
//     } else {
//         // printf("[MAIN] 电机1初始化失败\n");
//     }
    
//     // 初始化电机2
//     if (motor_init_device(SERVO_ID_2, "右电机") == 0) {
//         // printf("[MAIN] 电机2初始化成功\n");
//     } else {
//         // printf("[MAIN] 电机2初始化失败\n");
//     }
    
//     // 设置保护参数
//     motor_set_limits(SERVO_ID_1, g_data->config.max_current_1, g_data->config.max_speed_1);
//     motor_set_limits(SERVO_ID_2, g_data->config.max_current_2, g_data->config.max_speed_2);
    
//     // 使能电机
//     motor_enable_device(SERVO_ID_1, true);
//     motor_enable_device(SERVO_ID_2, true);
    
//     g_data->motors_initialized = true;
//     // printf("[MAIN] 电机系统初始化完成\n");
// }

// // 初始化传感器系统
// void init_sensor_system(void) {
//     // printf("[MAIN] 初始化传感器系统...\n");
    
//     // BME68X传感器在env_sensor_thread中初始化
//     // printf("[MAIN] 传感器系统就绪\n");
// }

// // 辅助函数：发送测试命令
// static void send_test_command(GlobalData* g_data) {
//     // 直接发送控制命令到消息队列
//     MotorCmdMsg cmd_msg;
//     memset(&cmd_msg, 0, sizeof(MotorCmdMsg));
//     cmd_msg.msg_type = MSG_TYPE_MOTOR_CMD;
    
//     static int cmd_count = 0;
    
//     // 生成测试命令
//     if (cmd_count % 10 == 0) {
//         cmd_msg.target_angle_1 = 130.0f;//抚养
//         cmd_msg.target_speed_1 = 50.0f;
//         cmd_msg.enable_1 = true;
//         cmd_msg.stop_1 = false;
//         // cmd_msg.direction_1 = 1;  // 逆时针
        
//         cmd_msg.target_angle_2 = -10.0f;
//         cmd_msg.target_speed_2 = 50.0f;
//         cmd_msg.enable_2 = true;
//         cmd_msg.stop_2 = false;
//         // cmd_msg.direction_2 = 0;  // 顺时针
//     } else if (cmd_count % 10 == 1) {
//         cmd_msg.target_angle_1 = 110.0f;
//         cmd_msg.target_speed_1 = 50.0f;
//         cmd_msg.enable_1 = true;
//                 cmd_msg.stop_1 = false;

//         cmd_msg.target_angle_2 = 20.0f;
//         cmd_msg.target_speed_2 = 50.0f;
//         cmd_msg.enable_2 = true;
//                 cmd_msg.stop_2 = false;

//     }
//      else if (cmd_count % 10 == 3) {
//         cmd_msg.target_angle_1 = 120.0f;
//         cmd_msg.target_speed_1 = 80.0f;
//         cmd_msg.enable_1 = true;
//                         cmd_msg.stop_1 = false;

        
//         cmd_msg.target_angle_2 = 0.0f;
//         cmd_msg.target_speed_2 = 80.0f;
//         cmd_msg.enable_2 = true;
//                 cmd_msg.stop_2 = false;

//     }
    
//     cmd_count++;
    
//     // 发送到控制命令消息队列
//     if (msg_queue_send_cmd(g_data->msg_queues.cmd_mq, &cmd_msg) == 0) {
//         // printf("[MAIN] 测试控制命令已发送到消息队列\n");
        
//         // 同时更新全局命令（供TCP线程使用）
//         pthread_mutex_lock(&g_data->cmd_mutex);
//         g_data->last_motor_cmd = cmd_msg;
//         pthread_cond_signal(&g_data->cmd_cond);
//         pthread_mutex_unlock(&g_data->cmd_mutex);
//     } else {
//         // printf("[MAIN] 发送控制命令失败\n");
//     }
// }

// // 初始化显示（简化版）
// void init_display(void) {
//     // printf("[MAIN] 显示初始化...\n");
//     // 实际初始化代码
// }

// void cleanup_display(void) {
//     // printf("[MAIN] 显示清理...\n");
//     // 实际清理代码
// }

// // 更新显示（简化版）
// void update_display(const EnvMsg* env, const ImuMsg* imu, 
//                    const MotorMsg* motor, bool tcp_connected, 
//                    bool motors_ready) {
//     // 清屏（终端版本）
//     // printf("\033[2J\033[1;1H");  // 清屏并移动光标到左上角
    
//     time_t now = time(NULL);
//     struct tm* tm_info = localtime(&now);
//     char time_str[20];
//     strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    

    
//     if (env) {
//         // printf("环境传感器:\n");
//         // printf("  温度: %6.1f°C   湿度: %6.1f%%\n", 
//         //        env->temperature, env->humidity);
//         // printf("  气压: %6.1f hPa   气体: %8.0f Ω\n",
//         //        env->pressure, env->gas_resistance);
//         // printf("  时间戳: %llu μs\n", env->timestamp);
//     } else {
//         // printf("环境传感器: 等待数据...\n");
//     }
    
//     // printf("----------------------------------------\n");
    
//     if (motor) {
//         // printf("电机状态:\n");
//         // printf("  电机1: %6.0f RPM  %5.0f mA  %6.1f°  %5.1f V  %s\n",
//         //        motor->speed_rpm_1, motor->current_ma_1, 
//         //        motor->position_deg_1, motor->voltage_1,
//         //        motor->alarm_code_1 == 0 ? "正常" : "告警");
        
//         // printf("  电机2: %6.0f RPM  %5.0f mA  %6.1f°  %5.1f V  %s\n",
//         //        motor->speed_rpm_2, motor->current_ma_2, 
//         //        motor->position_deg_2, motor->voltage_2,
//         //        motor->alarm_code_2 == 0 ? "正常" : "告警");
        
//         if (motor->alarm_code_1 != 0) {
//             // printf("  电机1告警码: 0x%08X\n", motor->alarm_code_1);
//         }
//         if (motor->alarm_code_2 != 0) {
//             // printf("  电机2告警码: 0x%08X\n", motor->alarm_code_2);
//         }
//         // printf("  时间戳: %llu μs\n", motor->timestamp);
//     } else {
//         // printf("电机状态: 等待数据...\n");
//     }
    
//     // printf("----------------------------------------\n");
    
//     // if (imu) {
//     //     // printf("IMU数据:\n");
//     //     // printf("  加速度: X=%6.3f, Y=%6.3f, Z=%6.3f m/s²\n",
//     //     //        imu->accel_x, imu->accel_y, imu->accel_z);
//     //     // printf("  角速度: X=%6.1f, Y=%6.1f, Z=%6.1f rad/s\n",
//     //     //        imu->gyro_x, imu->gyro_y, imu->gyro_z);
//     //     // printf("  时间戳: %llu μs\n", imu->timestamp);
//     // } else {
//     //     // printf("IMU数据: 等待数据...\n");
//     // }
    

    
//     // 显示消息队列状态
//     int env_count = msg_queue_get_count(g_data.msg_queues.env_mq);    // env_mq 不是 env_qid
//     int motor_count = msg_queue_get_count(g_data.msg_queues.motor_mq); // motor_mq 不是 motor_qid
//     //int imu_count = msg_queue_get_count(g_data.msg_queues.imu_mq);     // imu_mq 不是 imu_qid
//     int cmd_count = msg_queue_get_count(g_data.msg_queues.cmd_mq);     // cmd_mq 不是 cmd_qid
    
// }

// int main(int argc, char* argv[]) {
//     (void)argc;
//     (void)argv;

    
//     // 初始化全局数据
//     memset(&g_data, 0, sizeof(GlobalData));
    
//     // 初始化默认配置
//     init_default_config(&g_data.config);
    
//     // 初始化消息队列系统
//     init_msg_queue_system(&g_data);
    
//     // 初始化命令互斥锁和条件变量
//     pthread_mutex_init(&g_data.cmd_mutex, NULL);
//     pthread_cond_init(&g_data.cmd_cond, NULL);
    
//     g_data.system_running = true;
//     g_data.tcp_connected = false;
//     g_data.motors_initialized = false;
    
//     // 设置信号处理
//     signal(SIGINT, signal_handler);
//     signal(SIGTERM, signal_handler);
//     struct sigaction sa;
//     sa.sa_handler = signal_handler;
//     sigemptyset(&sa.sa_mask);
//     sa.sa_flags = 0;
//     sigaction(SIGINT,&sa,NULL);
//     // 初始化显示
//     init_display();
    
//     // 初始化电机系统
//     init_motor_system(&g_data);
    
//     // 初始化传感器系统
//     init_sensor_system();
    
//     // 创建线程数组
//     pthread_t threads[6];
//     int thread_count = 0;
//     int ret;
    
//     // printf("\n正在启动线程...\n");
    
//     // 1. 启动数据采集线程
//     ret = pthread_create(&threads[thread_count], NULL, env_sensor_thread, &g_data);
//     if (ret != 0) {
//         fprintf(stderr, "创建环境传感器线程失败\n");
//         return 1;
//     }
//     thread_count++;
//     // printf("  ✓ 环境传感器线程启动\n");
    
//     // ret = pthread_create(&threads[thread_count], NULL, imu_collector_thread, &g_data);
//     // if (ret != 0) {
//     //     fprintf(stderr, "创建IMU采集线程失败\n");
//     //     return 1;
//     // }
//     // thread_count++;
//     // printf("  ✓ IMU采集线程启动\n");
    
//     ret = pthread_create(&threads[thread_count], NULL, motor_status_thread, &g_data);
//     if (ret != 0) {
//         fprintf(stderr, "创建电机状态线程失败\n");
//         return 1;
//     }
//     thread_count++;
//     // printf("  ✓ 电机状态线程启动\n");
    
//     // 2. 启动屏幕显示线程
//     ret = pthread_create(&threads[thread_count], NULL, display_thread, &g_data);
//     if (ret != 0) {
//         fprintf(stderr, "创建显示线程失败\n");
//         return 1;
//     }
//     thread_count++;
//     // printf("  ✓ 显示线程启动\n");
    
//     // 3. 启动TCP客户端线程（连接主板）
//     ret = pthread_create(&threads[thread_count], NULL, tcp_report_thread, &g_data);
//     if (ret != 0) {
//         fprintf(stderr, "创建TCP上报线程失败\n");
//         return 1;
//     }
//     thread_count++;
//     // printf("  ✓ TCP上报线程启动\n");
    
//     // 4. 启动电机控制线程
//     ret = pthread_create(&threads[thread_count], NULL, motor_control_thread, &g_data);
//     if (ret != 0) {
//         fprintf(stderr, "创建电机控制线程失败\n");
//         return 1;
//     }
//     thread_count++;
//     // printf("  ✓ 电机控制线程启动\n");
    
//     // printf("\n========================================\n");
//     // printf("所有线程启动完成，系统运行中...\n");
//     // printf("使用消息队列进行线程间通信\n");
//     // printf("========================================\n");
    
//     // 主循环 - 发送示例控制命令
//     int loop_count = 0;
//     while (keep_running) {
//         // 主线程可以做一些监控工作
//         static int heartbeat = 0;
//         if (++heartbeat % 10 == 0) {  // 每10秒显示一次状态
//             // printf("\n[主线程] 系统运行 %d 秒，TCP连接: %s，电机: %s\n",
//             //        heartbeat,
//             //        g_data.tcp_connected ? "已建立" : "连接中...",
//             //        g_data.motors_initialized ? "就绪" : "未就绪");
            
//             // 显示消息队列状态
//             int env_count = msg_queue_get_count(g_data.msg_queues.env_mq);
//             int motor_count = msg_queue_get_count(g_data.msg_queues.motor_mq);
//             printf("  消息队列: 环境=%d条, 电机=%d条\n", env_count, motor_count);
//         }
        
//         // 每30秒发送一个示例命令
//         // if ( g_data.motors_initialized) {
//         //     send_test_command(&g_data);
//         // }
        
//         struct timeval tv;
//         tv.tv_sec = 0;
//         tv.tv_usec = 200000;
//         select(0,NULL,NULL,NULL,&tv);
//         if(ret==-1&&errno==EINTR)
//         {
//             break;
//         }
//     }
    
//     // 停止系统
//      printf("\n正在停止系统...\n");
//     g_data.system_running = false;
//     fflush(stdout);
//     // 等待所有线程结束
//     //  printf("等待线程退出...\n");
//     // for (int i = 0; i < thread_count; i++) {
//     //     pthread_join(threads[i], NULL);
//     //     printf("  ✓ 线程 %d 已退出\n", i);
//     // }
    
//     // 清理电机系统
//     motor_system_cleanup();
    
//     // 清理消息队列
//     msg_queue_cleanup(&g_data.msg_queues);

    
//     // 清理互斥锁和条件变量
//     pthread_mutex_destroy(&g_data.cmd_mutex);
//     pthread_cond_destroy(&g_data.cmd_cond);
    
//     cleanup_display();
    
//     printf("========================================\n");
//     printf("系统正常关闭\n");
//     printf("========================================\n");
//     exit(0);
//     return 0;
// }





// main.c - 修复函数声明
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include "shared_data.h"
#include "motor_api.h"
#include "bme68x.h"
#include "bme68x_defs.h"
#include "ipc_msg_queue.h"
#include "motor.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <stdint.h>
#include <stdarg.h>
#include <math.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include "serial_port.h"
// 声明包装函数
int motor_batch_read(uint32_t id, uint8_t* addrs, uint32_t* results, int count);
int motor_position_mode(uint32_t id, float target_degree, float speed_rpm, 
                       float accel_rpm_s, float decel_rpm_s);
int motor_speed_mode(uint32_t id, float speed_rpm, float accel_rpm_s, float decel_rpm_s);
int motor_current_mode(uint32_t id, int32_t current_ma);
int motor_enable(uint32_t id, int enable);
int motor_stop(uint32_t id);
int motor_dual_move(float angle1, float angle2, float speed_rpm);
// 声明所有线程函数
void* env_sensor_thread(void* arg);
void* imu_collector_thread(void* arg);
void* motor_status_thread(void* arg);
void* display_thread(void* arg);
void* tcp_report_thread(void* arg);
void* motor_control_thread(void* arg);

// 全局变量
GlobalData g_data;
volatile sig_atomic_t keep_running = 1;

// 信号处理函数
void signal_handler(int sig) {
    // printf("\n收到信号 %d，正在关闭...\n", sig);
    keep_running = 0;
    g_data.system_running = false;
}


// 连接服务器 - 声明（在 tcp_client.c 中实现）
bool connect_to_server(int* sockfd);

// 初始化默认配置
void init_default_config(SystemConfig* config) {
    memset(config, 0, sizeof(SystemConfig));
    
    // 网络配置
    strcpy(config->server_ip, "127.0.0.1");
    config->server_port = 8080;
    
    // 电机保护参数
    config->max_current_1 = 1400;  // 电机1最大电流 1400mA
    config->max_current_2 = 1400;   // 电机2最大电流 500mA
    config->max_speed_1 = 200;     // 电机1最大速度 200RPM
    config->max_speed_2 = 200;     // 电机2最大速度 150RPM
    
    // 默认运动参数
    config->default_accel = 1000.0f;  // 1000 RPM/s²
    config->default_decel = 1000.0f;  // 1000 RPM/s²
    
    // 系统配置
    config->debug_mode = 0;
    config->can_baudrate = 1000000;  // 1Mbps
}

// 初始化消息队列系统
void init_msg_queue_system(GlobalData* g_data) {
    // printf("[MAIN] 初始化消息队列系统...\n");
    
    // 使用POSIX消息队列初始化
    if (msg_queue_init(&g_data->msg_queues) != 0) {
        printf("[MAIN] 消息队列初始化失败\n");
        exit(1);
    }
    
    printf("[MAIN] 消息队列系统初始化完成\n");
    // printf("  环境队列: %d\n", g_data->msg_queues.env_mq);
    // printf("  电机队列: %d\n", g_data->msg_queues.motor_mq);
    // printf("  IMU队列: %d\n", g_data->msg_queues.imu_mq);
    // printf("  命令队列: %d\n", g_data->msg_queues.cmd_mq);
}

// 初始化电机系统
void init_motor_system(GlobalData* g_data) {
    // printf("[MAIN] 初始化电机系统...\n");
    
    // 初始化电机API
    if (motor_system_init() != 0) {
        // printf("[MAIN] 电机系统初始化失败\n");
        return;
    }
    
    // 初始化电机1
    if (motor_init_device(SERVO_ID_1, "左电机") == 0) {
        // printf("[MAIN] 电机1初始化成功\n");
    } else {
        // printf("[MAIN] 电机1初始化失败\n");
    }
    
    // 初始化电机2
    if (motor_init_device(SERVO_ID_2, "右电机") == 0) {
        // printf("[MAIN] 电机2初始化成功\n");
    } else {
        // printf("[MAIN] 电机2初始化失败\n");
    }
    
    // 设置保护参数
    motor_set_limits(SERVO_ID_1, g_data->config.max_current_1, g_data->config.max_speed_1);
    motor_set_limits(SERVO_ID_2, g_data->config.max_current_2, g_data->config.max_speed_2);
    
    // 使能电机
    motor_enable_device(SERVO_ID_1, true);
    motor_enable_device(SERVO_ID_2, true);
    
    g_data->motors_initialized = true;
    // printf("[MAIN] 电机系统初始化完成\n");
}

// 初始化传感器系统
void init_sensor_system(void) {
    // printf("[MAIN] 初始化传感器系统...\n");
    
    // BME68X传感器在env_sensor_thread中初始化
    // printf("[MAIN] 传感器系统就绪\n");
}

// 辅助函数：发送测试命令
static void send_test_command(GlobalData* g_data) {
    // 直接发送控制命令到消息队列
    MotorCmdMsg cmd_msg;
    memset(&cmd_msg, 0, sizeof(MotorCmdMsg));
    cmd_msg.msg_type = MSG_TYPE_MOTOR_CMD;
    
    static int cmd_count = 0;
    
    // 生成测试命令
    if (cmd_count % 10 == 0) {
        cmd_msg.target_angle_1 = 130.0f;//抚养
        cmd_msg.target_speed_1 = 50.0f;
        cmd_msg.enable_1 = true;
        cmd_msg.stop_1 = false;
        // cmd_msg.direction_1 = 1;  // 逆时针
        
        cmd_msg.target_angle_2 = -10.0f;
        cmd_msg.target_speed_2 = 50.0f;
        cmd_msg.enable_2 = true;
        cmd_msg.stop_2 = false;
        // cmd_msg.direction_2 = 0;  // 顺时针
    } else if (cmd_count % 10 == 1) {
        cmd_msg.target_angle_1 = 110.0f;
        cmd_msg.target_speed_1 = 50.0f;
        cmd_msg.enable_1 = true;
                cmd_msg.stop_1 = false;

        cmd_msg.target_angle_2 = 20.0f;
        cmd_msg.target_speed_2 = 50.0f;
        cmd_msg.enable_2 = true;
                cmd_msg.stop_2 = false;

    }
     else if (cmd_count % 10 == 3) {
        cmd_msg.target_angle_1 = 120.0f;
        cmd_msg.target_speed_1 = 80.0f;
        cmd_msg.enable_1 = true;
                        cmd_msg.stop_1 = false;

        
        cmd_msg.target_angle_2 = 0.0f;
        cmd_msg.target_speed_2 = 80.0f;
        cmd_msg.enable_2 = true;
                cmd_msg.stop_2 = false;

    }
    
    cmd_count++;
    
    // 发送到控制命令消息队列
    if (msg_queue_send_cmd(g_data->msg_queues.cmd_mq, &cmd_msg) == 0) {
        // printf("[MAIN] 测试控制命令已发送到消息队列\n");
        
        // 同时更新全局命令（供TCP线程使用）
        pthread_mutex_lock(&g_data->cmd_mutex);
        g_data->last_motor_cmd = cmd_msg;
        pthread_cond_signal(&g_data->cmd_cond);
        pthread_mutex_unlock(&g_data->cmd_mutex);
    } else {
        // printf("[MAIN] 发送控制命令失败\n");
    }
}

// 初始化显示（简化版）
void init_display(void) {
    // printf("[MAIN] 显示初始化...\n");
    // 实际初始化代码
}

void cleanup_display(void) {
    // printf("[MAIN] 显示清理...\n");
    // 实际清理代码
}

// 更新显示（简化版）
void update_display(const EnvMsg* env, const ImuMsg* imu, 
                   const MotorMsg* motor, bool tcp_connected, 
                   bool motors_ready) {
    // 清屏（终端版本）
    // printf("\033[2J\033[1;1H");  // 清屏并移动光标到左上角
    
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    

    
    if (env) {
        // printf("环境传感器:\n");
        // printf("  温度: %6.1f°C   湿度: %6.1f%%\n", 
        //        env->temperature, env->humidity);
        // printf("  气压: %6.1f hPa   气体: %8.0f Ω\n",
        //        env->pressure, env->gas_resistance);
        // printf("  时间戳: %llu μs\n", env->timestamp);
    } else {
        // printf("环境传感器: 等待数据...\n");
    }
    
    // printf("----------------------------------------\n");
    
    if (motor) {
        // printf("电机状态:\n");
        // printf("  电机1: %6.0f RPM  %5.0f mA  %6.1f°  %5.1f V  %s\n",
        //        motor->speed_rpm_1, motor->current_ma_1, 
        //        motor->position_deg_1, motor->voltage_1,
        //        motor->alarm_code_1 == 0 ? "正常" : "告警");
        
        // printf("  电机2: %6.0f RPM  %5.0f mA  %6.1f°  %5.1f V  %s\n",
        //        motor->speed_rpm_2, motor->current_ma_2, 
        //        motor->position_deg_2, motor->voltage_2,
        //        motor->alarm_code_2 == 0 ? "正常" : "告警");
        
        if (motor->alarm_code_1 != 0) {
            // printf("  电机1告警码: 0x%08X\n", motor->alarm_code_1);
        }
        if (motor->alarm_code_2 != 0) {
            // printf("  电机2告警码: 0x%08X\n", motor->alarm_code_2);
        }
        // printf("  时间戳: %llu μs\n", motor->timestamp);
    } else {
        // printf("电机状态: 等待数据...\n");
    }
    
    // printf("----------------------------------------\n");
    
    // if (imu) {
    //     // printf("IMU数据:\n");
    //     // printf("  加速度: X=%6.3f, Y=%6.3f, Z=%6.3f m/s²\n",
    //     //        imu->accel_x, imu->accel_y, imu->accel_z);
    //     // printf("  角速度: X=%6.1f, Y=%6.1f, Z=%6.1f rad/s\n",
    //     //        imu->gyro_x, imu->gyro_y, imu->gyro_z);
    //     // printf("  时间戳: %llu μs\n", imu->timestamp);
    // } else {
    //     // printf("IMU数据: 等待数据...\n");
    // }
    

    
    // 显示消息队列状态
    int env_count = msg_queue_get_count(g_data.msg_queues.env_mq);    // env_mq 不是 env_qid
    int motor_count = msg_queue_get_count(g_data.msg_queues.motor_mq); // motor_mq 不是 motor_qid
    //int imu_count = msg_queue_get_count(g_data.msg_queues.imu_mq);     // imu_mq 不是 imu_qid
    int cmd_count = msg_queue_get_count(g_data.msg_queues.cmd_mq);     // cmd_mq 不是 cmd_qid
    
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    
    // 初始化全局数据
    memset(&g_data, 0, sizeof(GlobalData));
    
    // 初始化默认配置
    init_default_config(&g_data.config);
    
    // 初始化消息队列系统
    init_msg_queue_system(&g_data);
    
    // 初始化命令互斥锁和条件变量
    pthread_mutex_init(&g_data.cmd_mutex, NULL);
    pthread_cond_init(&g_data.cmd_cond, NULL);
    
    g_data.system_running = true;
    g_data.tcp_connected = false;
    g_data.motors_initialized = false;
    
    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // 初始化显示
    init_display();
    
    // 初始化电机系统
    init_motor_system(&g_data);
    
    // 初始化传感器系统
    init_sensor_system();
    
    // 创建线程数组
    pthread_t threads[6];
    int thread_count = 0;
    int ret;
    
    // printf("\n正在启动线程...\n");
    
    // 1. 启动数据采集线程
    ret = pthread_create(&threads[thread_count], NULL, env_sensor_thread, &g_data);
    if (ret != 0) {
        fprintf(stderr, "创建环境传感器线程失败\n");
        return 1;
    }
    thread_count++;
    // printf("  ✓ 环境传感器线程启动\n");
    
    // ret = pthread_create(&threads[thread_count], NULL, imu_collector_thread, &g_data);
    // if (ret != 0) {
    //     fprintf(stderr, "创建IMU采集线程失败\n");
    //     return 1;
    // }
    // thread_count++;
    // printf("  ✓ IMU采集线程启动\n");
    
    ret = pthread_create(&threads[thread_count], NULL, motor_status_thread, &g_data);
    if (ret != 0) {
        fprintf(stderr, "创建电机状态线程失败\n");
        return 1;
    }
    thread_count++;
    // printf("  ✓ 电机状态线程启动\n");
    
    // 2. 启动屏幕显示线程
    ret = pthread_create(&threads[thread_count], NULL, display_thread, &g_data);
    if (ret != 0) {
        fprintf(stderr, "创建显示线程失败\n");
        return 1;
    }
    thread_count++;
    // printf("  ✓ 显示线程启动\n");
    
    // 3. 启动TCP客户端线程（连接主板）
    ret = pthread_create(&threads[thread_count], NULL, tcp_report_thread, &g_data);
    if (ret != 0) {
        fprintf(stderr, "创建TCP上报线程失败\n");
        return 1;
    }
    thread_count++;
    // printf("  ✓ TCP上报线程启动\n");
    
    // 4. 启动电机控制线程
    ret = pthread_create(&threads[thread_count], NULL, motor_control_thread, &g_data);
    if (ret != 0) {
        fprintf(stderr, "创建电机控制线程失败\n");
        return 1;
    }
    thread_count++;
    // printf("  ✓ 电机控制线程启动\n");
    
    // printf("\n========================================\n");
    // printf("所有线程启动完成，系统运行中...\n");
    // printf("使用消息队列进行线程间通信\n");
    // printf("========================================\n");
    
    // 主循环 - 发送示例控制命令
    int loop_count = 0;
    while (keep_running) {
        // 主线程可以做一些监控工作
        static int heartbeat = 0;
        if (++heartbeat % 10 == 0) {  // 每10秒显示一次状态
            // printf("\n[主线程] 系统运行 %d 秒，TCP连接: %s，电机: %s\n",
            //        heartbeat,
            //        g_data.tcp_connected ? "已建立" : "连接中...",
            //        g_data.motors_initialized ? "就绪" : "未就绪");
            
            // 显示消息队列状态
            int env_count = msg_queue_get_count(g_data.msg_queues.env_mq);
            int motor_count = msg_queue_get_count(g_data.msg_queues.motor_mq);
            printf("  消息队列: 环境=%d条, 电机=%d条\n", env_count, motor_count);
        }
        
        // 每30秒发送一个示例命令
        // if ( g_data.motors_initialized) {
        //     send_test_command(&g_data);
        // }
        
        loop_count++;
        sleep(2);
    }
    
    // 停止系统
    // printf("\n正在停止系统...\n");
    g_data.system_running = false;
    
    // 等待所有线程结束
    // printf("等待线程退出...\n");
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
        // printf("  ✓ 线程 %d 已退出\n", i);
    }
    
    // 清理电机系统
    motor_system_cleanup();
    
    // 清理消息队列
    msg_queue_cleanup(&g_data.msg_queues);

    
    // 清理互斥锁和条件变量
    pthread_mutex_destroy(&g_data.cmd_mutex);
    pthread_cond_destroy(&g_data.cmd_cond);
    
    cleanup_display();
    
    // printf("========================================\n");
    // printf("系统正常关闭\n");
    // printf("========================================\n");
    
    return 0;
}