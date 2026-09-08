// // thread_modules/display_thread.c
// #include "shared_data.h"
// #include "ipc/ipc_msg_queue.h"
// #include <stdio.h>
// #include <unistd.h>
// #include <time.h>
// #include <string.h>
// #include <fcntl.h>
// #include <termios.h>
// #include <errno.h>
// #include <poll.h>

// #define UART_DEVICE "/dev/ttyS0"
// static int uart_fd = -1;

// // 初始化串口
// int init_uart(const char* device, int baud) {
//     struct termios options;
//     uart_fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
//     if (uart_fd == -1) {
//         perror("[DISPLAY] 串口打开失败");
//         return -1;
//     }

//     if (tcgetattr(uart_fd, &options) != 0) {
//         perror("[DISPLAY] tcgetattr失败");
//         close(uart_fd);
//         return -1;
//     }
    
//     // 设置波特率（注意：Linux中需用cfsetispeed/cfsetospeed设置）
//     speed_t speed = B115200;
//     cfsetispeed(&options, speed);
//     cfsetospeed(&options, speed);
    
//     // 8N1 配置
//     options.c_cflag |= (CLOCAL | CREAD);
//     options.c_cflag &= ~PARENB;
//     options.c_cflag &= ~CSTOPB;
//     options.c_cflag &= ~CSIZE;
//     options.c_cflag |= CS8;
    
//     // 原始模式
//     options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
//     options.c_iflag &= ~(IXON | IXOFF | IXANY | IGNBRK);
//     options.c_oflag &= ~OPOST;
    
//     // 超时设置（可选）
//     options.c_cc[VMIN] = 0;
//     options.c_cc[VTIME] = 10;
    
//     if (tcsetattr(uart_fd, TCSANOW, &options) != 0) {
//         perror("[DISPLAY] tcsetattr失败");
//         close(uart_fd);
//         return -1;
//     }
    
//     // 清空缓冲区
//     tcflush(uart_fd, TCIOFLUSH);
//     return 0;
// }

// // // 安全发送至ESP（带错误处理）
// // void send_to_esp(const char* msg) {
// //     if (uart_fd < 0 || !msg || msg[0] == '\0') return;
    
// //     ssize_t len = strlen(msg);
// //     if (write(uart_fd, msg, len) != len) {
// //         // 静默失败，避免刷屏（生产环境可加计数器限流）
// //         return;
// //     }
// //     if (write(uart_fd, "\n", 1) != 1) {
// //         return;
// //     }
// //     // 确保数据发出（小数据量可选）
// //     tcdrain(uart_fd);
// // }
// // 替换原 send_to_esp 函数
// void send_to_esp(const char* msg) {
//     if (uart_fd < 0 || !msg || msg[0] == '\0') return;
    
//     ssize_t len = strlen(msg);
//     // 非阻塞写入：失败则丢弃，绝不阻塞线程
//     write(uart_fd, msg, len);
//     write(uart_fd, "\n", 1);
//     // ❌ 移除 tcdrain(uart_fd); —— 这是卡死根源！
// }

// // 显示线程 - 修复队列积压核心逻辑
// void* display_thread(void* arg) {
//     GlobalData* g_data = (GlobalData*)arg;
    
//     printf("[DISPLAY] 显示线程启动 (修复版: 循环消费队列)\n");
    
//     if (g_data == NULL) {
//         fprintf(stderr, "[DISPLAY] 错误：未传递有效的全局数据\n");
//         return NULL;
//     }
    
//     // 初始化串口
//     if (init_uart(UART_DEVICE, 115200) != 0) {
//         fprintf(stderr, "[DISPLAY] 错误：无法初始化串口 %s\n", UART_DEVICE);
//         return NULL;
//     }
//     printf("[DISPLAY] 串口 %s 初始化成功，波特率 115200\n", UART_DEVICE);

//     while (g_data->system_running) {
//         // =============== 环境传感器：循环读取保留最新 ===============
//         EnvMsg latest_env = {0};
//         int env_received = 0;
//         while (1) {
//             EnvMsg temp;
//             int ret = msg_queue_receive_env(g_data->msg_queues.env_mq, &temp, false);
//             if (ret > 0) {
//                 latest_env = temp;
//                 env_received = 1;
//             } else if (ret == 0 || (ret < 0 && errno == EAGAIN)) {
//                 break; // 队列空，正常退出
//             } else {
//                 // 其他错误（如队列损坏），跳出避免死循环
//                 fprintf(stderr, "[DISPLAY] 环境队列读取异常 (errno=%d)\n", errno);
//                 break;
//             }
//         }
        
//         if (env_received) {
//             char buf[512];
//             snprintf(buf, sizeof(buf),
//                 "[INFO][ENV] T=%.1f H=%.1f P=%.1f G=%.0f",
//                 latest_env.temperature, latest_env.humidity,
//                 latest_env.pressure, latest_env.gas_resistance);
//             send_to_esp(buf);
            
//             printf("环境传感器 (实时 | 最新):\n");
//             printf("  温度: %6.1f°C   湿度: %6.1f%%\n", 
//                    latest_env.temperature, latest_env.humidity);
//             printf("  气压: %6.1f hPa   气体: %8.0f Ω\n",
//                    latest_env.pressure, latest_env.gas_resistance);
//         } else {
//             printf("环境传感器: 等待数据...\n");
//         }
//         printf("----------------------------------------\n");

//         // =============== 电机数据：循环读取保留最新 ===============
//         MotorMsg latest_motor = {0};
//         int motor_received = 0;
//         while (1) {
//             MotorMsg temp;
//             int ret = msg_queue_receive_motor(g_data->msg_queues.motor_mq, &temp, false);
//             if (ret > 0) {
//                 latest_motor = temp;
//                 motor_received = 1;
//             } else if (ret == 0 || (ret < 0 && errno == EAGAIN)) {
//                 break;
//             } else {
//                 fprintf(stderr, "[DISPLAY] 电机队列读取异常 (errno=%d)\n", errno);
//                 break;
//             }
//         }
        
//         if (motor_received) {
//             // char buf[300];
//             // snprintf(buf, sizeof(buf),
//             //     "[INFO][MOTOR] S1=%.0f I1=%.0f P1=%.1f V1=%.1f S2=%.0f I2=%.0f P2=%.1f V2=%.1f A1=0x%08X A2=0x%08X",
//             //     latest_motor.speed_rpm_1, latest_motor.current_ma_1,
//             //     latest_motor.position_deg_1, latest_motor.voltage_1,
//             //     latest_motor.speed_rpm_2, latest_motor.current_ma_2,
//             //     latest_motor.position_deg_2, latest_motor.voltage_2,
//             //     latest_motor.alarm_code_1, latest_motor.alarm_code_2);
//             // send_to_esp(buf);
            
//             printf("电机状态 (实时 | 最新):\n");
//             printf("  电机1: %6.0f RPM  %5.0f mA  %6.1f°  %5.1f V\n",
//                    latest_motor.speed_rpm_1, latest_motor.current_ma_1, 
//                    latest_motor.position_deg_1, latest_motor.voltage_1);
//             printf("  电机2: %6.0f RPM  %5.0f mA  %6.1f°  %5.1f V\n",
//                    latest_motor.speed_rpm_2, latest_motor.current_ma_2, 
//                    latest_motor.position_deg_2, latest_motor.voltage_2);
            
//             if (latest_motor.alarm_code_1 != 0 || latest_motor.alarm_code_2 != 0) {
//                 printf("  告警: 电机1=0x%08X, 电机2=0x%08X\n",
//                        latest_motor.alarm_code_1, latest_motor.alarm_code_2);
//             }
//         } else {
//             printf("电机状态: 等待数据...\n");
//         }
//         printf("----------------------------------------\n");

//         // =============== IMU数据：循环读取保留最新（关键修复） ===============
//         ImuMsg latest_imu = {0};
//         int imu_received = 0;
//         while (1) {
//             ImuMsg temp;
//             int ret = msg_queue_receive_imu(g_data->msg_queues.imu_mq, &temp, false);
//             if (ret > 0) {
//                 latest_imu = temp; // 持续覆盖，最终保留最新
//                 imu_received = 1;
//             } else if (ret == 0 || (ret < 0 && errno == EAGAIN)) {
//                 break; // 队列空，正常退出
//             } else {
//                 fprintf(stderr, "[DISPLAY] IMU队列读取异常 (errno=%d)\n", errno);
//                 break;
//             }
//         }
        
//         if (imu_received) {
//             // char buf[256];
//             // snprintf(buf, sizeof(buf),
//             //     "[INFO][IMU] AX=%.3f AY=%.3f AZ=%.3f GX=%.3f GY=%.3f GZ=%.3f",
//             //     latest_imu.accel_x, latest_imu.accel_y, latest_imu.accel_z,
//             //     latest_imu.gyro_x, latest_imu.gyro_y, latest_imu.gyro_z);
//             // send_to_esp(buf);
            
//             printf("IMU数据 (实时 | 最新):\n");
//             printf("  加速度: X=%6.3f, Y=%6.3f, Z=%6.3f m/s²\n",
//                    latest_imu.accel_x, latest_imu.accel_y, latest_imu.accel_z);
//             printf("  角速度: X=%6.3f, Y=%6.3f, Z=%6.3f rad/s\n",
//                    latest_imu.gyro_x, latest_imu.gyro_y, latest_imu.gyro_z);
//         } else {
//             printf("IMU数据: 等待数据...\n");
//         }
//         printf("----------------------------------------\n");

//         // =============== 显示队列状态（清空后应接近0） ===============
//         int env_count = msg_queue_get_count(g_data->msg_queues.env_mq);
//         int motor_count = msg_queue_get_count(g_data->msg_queues.motor_mq);
//         int imu_count = msg_queue_get_count(g_data->msg_queues.imu_mq);
//         int cmd_count = msg_queue_get_count(g_data->msg_queues.cmd_mq);
        
//         printf("消息队列状态 (消费后):\n");
//         printf("  环境: %3d | 电机: %3d | IMU: %3d | 命令: %3d 条\n", 
//                env_count, motor_count, imu_count, cmd_count);
//         printf("========================================\n");
//         printf("系统运行中... (刷新频率: 2Hz)\n\n");

//         // =============== 用户自定义日志输入 ===============
//         struct pollfd pfd = { .fd = STDIN_FILENO, .events = POLLIN };
//         if (poll(&pfd, 1, 0) > 0 && (pfd.revents & POLLIN)) {
//             char user_input[256] = {0};
//             if (fgets(user_input, sizeof(user_input), stdin)) {
//                 // 移除换行符
//                 size_t len = strlen(user_input);
//                 if (len > 0 && user_input[len-1] == '\n') {
//                     user_input[len-1] = '\0';
//                 }
                
//                 // 跳过空输入
//                 if (strlen(user_input) == 0) {
//                     printf("提示：输入日志内容（或 'quit' 退出输入模式）\n");
//                     continue;
//                 }
                
//                 // 特殊命令：退出输入模式（可选）
//                 if (strcmp(user_input, "quit") == 0) {
//                     printf("[USER] 退出自定义输入模式\n");
//                     continue;
//                 }
                
//                 // 构造 [INFO][USER] 日志
//                 char user_log[300];
//                 snprintf(user_log, sizeof(user_log), "[INFO][USER] %s", user_input);
//                 send_to_esp(user_log);
                
//                 // 本地回显（绿色）
//                 printf("\033[0;32m[本地] 已发送自定义日志: %s\033[0m\n", user_input);
//             }
//         }

//         // 刷新间隔：500ms
//         usleep(500000);
//     }

//     // 清理资源
//     if (uart_fd >= 0) {
//         tcflush(uart_fd, TCIOFLUSH);
//         close(uart_fd);
//         uart_fd = -1;
//     }
    
//     printf("[DISPLAY] 显示线程安全退出\n");
//     return NULL;
// }


// thread_modules/display_thread.c - 支持从 log_mq 接收日志并发送给 ESP
#include "shared_data.h"
#include "ipc/ipc_msg_queue.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <poll.h>

#define UART_DEVICE "/dev/ttyS0"
static int uart_fd = -1;

// 初始化串口（保持不变）
int init_uart(const char* device, int baud) {
    struct termios options;
    uart_fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (uart_fd == -1) {
        perror("[DISPLAY] 串口打开失败");
        return -1;
    }
    if (tcgetattr(uart_fd, &options) != 0) {
        perror("[DISPLAY] tcgetattr失败");
        close(uart_fd);
        return -1;
    }

    speed_t speed = B115200;
    cfsetispeed(&options, speed);
    cfsetospeed(&options, speed);

    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_iflag &= ~(IXON | IXOFF | IXANY | IGNBRK);
    options.c_oflag &= ~OPOST;

    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 10;

    if (tcsetattr(uart_fd, TCSANOW, &options) != 0) {
        perror("[DISPLAY] tcsetattr失败");
        close(uart_fd);
        return -1;
    }
    tcflush(uart_fd, TCIOFLUSH);
    return 0;
}

// 发送到 ESP（非阻塞）
void send_to_esp(const char* msg) {
    if (uart_fd < 0 || !msg || msg[0] == '\0') return;
    write(uart_fd, msg, strlen(msg));
    write(uart_fd, "\n", 1);
}

// 显示线程
void* display_thread(void* arg) {
    GlobalData* g_data = (GlobalData*)arg;
    printf("[DISPLAY] 显示线程启动 (支持 TCP 日志)\n");
    if (!g_data) {
        fprintf(stderr, "[DISPLAY] 全局数据为空\n");
        return NULL;
    }

    if (init_uart(UART_DEVICE, 115200) != 0) {
        fprintf(stderr, "[DISPLAY] 串口初始化失败\n");
        return NULL;
    }

    while (g_data->system_running) {
        // ====== 消费日志消息（新增）======
        LogMsg latest_log = {0};
        int log_received = 0;
        while (1) {
            LogMsg temp;
            int ret = msg_queue_receive_log(g_data->msg_queues.log_mq, &temp, false);
            if (ret > 0) {
                latest_log = temp;
                log_received = 1;
            } else if (ret == 0 || (ret < 0 && (errno == EAGAIN || errno == ETIMEDOUT)))  {
                break;
            } else {
                fprintf(stderr, "[DISPLAY] 日志队列读取异常 (errno=%d)\n", errno);
                break;
            }
        }
        if (log_received) {
            char buf[300];
            snprintf(buf, sizeof(buf), "[INFO][REMOTE] %s", latest_log.message);
            send_to_esp(buf);
            printf("\033[0;36m[远程日志] %s\033[0m\n", latest_log.message);
        }

        // ====== 环境数据 ======
        EnvMsg latest_env = {0};
        int env_received = 0;
        while (1) {
            EnvMsg temp;
            int ret = msg_queue_receive_env(g_data->msg_queues.env_mq, &temp, false);
            if (ret > 0) {
                latest_env = temp;
                env_received = 1;
            } else if (ret == 0 || (ret < 0 && (errno == EAGAIN || errno == ETIMEDOUT)))  {
                break;
            } else {
                fprintf(stderr, "[DISPLAY] 环境队列读取异常\n");
                break;
            }
        }
        if (env_received) {
            char buf[512];
            snprintf(buf, sizeof(buf), "[INFO][ENV] T=%.1f H=%.1f P=%.1f G=%.0f",
                latest_env.temperature, latest_env.humidity,
                latest_env.pressure, latest_env.gas_resistance);
            send_to_esp(buf);
            printf("环境传感器:\n");
            printf(" 温度: %6.1f°C 湿度: %6.1f%%\n", latest_env.temperature, latest_env.humidity);
            printf(" 气压: %6.1f hPa 气体: %8.0f Ω\n", latest_env.pressure, latest_env.gas_resistance);
        }

        // ====== 电机数据 ======
        MotorMsg latest_motor = {0};
        int motor_received = 0;
        while (1) {
            MotorMsg temp;
            int ret = msg_queue_receive_motor(g_data->msg_queues.motor_mq, &temp, false);
            if (ret > 0) {
                latest_motor = temp;
                motor_received = 1;
            }else if (ret == 0 || (ret < 0 && (errno == EAGAIN || errno == ETIMEDOUT)))  {
                break;
            } else {
                fprintf(stderr, "[DISPLAY] 电机队列读取异常\n");
                break;
            }
        }
        if (motor_received) {
            printf("电机状态:\n");
            printf(" 电机1: %6.0f RPM %5.0f mA %6.1f° %5.1f V\n",
                latest_motor.speed_rpm_1, latest_motor.current_ma_1,
                latest_motor.position_deg_1, latest_motor.voltage_1);
            printf(" 电机2: %6.0f RPM %5.0f mA %6.1f° %5.1f V\n",
                latest_motor.speed_rpm_2, latest_motor.current_ma_2,
                latest_motor.position_deg_2, latest_motor.voltage_2);
            if (latest_motor.alarm_code_1 || latest_motor.alarm_code_2) {
                printf(" 告警: M1=0x%08X M2=0x%08X\n",
                    latest_motor.alarm_code_1, latest_motor.alarm_code_2);
            }
        }

        // ====== IMU 数据 ======
        ImuMsg latest_imu = {0};
        int imu_received = 0;
        while (1) {
            ImuMsg temp;
            int ret = msg_queue_receive_imu(g_data->msg_queues.imu_mq, &temp, false);
            if (ret > 0) {
                latest_imu = temp;
                imu_received = 1;
            }else if (ret == 0 || (ret < 0 && (errno == EAGAIN || errno == ETIMEDOUT)))  {
                break;
            } else {
                fprintf(stderr, "[DISPLAY] IMU队列读取异常\n");
                break;
            }
        }
        if (imu_received) {
            printf("IMU数据:\n");
            printf(" 加速度: X=%6.3f Y=%6.3f Z=%6.3f m/s²\n",
                latest_imu.accel_x, latest_imu.accel_y, latest_imu.accel_z);
            printf(" 角速度: X=%6.3f Y=%6.3f Z=%6.3f rad/s\n",
                latest_imu.gyro_x, latest_imu.gyro_y, latest_imu.gyro_z);
        }

        // ====== 队列状态 ======
        printf("----------------------------------------\n");
        printf("队列状态: ENV=%d MOTOR=%d IMU=%d CMD=%d LOG=%d\n",
            msg_queue_get_count(g_data->msg_queues.env_mq),
            msg_queue_get_count(g_data->msg_queues.motor_mq),
            msg_queue_get_count(g_data->msg_queues.imu_mq),
            msg_queue_get_count(g_data->msg_queues.cmd_mq),
            msg_queue_get_count(g_data->msg_queues.log_mq));
        printf("========================================\n\n");

        usleep(500000); // 2Hz
    }

    if (uart_fd >= 0) {
        tcflush(uart_fd, TCIOFLUSH);
        close(uart_fd);
        uart_fd = -1;
    }
    printf("[DISPLAY] 显示线程退出\n");
    return NULL;
}