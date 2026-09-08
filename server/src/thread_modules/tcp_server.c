// // #include "shared_data.h"
// // #include <sys/socket.h>
// // #include <netinet/in.h>
// // #include <arpa/inet.h>
// // #include <netdb.h>
// // #include <string.h>
// // #include <unistd.h>
// // #include <stdio.h>
// // #include <stdlib.h>
// // #include <pthread.h>
// // #include <sys/time.h>
// // #include <stdbool.h>
// // #include <fcntl.h>
// // #include <errno.h>
// // #include <time.h>
// // #include "ipc_msg_queue.h"

// // #define REMOTE_HOST "192.168.7.2"
// // #define REMOTE_PORT 8888
// // #define RECONNECT_INTERVAL 3

// // // ======================
// // // 【新增】状态记忆区 (State Retention)
// // // 使用 static 变量保存上一次的有效状态，实现“增量更新”
// // // ======================
// // static float last_target_angle_1 = 0.0;
// // static float last_target_speed_1 = 0.0;
// // static bool  last_enable_1 = false;
// // static bool  last_stop_1 = false;

// // static float last_target_angle_2 = 0.0;
// // static float last_target_speed_2 = 0.0;
// // static bool  last_enable_2 = false;
// // static bool  last_stop_2 = false;
// // // =================================

// // // TCP连接管理
// // typedef struct {
// //     int sockfd;
// //     struct sockaddr_in server_addr;
// //     bool connected;
// //     pthread_t recv_thread;
// // } TCPConnection;

// // void* tcp_recv_thread(void* arg);
// // void tcp_disconnect(TCPConnection* conn);
// // bool tcp_send_data(int sockfd, const char* data, size_t len);
// // bool connect_to_server(TCPConnection* conn);
// // bool send_tcp_data(TCPConnection* conn, const void* data, size_t len);
// // ssize_t recv_tcp_data(TCPConnection* conn, void* buffer, size_t buf_size);
// // void disconnect_server(TCPConnection* conn);

// // // 统一解析 TCP 命令（支持 MOTOR_CMD, LOG_MSG, SYS_CMD）
// // void parse_tcp_command(const char* cmd, GlobalData* g_data);

// // // 数据上报线程 - POSIX消息队列版本
// // void* tcp_report_thread(void* arg) {
// //     GlobalData* g_data = (GlobalData*)arg;
// //     TCPConnection conn = {0};
// //     // printf("[TCP] TCP上报线程启动\n");

// //     while (g_data->system_running) {
// //         // 连接远程主板
// //         if (!connect_to_server(&conn)) {
// //             g_data->tcp_connected = false;
// //             sleep(RECONNECT_INTERVAL);
// //             continue;
// //         }
// //         g_data->tcp_connected = true;
// //         // printf("[TCP] 已连接到主板: %s:%d\n", REMOTE_HOST, REMOTE_PORT);

// //         // 启动接收线程
// //         if (pthread_create(&conn.recv_thread, NULL, tcp_recv_thread, &conn) != 0) {
// //             // printf("[TCP] 创建接收线程失败\n");
// //             tcp_disconnect(&conn);
// //             continue;
// //         }

// //         // 数据上报循环
// //         while (g_data->tcp_connected && g_data->system_running) {
// //             EnvMsg env_msg;
// //             MotorMsg motor_msg;

// //             // 从POSIX消息队列读取环境数据（非阻塞）
// //             if (msg_queue_receive_env(g_data->msg_queues.env_mq, &env_msg, false) > 0) {
// //                 char report[256];
// //                 snprintf(report, sizeof(report),
// //                     "ENV:TEMP=%.1f,HUM=%.1f,PRESS=%.1f,GAS=%.0f,TS=%llu\n",
// //                     env_msg.temperature, env_msg.humidity, env_msg.pressure,
// //                     env_msg.gas_resistance, env_msg.timestamp);
// //                 if (!tcp_send_data(conn.sockfd, report, strlen(report))) {
// //                     // printf("[TCP] 发送环境数据失败\n");
// //                     break;
// //                 }
// //             }

// //             // 从POSIX消息队列读取电机数据（非阻塞）
// //             if (msg_queue_receive_motor(g_data->msg_queues.motor_mq, &motor_msg, false) > 0) {
// //                 char report[512];
// //                 snprintf(report, sizeof(report),
// //                     "MOTOR:M1_SPD=%.1f,M1_CUR=%.0f,M1_POS=%.1f,M1_V=%.1f,"
// //                     "M2_SPD=%.1f,M2_CUR=%.0f,M2_POS=%.1f,M2_V=%.1f,"
// //                     "M1_ALARM=0x%08X,M2_ALARM=0x%08X,TS=%llu\n",
// //                     motor_msg.speed_rpm_1, motor_msg.current_ma_1, motor_msg.position_deg_1, motor_msg.voltage_1,
// //                     motor_msg.speed_rpm_2, motor_msg.current_ma_2, motor_msg.position_deg_2, motor_msg.voltage_2,
// //                     motor_msg.alarm_code_1, motor_msg.alarm_code_2, motor_msg.timestamp);
// //                 if (!tcp_send_data(conn.sockfd, report, strlen(report))) {
// //                     // printf("[TCP] 发送电机数据失败\n");
// //                     break;
// //                 }
// //             }

// //             // 检查连接状态 - 发送心跳包
// //             if (!tcp_send_data(conn.sockfd, "\n", 1)) {
// //                 // printf("[TCP] 连接已断开\n");
// //                 g_data->tcp_connected = false;
// //                 break;
// //             }

// //             // 上报频率：10Hz
// //             usleep(100000);
// //         }

// //         // 连接断开，清理
// //         // printf("[TCP] 与主板连接断开\n");
// //         g_data->tcp_connected = false;
// //         tcp_disconnect(&conn);

// //         // 等待接收线程结束
// //         if (conn.recv_thread) {
// //             pthread_join(conn.recv_thread, NULL);
// //         }
// //         memset(&conn, 0, sizeof(conn));
// //     }
// //     tcp_disconnect(&conn);
// //     // printf("[TCP] TCP上报线程退出\n");
// //     return NULL;
// // }

// // // ======================
// // // 接收命令线程
// // // ======================
// // void* tcp_recv_thread(void* arg) {
// //     TCPConnection* conn = (TCPConnection*)arg;
// //     char buffer[1024];
// //     if (!conn) {
// //         // printf("[TCP] 接收线程参数无效\n");
// //         return NULL;
// //     }
// //     // printf("[TCP] 接收线程启动\n");

// //     while (conn->connected) {
// //         memset(buffer, 0, sizeof(buffer));
// //         int n = recv(conn->sockfd, buffer, sizeof(buffer) - 1, 0);
// //         if (n <= 0) {
// //             if (n == 0) {
// //                 printf("[TCP] 服务器主动断开连接\n");
// //             } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
// //                 continue;
// //             } else {
// //                 perror("[TCP] 接收数据失败");
// //             }
// //             conn->connected = false;
// //             break;
// //         }
// //         buffer[n] = '\0';

// //         // 移除换行符
// //         size_t len = strlen(buffer);
// //         while (len > 0 && (buffer[len-1] == '\n' || buffer[len-1] == '\r')) {
// //             buffer[--len] = '\0';
// //         }

// //         printf("[TCP] 收到命令: %s\n", buffer);

// //         // 访问全局数据
// //         extern GlobalData g_data;
// //         parse_tcp_command(buffer, &g_data);

// //         // 发送确认
// //         char ack[100];
// //         snprintf(ack, sizeof(ack), "ACK:CMD_RECEIVED\n");
// //         tcp_send_data(conn->sockfd, ack, strlen(ack));
// //     }
// //     // printf("[TCP] 接收线程退出\n");
// //     return NULL;
// // }

// // // ======================
// // // 统一解析 TCP 命令 (核心修改部分)
// // // 支持状态保持：未收到的参数沿用上一次的值
// // // ======================
// // void parse_tcp_command(const char* cmd, GlobalData* g_data) {
// //     if (!cmd || !g_data) {
// //         printf("[TCP] 命令或全局数据无效\n");
// //         return;
// //     }

// //     if (strncmp(cmd, "MOTOR_CMD:", 10) == 0) {
// //         MotorCmdMsg motor_cmd = {0};
// //         motor_cmd.msg_type = MSG_TYPE_MOTOR_CMD;

// //         // 【关键步骤 1】初始化为本电机的“历史状态”
// //         // 这样如果命令里没提某个参数，它就会自动保留旧值
// //         motor_cmd.target_angle_1 = last_target_angle_1;
// //         motor_cmd.target_speed_1 = last_target_speed_1;
// //         motor_cmd.enable_1       = last_enable_1;
// //         motor_cmd.stop_1         = last_stop_1;

// //         motor_cmd.target_angle_2 = last_target_angle_2;
// //         motor_cmd.target_speed_2 = last_target_speed_2;
// //         motor_cmd.enable_2       = last_enable_2;
// //         motor_cmd.stop_2         = last_stop_2;

// //         char* cmd_copy = strdup(cmd + 10);
// //         if (!cmd_copy) return;

// //         char* token = strtok(cmd_copy, ",");
// //         while (token) {
// //             // 【关键步骤 2】只有解析到具体字段时，才更新当前值和“历史记忆”
            
// //             if (strstr(token, "M1_ANGLE=")) {
// //                 float val = atof(strchr(token, '=') + 1);
// //                 motor_cmd.target_angle_1 = val;
// //                 last_target_angle_1 = val; // 更新记忆
// //             } 
// //             else if (strstr(token, "M1_SPEED=")) {
// //                 float val = atof(strchr(token, '=') + 1);
// //                 motor_cmd.target_speed_1 = val;
// //                 last_target_speed_1 = val; // 更新记忆
// //             } 
// //             else if (strstr(token, "M1_ENABLE=")) {
// //                 int val = atoi(strchr(token, '=') + 1);
// //                 motor_cmd.enable_1 = (val != 0);
// //                 last_enable_1 = motor_cmd.enable_1; // 更新记忆
// //             } 
// //             else if (strstr(token, "M1_STOP=")) {
// //                 int val = atoi(strchr(token, '=') + 1);
// //                 motor_cmd.stop_1 = (val != 0);
// //                 last_stop_1 = motor_cmd.stop_1; // 更新记忆
// //             } 
            
// //             else if (strstr(token, "M2_ANGLE=")) {
// //                 float val = atof(strchr(token, '=') + 1);
// //                 motor_cmd.target_angle_2 = val;
// //                 last_target_angle_2 = val; // 更新记忆
// //             } 
// //             else if (strstr(token, "M2_SPEED=")) {
// //                 float val = atof(strchr(token, '=') + 1);
// //                 motor_cmd.target_speed_2 = val;
// //                 last_target_speed_2 = val; // 更新记忆
// //             } 
// //             else if (strstr(token, "M2_ENABLE=")) {
// //                 int val = atoi(strchr(token, '=') + 1);
// //                 motor_cmd.enable_2 = (val != 0);
// //                 last_enable_2 = motor_cmd.enable_2; // 更新记忆
// //             } 
// //             else if (strstr(token, "M2_STOP=")) {
// //                 int val = atoi(strchr(token, '=') + 1);
// //                 motor_cmd.stop_2 = (val != 0);
// //                 last_stop_2 = motor_cmd.stop_2; // 更新记忆
// //             } 
// //             else if (strstr(token, "MODE=")) {
// //                 motor_cmd.control_mode = atoi(strchr(token, '=') + 1);
// //             }
            
// //             token = strtok(NULL, ",");
// //         }
// //         free(cmd_copy);

// //         // 【安全保护】如果触发了 STOP，强制关闭 Enable
// //         if (motor_cmd.stop_1) {
// //             motor_cmd.enable_1 = false;
// //             last_enable_1 = false;
// //         }
// //         if (motor_cmd.stop_2) {
// //             motor_cmd.enable_2 = false;
// //             last_enable_2 = false;
// //         }

// //         // 发送队列
// //         if (msg_queue_send_cmd(g_data->msg_queues.cmd_mq, &motor_cmd) == 0) {
// //             pthread_mutex_lock(&g_data->cmd_mutex);
// //             g_data->last_motor_cmd = motor_cmd;
// //             pthread_cond_signal(&g_data->cmd_cond);
// //             pthread_mutex_unlock(&g_data->cmd_mutex);
// //             // printf("[TCP] 电机命令已入队 (M1_EN=%d, M2_EN=%d)\n", motor_cmd.enable_1, motor_cmd.enable_2);
// //         } else {
// //             perror("[TCP] 电机命令入队失败");
// //         }

// //     } else if (strncmp(cmd, "LOG_MSG:", 8) == 0) {
// //         // 处理日志消息
// //         const char* log_content = cmd + 8;
// //         if (strlen(log_content) == 0) {
// //             printf("[TCP] 日志内容为空\n");
// //             return;
// //         }

// //         LogMsg log_msg = {0};
// //         log_msg.msg_type = MSG_TYPE_LOG_MSG;
// //         strncpy(log_msg.message, log_content, sizeof(log_msg.message) - 1);
// //         log_msg.timestamp = time(NULL);

// //         if (msg_queue_send_log(g_data->msg_queues.log_mq, &log_msg) == 0) {
// //             printf("[TCP] 日志已入队: %s\n", log_msg.message);
// //         } else {
// //             perror("[TCP] 日志入队失败");
// //         }

// //     } else if (strncmp(cmd, "SYS_CMD:", 8) == 0) {
// //         if (strstr(cmd, "SHUTDOWN")) {
// //             printf("[TCP] 收到关机命令\n");
// //             g_data->system_running = false;
// //         } else if (strstr(cmd, "STATUS")) {
// //             printf("[TCP] 收到状态查询命令\n");
// //             // 这里可以添加发送当前状态的逻辑
// //         } else if (strstr(cmd, "RESET")) {
// //             printf("[TCP] 收到复位命令\n");
// //             // 可选：重置静态记忆变量
// //             last_enable_1 = false; last_enable_2 = false;
// //             last_stop_1 = false; last_stop_2 = false;
// //         }
// //     } else {
// //         printf("[TCP] 未知命令: %s\n", cmd);
// //     }
// // }

// // // ======================
// // // 连接服务器
// // // ======================
// // bool connect_to_server(TCPConnection* conn) {
// //     if (!conn) return false;
// //     conn->connected = false;
// //     conn->sockfd = -1;
// //     conn->recv_thread = 0;

// //     conn->sockfd = socket(AF_INET, SOCK_STREAM, 0);
// //     if (conn->sockfd < 0) {
// //         perror("[TCP] socket创建失败");
// //         return false;
// //     }

// //     struct timeval timeout = {2, 0};
// //     setsockopt(conn->sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
// //     setsockopt(conn->sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

// //     struct hostent* server = gethostbyname(REMOTE_HOST);
// //     if (server == NULL) {
// //         fprintf(stderr, "[TCP] 无法解析主机: %s\n", REMOTE_HOST);
// //         close(conn->sockfd);
// //         return false;
// //     }

// //     memset(&conn->server_addr, 0, sizeof(conn->server_addr));
// //     conn->server_addr.sin_family = AF_INET;
// //     memcpy(&conn->server_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
// //     conn->server_addr.sin_port = htons(REMOTE_PORT);

// //     if (connect(conn->sockfd, (struct sockaddr*)&conn->server_addr, sizeof(conn->server_addr)) < 0) {
// //         // printf("[TCP] 连接主板失败: %s:%d\n", REMOTE_HOST, REMOTE_PORT);
// //         close(conn->sockfd);
// //         return false;
// //     }
// //     conn->connected = true;
// //     return true;
// // }

// // // ======================
// // // 发送数据到服务器
// // // ======================
// // bool tcp_send_data(int sockfd, const char* data, size_t len) {
// //     if (sockfd < 0 || !data || len == 0) return false;
// //     ssize_t sent = send(sockfd, data, len, 0);
// //     if (sent < 0) {
// //         perror("[TCP] 发送数据失败");
// //         return false;
// //     }
// //     if ((size_t)sent < len) {
// //         // printf("[TCP] 数据未完全发送: %zd/%zu bytes\n", sent, len);
// //         return false;
// //     }
// //     return true;
// // }

// // // ======================
// // // 断开连接
// // // ======================
// // void tcp_disconnect(TCPConnection* conn) {
// //     if (!conn || conn->sockfd < 0) return;
// //     if (conn->connected) {
// //         const char* msg = "BYE\n";
// //         send(conn->sockfd, msg, strlen(msg), 0);
// //         shutdown(conn->sockfd, SHUT_RDWR);
// //         conn->connected = false;
// //     }
// //     close(conn->sockfd);
// //     conn->sockfd = -1;
// //     // printf("[TCP] 连接已断开\n");
// // }


// #include "shared_data.h"
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <netdb.h>
// #include <string.h>
// #include <unistd.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <pthread.h>
// #include <sys/time.h>
// #include <stdbool.h>
// #include <fcntl.h>
// #include <errno.h>
// #include <time.h>
// #include "ipc_msg_queue.h"

// // ======================
// // 【配置修改】
// // 1. 监听 IP 修改为从机 IP
// // 2. 端口保持不变
// // ======================
// #define LISTEN_IP "192.168.7.1"
// #define LISTEN_PORT 8888
// #define BACKLOG 1

// // ======================
// // 【新增】状态记忆区 (State Retention)
// // 使用 static 变量保存上一次的有效状态，实现“增量更新”
// // ======================
// static float last_target_angle_1 = 0.0;
// static float last_target_speed_1 = 0.0;
// static bool last_enable_1 = false;
// static bool last_stop_1 = false;
// static float last_target_angle_2 = 0.0;
// static float last_target_speed_2 = 0.0;
// static bool last_enable_2 = false;
// static bool last_stop_2 = false;

// // =================================
// // TCP连接管理结构体 (服务端模式)
// // =================================
// typedef struct {
//     int sockfd;
//     struct sockaddr_in client_addr;
//     bool connected;
//     pthread_t recv_thread;
// } TCPConnection;

// void* tcp_recv_thread(void* arg);
// void tcp_disconnect(TCPConnection* conn);
// bool tcp_send_data(int sockfd, const char* data, size_t len);
// void parse_tcp_command(const char* cmd, GlobalData* g_data, int sockfd);

// // ======================
// // 数据上报线程 (服务端模式)
// // ======================
// void* tcp_report_thread(void* arg) {
//     GlobalData* g_data = (GlobalData*)arg;
//     TCPConnection conn = {0};
//     int server_fd = -1;
//     struct sockaddr_in address;
//     int addrlen = sizeof(address);

//     // 1. 创建监听Socket
//     if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
//         perror("socket failed");
//         exit(EXIT_FAILURE);
//     }

//     // 2. 绑定从机 IP: 192.168.7.1:8888
//     address.sin_family = AF_INET;
//     address.sin_addr.s_addr = inet_addr(LISTEN_IP);
//     address.sin_port = htons(LISTEN_PORT);

//     if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
//         perror("bind failed");
//         close(server_fd);
//         exit(EXIT_FAILURE);
//     }

//     // 3. 开始监听
//     if (listen(server_fd, BACKLOG) < 0) {
//         perror("listen");
//         close(server_fd);
//         exit(EXIT_FAILURE);
//     }

//     printf("[TCP] 从机已启动，正在监听 %s:%d，等待主机连接...\n", LISTEN_IP, LISTEN_PORT);

//     while (g_data->system_running) {
//         // 4. 等待主机连接 (阻塞)
//         printf("[TCP] 等待主机连接...\n");
//         if ((conn.sockfd = accept(server_fd, (struct sockaddr *)&conn.client_addr, (socklen_t*)&addrlen)) < 0) {
//             if (g_data->system_running) perror("accept");
//             continue;
//         }

//         conn.connected = true;
//         g_data->tcp_connected = true;
//         printf("[TCP] 主机已连接: %s\n", inet_ntoa(conn.client_addr.sin_addr));

//         // 5. 启动接收线程 (处理命令)
//         if (pthread_create(&conn.recv_thread, NULL, tcp_recv_thread, &conn) != 0) {
//             printf("[TCP] 创建接收线程失败\n");
//             tcp_disconnect(&conn);
//             continue;
//         }

//         // 6. 数据上报循环
//         while (g_data->tcp_connected && g_data->system_running) {
//             EnvMsg env_msg;
//             MotorMsg motor_msg;

//             // 发送环境数据
//             if (msg_queue_receive_env(g_data->msg_queues.env_mq, &env_msg, false) > 0) {
//                 char report[256];
//                 snprintf(report, sizeof(report),
//                          "ENV:TEMP=%.1f,HUM=%.1f,PRESS=%.1f,GAS=%.0f,TS=%llu\n",
//                          env_msg.temperature, env_msg.humidity, env_msg.pressure,
//                          env_msg.gas_resistance, env_msg.timestamp);
//                 if (!tcp_send_data(conn.sockfd, report, strlen(report))) break;
//             }

//             // 发送电机数据
//             if (msg_queue_receive_motor(g_data->msg_queues.motor_mq, &motor_msg, false) > 0) {
//                 char report[512];
//                 snprintf(report, sizeof(report),
//                          "MOTOR:M1_SPD=%.1f,M1_CUR=%.0f,M1_POS=%.1f,M1_V=%.1f,"
//                          "M2_SPD=%.1f,M2_CUR=%.0f,M2_POS=%.1f,M2_V=%.1f,"
//                          "M1_ALARM=0x%08X,M2_ALARM=0x%08X,TS=%llu\n",
//                          motor_msg.speed_rpm_1, motor_msg.current_ma_1, motor_msg.position_deg_1, motor_msg.voltage_1,
//                          motor_msg.speed_rpm_2, motor_msg.current_ma_2, motor_msg.position_deg_2, motor_msg.voltage_2,
//                          motor_msg.alarm_code_1, motor_msg.alarm_code_2, motor_msg.timestamp);
//                 if (!tcp_send_data(conn.sockfd, report, strlen(report))) break;
//             }

//             // 检查连接状态 - 发送心跳包
//             if (!tcp_send_data(conn.sockfd, "\n", 1)) {
//                 g_data->tcp_connected = false;
//                 break;
//             }
//             // 上报频率：10Hz
//             usleep(100000);
//         }

//         // 连接断开，清理
//         g_data->tcp_connected = false;
//         tcp_disconnect(&conn);
//         if (conn.recv_thread) pthread_join(conn.recv_thread, NULL);
//         memset(&conn, 0, sizeof(conn));
//     }

//     tcp_disconnect(&conn);
//     return NULL;
// }

// // ======================
// // 接收命令线程
// // ======================
// void* tcp_recv_thread(void* arg) {
//     TCPConnection* conn = (TCPConnection*)arg;
//     char buffer[1024];
//     if (!conn) {
//         return NULL;
//     }
//     while (conn->connected) {
//         memset(buffer, 0, sizeof(buffer));
//         int n = recv(conn->sockfd, buffer, sizeof(buffer) - 1, 0);
//         if (n <= 0) {
//             if (n == 0) {
//                 printf("[TCP] 服务器主动断开连接\n");
//             } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
//                 continue;
//             } else {
//                 perror("[TCP] 接收数据失败");
//             }
//             conn->connected = false;
//             break;
//         }
//         buffer[n] = '\0';
//         // 移除换行符
//         size_t len = strlen(buffer);
//         while (len > 0 && (buffer[len-1] == '\n' || buffer[len-1] == '\r')) {
//             buffer[--len] = '\0';
//         }
//         printf("[TCP] 收到命令: %s\n", buffer);
//         // 访问全局数据
//         extern GlobalData g_data;
//         parse_tcp_command(buffer, &g_data, conn->sockfd);
//     }
//     return NULL;
// }

// // ======================
// // 统一解析 TCP 命令 (核心修改部分)
// // 支持状态保持：未收到的参数沿用上一次的值
// // ======================
// void parse_tcp_command(const char* cmd, GlobalData* g_data, int sockfd) {
//     if (!cmd || !g_data) {
//         printf("[TCP] 命令或全局数据无效\n");
//         return;
//     }
//     if (strncmp(cmd, "MOTOR_CMD:", 10) == 0) {
//         MotorCmdMsg motor_cmd = {0};
//         motor_cmd.msg_type = MSG_TYPE_MOTOR_CMD;
//         // 【关键步骤 1】初始化为本电机的“历史状态”
//         motor_cmd.target_angle_1 = last_target_angle_1;
//         motor_cmd.target_speed_1 = last_target_speed_1;
//         motor_cmd.enable_1 = last_enable_1;
//         motor_cmd.stop_1 = last_stop_1;
//         motor_cmd.target_angle_2 = last_target_angle_2;
//         motor_cmd.target_speed_2 = last_target_speed_2;
//         motor_cmd.enable_2 = last_enable_2;
//         motor_cmd.stop_2 = last_stop_2;

//         char* cmd_copy = strdup(cmd + 10);
//         if (!cmd_copy) return;
//         char* token = strtok(cmd_copy, ",");
//         while (token) {
//             // 【关键步骤 2】只有解析到具体字段时，才更新当前值和“历史记忆”
//             if (strstr(token, "M1_ANGLE=")) {
//                 float val = atof(strchr(token, '=') + 1);
//                 motor_cmd.target_angle_1 = val;
//                 last_target_angle_1 = val;
//             } else if (strstr(token, "M1_SPEED=")) {
//                 float val = atof(strchr(token, '=') + 1);
//                 motor_cmd.target_speed_1 = val;
//                 last_target_speed_1 = val;
//             } else if (strstr(token, "M1_ENABLE=")) {
//                 int val = atoi(strchr(token, '=') + 1);
//                 motor_cmd.enable_1 = (val != 0);
//                 last_enable_1 = motor_cmd.enable_1;
//             } else if (strstr(token, "M1_STOP=")) {
//                 int val = atoi(strchr(token, '=') + 1);
//                 motor_cmd.stop_1 = (val != 0);
//                 last_stop_1 = motor_cmd.stop_1;
//             } else if (strstr(token, "M2_ANGLE=")) {
//                 float val = atof(strchr(token, '=') + 1);
//                 motor_cmd.target_angle_2 = val;
//                 last_target_angle_2 = val;
//             } else if (strstr(token, "M2_SPEED=")) {
//                 float val = atof(strchr(token, '=') + 1);
//                 motor_cmd.target_speed_2 = val;
//                 last_target_speed_2 = val;
//             } else if (strstr(token, "M2_ENABLE=")) {
//                 int val = atoi(strchr(token, '=') + 1);
//                 motor_cmd.enable_2 = (val != 0);
//                 last_enable_2 = motor_cmd.enable_2;
//             } else if (strstr(token, "M2_STOP=")) {
//                 int val = atoi(strchr(token, '=') + 1);
//                 motor_cmd.stop_2 = (val != 0);
//                 last_stop_2 = motor_cmd.stop_2;
//             } else if (strstr(token, "MODE=")) {
//                 motor_cmd.control_mode = atoi(strchr(token, '=') + 1);
//             }
//             token = strtok(NULL, ",");
//         }
//         free(cmd_copy);
//         // 【安全保护】如果触发了 STOP，强制关闭 Enable
//         if (motor_cmd.stop_1) {
//             motor_cmd.enable_1 = false;
//             last_enable_1 = false;
//         }
//         if (motor_cmd.stop_2) {
//             motor_cmd.enable_2 = false;
//             last_enable_2 = false;
//         }
//         // 发送队列
//         if (msg_queue_send_cmd(g_data->msg_queues.cmd_mq, &motor_cmd) == 0) {
//             pthread_mutex_lock(&g_data->cmd_mutex);
//             g_data->last_motor_cmd = motor_cmd;
//             pthread_cond_signal(&g_data->cmd_cond);
//             pthread_mutex_unlock(&g_data->cmd_mutex);
//         } else {
//             perror("[TCP] 电机命令入队失败");
//         }
//     } else if (strncmp(cmd, "LOG_MSG:", 8) == 0) {
//         // 处理日志消息
//         const char* log_content = cmd + 8;
//         if (strlen(log_content) == 0) {
//             printf("[TCP] 日志内容为空\n");
//             return;
//         }
//         LogMsg log_msg = {0};
//         log_msg.msg_type = MSG_TYPE_LOG_MSG;
//         strncpy(log_msg.message, log_content, sizeof(log_msg.message) - 1);
//         log_msg.timestamp = time(NULL);
//         if (msg_queue_send_log(g_data->msg_queues.log_mq, &log_msg) == 0) {
//             printf("[TCP] 日志已入队: %s\n", log_msg.message);
//         } else {
//             perror("[TCP] 日志入队失败");
//         }
//     } else if (strncmp(cmd, "SYS_CMD:", 8) == 0) {
//         if (strstr(cmd, "SHUTDOWN")) {
//             printf("[TCP] 收到关机命令\n");
//             g_data->system_running = false;
//         } else if (strstr(cmd, "STATUS")) {
//             printf("[TCP] 收到状态查询命令\n");
//         } else if (strstr(cmd, "RESET")) {
//             printf("[TCP] 收到复位命令\n");
//             // 可选：重置静态记忆变量
//             last_enable_1 = false;
//             last_enable_2 = false;
//             last_stop_1 = false;
//             last_stop_2 = false;
//         }
//     } else if (strncmp(cmd, "NTP_REQ", 7) == 0) {
//         // 【新增】NTP 时间同步请求
//         printf("[TCP] 收到 NTP 时间同步请求\n");
//         struct timeval tv;
//         gettimeofday(&tv, NULL);
//         // 假设协议为 NTP_RESP:TIMESTAMP=<秒>.<微秒>
//         char ntp_resp[128];
//         snprintf(ntp_resp, sizeof(ntp_resp), "NTP_RESP:TIMESTAMP=%ld.%06ld\n", tv.tv_sec, tv.tv_usec);
//         tcp_send_data(sockfd, ntp_resp, strlen(ntp_resp));
//     } else {
//         printf("[TCP] 未知命令: %s\n", cmd);
//     }
// }

// // ======================
// // 发送数据到服务器
// // ======================
// bool tcp_send_data(int sockfd, const char* data, size_t len) {
//     if (sockfd < 0 || !data || len == 0) return false;
//     ssize_t sent = send(sockfd, data, len, 0);
//     if (sent < 0) {
//         perror("[TCP] 发送数据失败");
//         return false;
//     }
//     if ((size_t)sent < len) {
//         return false;
//     }
//     return true;
// }

// // ======================
// // 断开连接
// // ======================
// void tcp_disconnect(TCPConnection* conn) {
//     if (!conn || conn->sockfd < 0) return;
//     if (conn->connected) {
//         const char* msg = "BYE\n";
//         send(conn->sockfd, msg, strlen(msg), 0);
//         shutdown(conn->sockfd, SHUT_RDWR);
//         conn->connected = false;
//     }
//     close(conn->sockfd);
//     conn->sockfd = -1;
// }

#include "shared_data.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include "ipc_msg_queue.h"

#define LISTEN_IP "0.0.0.0"
#define LISTEN_PORT 8888
#define BACKLOG 1

// ======================
// 【全局变量】
// ======================
static int g_server_fd = -1;
static volatile sig_atomic_t g_shutdown_requested = 0;

// ======================
// 【状态记忆区】
// ======================
static float last_target_angle_1 = 0.0;
static float last_target_speed_1 = 0.0;
static bool last_enable_1 = false;
static bool last_stop_1 = false;
static float last_target_angle_2 = 0.0;
static float last_target_speed_2 = 0.0;
static bool last_enable_2 = false;
static bool last_stop_2 = false;

typedef struct {
    int sockfd;
    struct sockaddr_in client_addr;
    bool connected;
    pthread_t recv_thread;
} TCPConnection;

void* tcp_recv_thread(void* arg);
void tcp_disconnect(TCPConnection* conn);
bool tcp_send_data(int sockfd, const char* data, size_t len);
void parse_tcp_command(const char* cmd, GlobalData* g_data, int sockfd);

// ======================
// 数据上报线程 (服务端模式)
// ======================
void* tcp_report_thread(void* arg) {
    GlobalData* g_data = (GlobalData*)arg;
    TCPConnection conn = {0};
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // 1. 创建监听Socket
    if ((g_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(g_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(LISTEN_IP);
    address.sin_port = htons(LISTEN_PORT);

    if (bind(g_server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(g_server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(g_server_fd, BACKLOG) < 0) {
        perror("listen");
        close(g_server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[TCP] 从机已启动，正在监听 %s:%d\n", LISTEN_IP, LISTEN_PORT);
    fflush(stdout);

    while (g_data->system_running) {
        // 设置 accept 超时，防止永久阻塞导致无法响应退出信号
        struct timeval timeout;
        timeout.tv_sec = 1; // 1秒超时
        timeout.tv_usec = 0;
        setsockopt(g_server_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

        conn.sockfd = accept(g_server_fd, (struct sockaddr *)&conn.client_addr, (socklen_t*)&addrlen);
        
        // 检查退出条件
        if (!g_data->system_running) {
            printf("[TCP] 检测到退出信号，停止接受新连接。\n");
            break;
        }

        if (conn.sockfd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 超时，继续循环检查 system_running
                continue;
            }
            if (errno != EINTR) {
                break;
            }
            perror("accept error");
            continue;
        }

        conn.connected = true;
        g_data->tcp_connected = true;
        printf("[TCP] 主机已连接: %s\n", inet_ntoa(conn.client_addr.sin_addr));
        fflush(stdout);

        if (pthread_create(&conn.recv_thread, NULL, tcp_recv_thread, &conn) != 0) {
            printf("[TCP] 创建接收线程失败\n");
            tcp_disconnect(&conn);
            continue;
        }

        // 数据上报循环
        while (g_data->tcp_connected && g_data->system_running) {
            EnvMsg env_msg;
            MotorMsg motor_msg;

            if (msg_queue_receive_env(g_data->msg_queues.env_mq, &env_msg, false) > 0) {
                char report[256];
                snprintf(report, sizeof(report),
                         "ENV:TEMP=%.1f,HUM=%.1f,PRESS=%.1f,GAS=%.0f,TS=%llu\n",
                         env_msg.temperature, env_msg.humidity, env_msg.pressure,
                         env_msg.gas_resistance, env_msg.timestamp);
                if (!tcp_send_data(conn.sockfd, report, strlen(report))) break;
            }

            if (msg_queue_receive_motor(g_data->msg_queues.motor_mq, &motor_msg, false) > 0) {
                char report[512];
                snprintf(report, sizeof(report),
                         "MOTOR:M1_SPD=%.1f,M1_CUR=%.0f,M1_POS=%.1f,M1_V=%.1f,"
                         "M2_SPD=%.1f,M2_CUR=%.0f,M2_POS=%.1f,M2_V=%.1f,"
                         "M1_ALARM=0x%08X,M2_ALARM=0x%08X,TS=%llu\n",
                         motor_msg.speed_rpm_1, motor_msg.current_ma_1, motor_msg.position_deg_1, motor_msg.voltage_1,
                         motor_msg.speed_rpm_2, motor_msg.current_ma_2, motor_msg.position_deg_2, motor_msg.voltage_2,
                         motor_msg.alarm_code_1, motor_msg.alarm_code_2, motor_msg.timestamp);
                if (!tcp_send_data(conn.sockfd, report, strlen(report))) break;
            }

            if (!tcp_send_data(conn.sockfd, "\n", 1)) {
                g_data->tcp_connected = false;
                break;
            }
            usleep(100000);
        }

        // 断开当前客户端
        g_data->tcp_connected = false;
        tcp_disconnect(&conn);
        
        // 等待接收线程退出
        if (conn.recv_thread) {
            pthread_join(conn.recv_thread, NULL);
            conn.recv_thread = 0;
        }
        memset(&conn, 0, sizeof(conn));
    }

    // ======================
    // 最终清理
    // ======================
    printf("[TCP] 正在执行最终资源清理...\n");
    fflush(stdout);

    // 确保客户端连接已断开
    if (conn.sockfd >= 0) {
        tcp_disconnect(&conn);
    }
    
    // 确保接收线程已退出
    if (conn.recv_thread) {
        pthread_join(conn.recv_thread, NULL);
    }

    // 关闭监听 Socket
    if (g_server_fd >= 0) {
        close(g_server_fd);
        g_server_fd = -1;
    }

    printf("[TCP] 资源清理完毕，TCP线程安全退出。\n");
    fflush(stdout);
    
    return NULL;
}

// ======================
// 接收命令线程
// ======================
void* tcp_recv_thread(void* arg) {
    TCPConnection* conn = (TCPConnection*)arg;
    char buffer[1024];
    if (!conn) return NULL;

    while (conn->connected) {
        extern GlobalData g_data;
        if (!g_data.system_running) {
            break;
        }

        // 设置 recv 超时，防止永久阻塞
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        setsockopt(conn->sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

        memset(buffer, 0, sizeof(buffer));
        int n = recv(conn->sockfd, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            if (n == 0) {
                printf("[TCP] 主机主动断开连接\n");
            } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            } else {
                // socket 错误或被关闭
            }
            conn->connected = false;
            break;
        }
        buffer[n] = '\0';
        
        size_t len = strlen(buffer);
        while (len > 0 && (buffer[len-1] == '\n' || buffer[len-1] == '\r')) {
            buffer[--len] = '\0';
        }
        if (strlen(buffer) == 0) continue;

        parse_tcp_command(buffer, &g_data, conn->sockfd);
        
        char ack[100];
        snprintf(ack, sizeof(ack), "ACK:CMD_RECEIVED\n");
        tcp_send_data(conn->sockfd, ack, strlen(ack));
    }
    return NULL;
}

// ======================
// 统一解析 TCP 命令
// ======================
void parse_tcp_command(const char* cmd, GlobalData* g_data, int sockfd) {
    if (!cmd || !g_data) return;

    if (strncmp(cmd, "MOTOR_CMD:", 10) == 0) {
        MotorCmdMsg motor_cmd = {0};
        motor_cmd.msg_type = MSG_TYPE_MOTOR_CMD;

        motor_cmd.target_angle_1 = last_target_angle_1;
        motor_cmd.target_speed_1 = last_target_speed_1;
        motor_cmd.enable_1 = last_enable_1;
        motor_cmd.stop_1 = last_stop_1;
        motor_cmd.target_angle_2 = last_target_angle_2;
        motor_cmd.target_speed_2 = last_target_speed_2;
        motor_cmd.enable_2 = last_enable_2;
        motor_cmd.stop_2 = last_stop_2;

        char* cmd_copy = strdup(cmd + 10);
        if (!cmd_copy) return;
        char* token = strtok(cmd_copy, ",");
        while (token) {
            if (strstr(token, "M1_ANGLE=")) {
                float val = atof(strchr(token, '=') + 1);
                motor_cmd.target_angle_1 = val; last_target_angle_1 = val;
            } else if (strstr(token, "M1_SPEED=")) {
                float val = atof(strchr(token, '=') + 1);
                motor_cmd.target_speed_1 = val; last_target_speed_1 = val;
            } else if (strstr(token, "M1_ENABLE=")) {
                int val = atoi(strchr(token, '=') + 1);
                motor_cmd.enable_1 = (val != 0); last_enable_1 = motor_cmd.enable_1;
            } else if (strstr(token, "M1_STOP=")) {
                int val = atoi(strchr(token, '=') + 1);
                motor_cmd.stop_1 = (val != 0); last_stop_1 = motor_cmd.stop_1;
            } else if (strstr(token, "M2_ANGLE=")) {
                float val = atof(strchr(token, '=') + 1);
                motor_cmd.target_angle_2 = val; last_target_angle_2 = val;
            } else if (strstr(token, "M2_SPEED=")) {
                float val = atof(strchr(token, '=') + 1);
                motor_cmd.target_speed_2 = val; last_target_speed_2 = val;
            } else if (strstr(token, "M2_ENABLE=")) {
                int val = atoi(strchr(token, '=') + 1);
                motor_cmd.enable_2 = (val != 0); last_enable_2 = motor_cmd.enable_2;
            } else if (strstr(token, "M2_STOP=")) {
                int val = atoi(strchr(token, '=') + 1);
                motor_cmd.stop_2 = (val != 0); last_stop_2 = motor_cmd.stop_2;
            } else if (strstr(token, "MODE=")) {
                motor_cmd.control_mode = atoi(strchr(token, '=') + 1);
            }
            token = strtok(NULL, ",");
        }
        free(cmd_copy);

        if (motor_cmd.stop_1) { motor_cmd.enable_1 = false; last_enable_1 = false; }
        if (motor_cmd.stop_2) { motor_cmd.enable_2 = false; last_enable_2 = false; }

        if (msg_queue_send_cmd(g_data->msg_queues.cmd_mq, &motor_cmd) == 0) {
            pthread_mutex_lock(&g_data->cmd_mutex);
            g_data->last_motor_cmd = motor_cmd;
            pthread_cond_signal(&g_data->cmd_cond);
            pthread_mutex_unlock(&g_data->cmd_mutex);
        }
    } else if (strncmp(cmd, "LOG_MSG:", 8) == 0) {
        const char* log_content = cmd + 8;
        if (strlen(log_content) == 0) return;
        LogMsg log_msg = {0};
        log_msg.msg_type = MSG_TYPE_LOG_MSG;
        strncpy(log_msg.message, log_content, sizeof(log_msg.message) - 1);
        log_msg.timestamp = time(NULL);
        msg_queue_send_log(g_data->msg_queues.log_mq, &log_msg);
    } else if (strncmp(cmd, "SYS_CMD:", 8) == 0) {
        if (strstr(cmd, "SHUTDOWN")) {
            printf("[TCP] 收到关机命令\n");
            g_data->system_running = false;
        } else if (strstr(cmd, "RESET")) {
            last_enable_1 = false; last_enable_2 = false;
            last_stop_1 = false; last_stop_2 = false;
        }
    } else if (strncmp(cmd, "NTP_REQ", 7) == 0) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        char ntp_resp[128];
        snprintf(ntp_resp, sizeof(ntp_resp), "NTP_RESP:TIMESTAMP=%ld.%06ld\n", tv.tv_sec, tv.tv_usec);
        tcp_send_data(sockfd, ntp_resp, strlen(ntp_resp));
    }
}

bool tcp_send_data(int sockfd, const char* data, size_t len) {
    if (sockfd < 0 || !data || len == 0) return false;
    ssize_t sent = send(sockfd, data, len, MSG_NOSIGNAL);
    return (sent > 0 && (size_t)sent == len);
}

void tcp_disconnect(TCPConnection* conn) {
    if (!conn || conn->sockfd < 0) return;
    if (conn->connected) {
        const char* msg = "BYE\n";
        send(conn->sockfd, msg, strlen(msg), MSG_NOSIGNAL);
        shutdown(conn->sockfd, SHUT_RDWR);
        conn->connected = false;
    }
    close(conn->sockfd);
    conn->sockfd = -1;
}
