// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <sys/types.h>
// #include <sys/wait.h>
// #include <stdint.h>
// #include <stdarg.h>
// #include <math.h>
// #include <signal.h>
// #include <time.h>
// #include <fcntl.h>
// #include <sys/select.h>
// #include <errno.h>

// // -------------------------- 基础配置 --------------------------
// #define CAN_IF "can0"
// #define BAUD "1000000" // 行星版本请改为 "500000"
// #define SERVO_ID_1 0x001
// #define SERVO_ID_2 0x002

// // -------------------------- 协议指令定义 --------------------------
// #define CMD_WRITE       0x01
// #define CMD_WRITE_ACK   0x02
// #define CMD_READ        0x03
// #define CMD_READ_ACK    0x04
// #define CMD_FAST_WRITE  0x05
// #define CMD_NET_MANAGE  0x00

// // -------------------------- 写指令返回状态 --------------------------
// #define WRITE_SUCCESS       0x01
// #define WRITE_FAIL          0x00
// #define WRITE_DATA_ERR      0x03
// #define WRITE_OUT_OF_RANGE  0x04
// #define WRITE_CORRECTED     0x05

// // -------------------------- 快速响应模式 --------------------------
// #define FAST_MODE_READ   0x01  // 快速读模式
// #define FAST_MODE_WRITE  0x02  // 快速写模式（不等待响应）
// #define NORMAL_MODE      0x00  // 正常模式（等待响应）

// // -------------------------- 寄存器地址 (根据协议文档 V1.0 整理) --------------------------

// // 0x00 ~ 0x0F
// #define ADDR_DEV_SN         0x02  // 设备序列号 (R, 32位)
// #define ADDR_HW_VERSION     0x03  // 硬件版本号 (R, 32位)
// #define ADDR_FW_VERSION     0x04  // 固件版本号 (R, 32位)
// #define ADDR_CURRENT        0x05  // 当前电流 (R, 32位有符号, 单位: mA)
// #define ADDR_SPEED          0x06  // 当前速度 (R, 32位有符号, 单位: 脉冲/秒)
// #define ADDR_POSITION       0x07  // 当前位置 (R, 32位有符号, 单位: 脉冲)
// #define ADDR_TARGET_CURRENT 0x08  // 目标电流 (R/W, 32位有符号, 单位: mA)
// #define ADDR_TARGET_SPEED   0x09  // 目标速度 (R/W, 32位有符号, 单位: 脉冲/秒)
// #define ADDR_TARGET_POS     0x0A  // 目标位置 (R/W, 32位有符号, 单位: 脉冲)
// #define ADDR_ACCEL          0x0B  // 加速度 (R/W, 32位, 单位: 脉冲/秒²)
// #define ADDR_DECEL          0x0C  // 减速度 (R/W, 32位, 单位: 脉冲/秒²)
// #define ADDR_WORK_MODE      0x0F  // 工作模式 (R/W, 32位)

// // 0x10 ~ 0x1F
// #define ADDR_ENABLE         0x10  // 使能/失能 (R/W, 32位, 0x00=失能, 0x01=使能)
// #define ADDR_STOP_RUN       0x11  // 停止运行 (R/W, 32位, 0x01=停止)
// #define ADDR_POSITION_OFFSET 0x12 // 位置偏移 (R/W, 32位有符号, 单位: 脉冲)
// #define ADDR_ALARM_CODE     0x15  // 告警代码 (R/W, 32位, 写0x00清除)
// #define ADDR_GEAR_RATIO     0x1A  // 电子齿轮比 (R, 32位)
// #define ADDR_BUS_VOLTAGE    0x1B  // 母线电压 (R, 32位, 单位: 0.1V)

// // 0x30 ~ 0x3F (重要的参数配置寄存器)
// #define ADDR_MAX_CURRENT    0x30  // 最大电流限制 (R/W, 32位, 单位: mA)
// #define ADDR_MAX_SPEED      0x31  // 最大速度限制 (R/W, 32位, 单位: 脉冲/秒)
// #define ADDR_POSITION_LOOP_KP 0x23 // 位置环比例增益KP (R/W, 32位)
// #define ADDR_POSITION_LOOP_KD 0x24 // 位置环微分增益KD (R/W, 32位)
// #define ADDR_SPEED_LOOP_KP  0x25  // 速度环比例增益KP (R/W, 32位)
// #define ADDR_SPEED_LOOP_KI  0x26  // 速度环积分增益KI (R/W, 32位)
// #define ADDR_CURRENT_LOOP_KP 0x27 // 电流环比例增益KP (R/W, 32位)
// #define ADDR_CURRENT_LOOP_KI 0x28 // 电流环积分增益KI (R/W, 32位)
// #define ADDR_HOMING_MODE    0x38  // 回零模式 (R/W, 32位)
// #define ADDR_HOMING_SPEED   0x39  // 回零速度 (R/W, 32位, 单位: 脉冲/秒)
// #define ADDR_CAN_ID         0x4B  // CAN ID (R/W, 32位) <--- 重要：用于修改ID
// #define ADDR_FILTER_LOW     0x3B  // 滤波下限 (R/W, 32位)
// #define ADDR_FILTER_HIGH    0x3C  // 滤波上限 (R/W, 32位)
// #define ADDR_ENCODER_DIR    0x3D  // 编码器方向 (R/W, 32位, 0x00=默认, 0x01=反向)
// #define ADDR_MOTOR_DIR      0x3E  // 电机方向 (R/W, 32位, 0x00=默认, 0x01=反向)

// // 工作模式定义
// #define MODE_POSITION       0x01  // 位置模式
// #define MODE_SPEED          0x03  // 速度模式
// #define MODE_CURRENT        0x04  // 电流模式

// // 角度范围
// #define MIN_ANGLE          -180.0f
// #define MAX_ANGLE          180.0f
// #define PULSES_PER_REVOLUTION 65536  // 16位编码器

// // -------------------------- 全局模式设置 --------------------------
// static int g_fast_mode = 0;  // 0=正常, 1=快速读, 2=快速写

// // -------------------------- 优化工具函数 --------------------------
// void run_cmd_fmt(const char* fmt, ...) {
//     char cmd[256];
//     va_list args;
//     va_start(args, fmt);
//     vsnprintf(cmd, sizeof(cmd)-1, fmt, args);
//     cmd[sizeof(cmd)-1] = '\0';
//     va_end(args);
//     system(cmd);
// }

// void set_fast_mode(int mode) {
//     g_fast_mode = mode;
//     switch (mode) {
//         case FAST_MODE_READ:
//             printf("[INFO] 切换到快速读模式\n");
//             break;
//         case FAST_MODE_WRITE:
//             printf("[INFO] 切换到快速写模式（不等待响应）\n");
//             break;
//         default:
//             printf("[INFO] 切换到正常模式\n");
//             break;
//     }
// }

// // -------------------------- 优化的CAN操作 --------------------------
// int can_init() {
//     // 快速初始化CAN
//     run_cmd_fmt("ip link set %s down 2>/dev/null", CAN_IF);
//     run_cmd_fmt("ip link set %s type can bitrate %s 2>/dev/null", CAN_IF, BAUD);
//     run_cmd_fmt("ip link set %s up 2>/dev/null", CAN_IF);
//     usleep(50000);  // 50ms等待
//     return 0;
// }

// // 优化的发送函数
// int can_send_cmd_opt(uint32_t id, uint8_t cmd, uint8_t addr, uint32_t data, int wait_response) {
//     uint8_t can_data[6] = {cmd, addr, (data>>24)&0xFF, (data>>16)&0xFF, (data>>8)&0xFF, data&0xFF};
//     char cmd_str[256];
    
//     if (wait_response) {
//         snprintf(cmd_str, sizeof(cmd_str), "cansend %s %03X#%02X%02X%02X%02X%02X%02X",
//                  CAN_IF, id, can_data[0], can_data[1], can_data[2], 
//                  can_data[3], can_data[4], can_data[5]);
//     } else {
//         // 快速模式：后台发送，不等待
//         snprintf(cmd_str, sizeof(cmd_str), 
//                  "cansend %s %03X#%02X%02X%02X%02X%02X%02X >/dev/null 2>&1 &",
//                  CAN_IF, id, can_data[0], can_data[1], can_data[2], 
//                  can_data[3], can_data[4], can_data[5]);
//     }
    
//     return system(cmd_str) == 0 ? 0 : -1;
// }

// // 快速写函数（不等待响应）
// int can_fast_write(uint32_t id, uint8_t addr, uint32_t data) {
//     return can_send_cmd_opt(id, CMD_WRITE, addr, data, 0);  // 不等待响应
// }

// // 批量快速写入
// int can_batch_fast_write(uint32_t id, uint8_t* addrs, uint32_t* values, int count) {
//     for (int i = 0; i < count; i++) {
//         if (can_fast_write(id, addrs[i], values[i]) != 0) {
//             return -1;
//         }
//         usleep(2000);  // 微小延迟避免总线拥塞
//     }
//     return 0;
// }

// // -------------------------- 优化的响应解析函数 --------------------------

// // 快速监听函数（使用非阻塞IO）
// char* can_start_fast_listen(uint32_t id, pid_t* pid_out) {
//     static char dump_file[64];
//     int fd;
    
//     // 创建临时文件
//     snprintf(dump_file, sizeof(dump_file), "/tmp/can_fast_%d_XXXXXX", getpid());
//     fd = mkstemp(dump_file);
//     if (fd < 0) {
//         perror("[ERR] 创建临时文件失败");
//         return NULL;
//     }
//     close(fd);
    
//     // 启动candump后台进程
//     char cmd[256];
//     snprintf(cmd, sizeof(cmd), "candump %s,0x%X:0x%X -t a -n 1 > %s 2>/dev/null & echo $!",
//              CAN_IF, id, id, dump_file);
    
//     FILE* fp = popen(cmd, "r");
//     if (fp) {
//         if (pid_out && fscanf(fp, "%d", pid_out) == 1) {
//             // 成功获取PID
//         }
//         pclose(fp);
//     }
    
//     usleep(10000);  // 10ms等待监听启动
//     return dump_file;
// }

// // 快速读响应解析（带超时）
// int can_fast_parse_read(const char* dump_file, uint32_t id, uint8_t addr, uint32_t* out_data) {
//     if (!dump_file || !out_data) return -1;
    
//     struct timespec start, now;
//     clock_gettime(CLOCK_MONOTONIC, &start);
//     const long timeout_ns = 200000000;  // 200ms超时
    
//     while (1) {
//         FILE* fp = fopen(dump_file, "r");
//         if (!fp) {
//             usleep(10000);
//             continue;
//         }
        
//         char buf[256];
//         while (fgets(buf, sizeof(buf), fp)) {
//             uint32_t resp_id;
//             uint8_t resp_data[8];
//             int dlc;
            
//             if (sscanf(buf, "%*[^)] )  %*s %X   [%d]  %02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
//                        &resp_id, &dlc,
//                        &resp_data[0], &resp_data[1], &resp_data[2], &resp_data[3],
//                        &resp_data[4], &resp_data[5], &resp_data[6], &resp_data[7]) >= 8) {
                
//                 if (resp_id == id && resp_data[0] == CMD_READ_ACK && resp_data[1] == addr) {
//                     *out_data = ((uint32_t)resp_data[2] << 24) |
//                                 ((uint32_t)resp_data[3] << 16) |
//                                 ((uint32_t)resp_data[4] << 8) |
//                                 (uint32_t)resp_data[5];
                    
//                     fclose(fp);
//                     unlink(dump_file);  // 删除临时文件
//                     return 0;
//                 }
//             }
//         }
        
//         fclose(fp);
        
//         // 检查超时
//         clock_gettime(CLOCK_MONOTONIC, &now);
//         long elapsed = (now.tv_sec - start.tv_sec) * 1000000000L + 
//                        (now.tv_nsec - start.tv_nsec);
        
//         if (elapsed > timeout_ns) {
//             printf("[TIMEOUT] 读响应超时 (ADDR=0x%02X)\n", addr);
//             unlink(dump_file);
//             return -1;
//         }
        
//         usleep(5000);  // 5ms轮询间隔
//     }
// }

// // 优化的读函数
// int can_fast_read(uint32_t id, uint8_t addr, uint32_t* out_data) {
//     pid_t pid;
//     char* dump_file = can_start_fast_listen(id, &pid);
//     if (!dump_file) return -1;
    
//     // 发送读命令
//     if (can_send_cmd_opt(id, CMD_READ, addr, 0, 1) != 0) {
//         kill(pid, SIGTERM);
//         unlink(dump_file);
//         return -1;
//     }
    
//     // 快速解析响应
//     int ret = can_fast_parse_read(dump_file, id, addr, out_data);
    
//     // 确保清理
//     if (pid > 0) kill(pid, SIGTERM);
//     return ret;
// }

// // -------------------------- 优化的角度转换函数 --------------------------

// // -180~180度转脉冲（你的原有逻辑）
// int32_t degree_to_pulse(float degree) {
//     if (degree == MAX_ANGLE) {
//         return 0x00008000;  // 180度特殊处理
//     }
//     return (int32_t)(((degree - MIN_ANGLE) / (MAX_ANGLE - MIN_ANGLE)) * PULSES_PER_REVOLUTION) - 0x8000;
// }

// // 脉冲转-180~180度
// float pulse_to_degree(int32_t pulse) {
//     return ((float)pulse + 0x8000) * (MAX_ANGLE - MIN_ANGLE) / PULSES_PER_REVOLUTION + MIN_ANGLE;
// }

// // RPM转脉冲/秒
// uint32_t rpm_to_pulse_per_sec(float rpm) {
//     return (uint32_t)(rpm * PULSES_PER_REVOLUTION / 60.0f);
// }

// // 加速度RPM/s转脉冲/秒²
// uint32_t accel_rpm_to_pulse(float accel_rpm_s) {
//     return (uint32_t)(accel_rpm_s * PULSES_PER_REVOLUTION / 60.0f);
// }

// // -------------------------- 优化的业务功能函数 ---------------------------

// // 快速读取设备信息
// void fast_read_device_info(uint32_t id) {
//     uint32_t sn, hw_ver, fw_ver;
//     printf("\n[FAST] 读取设备信息 (ID: 0x%X)\n", id);
    
//     if (can_fast_read(id, ADDR_DEV_SN, &sn) == 0) {
//         printf("  序列号: 0x%08X\n", sn);
//     }
    
//     if (can_fast_read(id, ADDR_HW_VERSION, &hw_ver) == 0) {
//         printf("  硬件版本: V%d.%d\n", (hw_ver >> 16) & 0xFFFF, hw_ver & 0xFFFF);
//     }
    
//     if (can_fast_read(id, ADDR_FW_VERSION, &fw_ver) == 0) {
//         printf("  固件版本: V%d.%d\n", (fw_ver >> 16) & 0xFFFF, fw_ver & 0xFFFF);
//     }
// }

// // 快速读取实时状态（电流、速度、位置、电压）
// void fast_read_real_time_status(uint32_t id) {
//     uint32_t current, speed, position, voltage;
//     printf("\n[FAST] 实时状态 (ID: 0x%X)\n", id);
    
//     // 并行读取（实际是串行但很快）
//     if (can_fast_read(id, ADDR_CURRENT, &current) == 0) {
//         printf("  电流: %d mA\n", (int32_t)current);
//     }
    
//     if (can_fast_read(id, ADDR_SPEED, &speed) == 0) {
//         int32_t signed_speed = (int32_t)speed;
//         float rpm = (float)signed_speed * 60.0f / PULSES_PER_REVOLUTION;
//         printf("  速度: %d pps (%.1f rpm)\n", signed_speed, rpm);
//     }
    
//     if (can_fast_read(id, ADDR_POSITION, &position) == 0) {
//         int32_t signed_pos = (int32_t)position;
//         float angle = pulse_to_degree(signed_pos);
//         printf("  位置: %d pulse (%.1f°)\n", signed_pos, angle);
//     }
    
//     if (can_fast_read(id, ADDR_BUS_VOLTAGE, &voltage) == 0) {
//         printf("  电压: %.1f V\n", voltage * 0.1f);
//     }
// }

// // 快速批量读取（一次读取多个寄存器）
// int fast_batch_read(uint32_t id, uint8_t* addrs, uint32_t* results, int count) {
//     printf("[FAST] 批量读取 %d 个寄存器\n", count);
    
//     for (int i = 0; i < count; i++) {
//         if (can_fast_read(id, addrs[i], &results[i]) != 0) {
//             printf("  寄存器 0x%02X: 读取失败\n", addrs[i]);
//             results[i] = 0xFFFFFFFF;
//         }
//     }
//     return 0;
// }

// // -------------------------- 优化的运动控制函数 --------------------------

// // 快速配置位置模式（不等待响应）
// int fast_config_position_mode(uint32_t id, float target_degree, float speed_rpm, 
//                             float accel_rpm_s, float decel_rpm_s) {
//     // 参数转换
//     int32_t target_pulse = degree_to_pulse(target_degree);
//     uint32_t speed_pulse = rpm_to_pulse_per_sec(speed_rpm);
//     uint32_t accel_pulse = accel_rpm_to_pulse(accel_rpm_s);
//     uint32_t decel_pulse = accel_rpm_to_pulse(decel_rpm_s);
    
//     printf("[FAST] 配置位置模式: %.1f° @ %.1f rpm\n", target_degree, speed_rpm);
    
//     // 批量快速写入（不等待响应）
//     uint8_t addrs[] = {
//         ADDR_WORK_MODE,
//         ADDR_TARGET_SPEED,
//         ADDR_ACCEL,
//         ADDR_DECEL,
//         ADDR_TARGET_POS
//     };
    
//     uint32_t values[] = {
//         MODE_POSITION,
//         speed_pulse,
//         accel_pulse,
//         decel_pulse,
//         (uint32_t)target_pulse
//     };
    
//     return can_batch_fast_write(id, addrs, values, 5);
// }

// // 快速配置速度模式
// int fast_config_speed_mode(uint32_t id, float speed_rpm, 
//                          float accel_rpm_s, float decel_rpm_s) {
//     int32_t speed_pulse = (int32_t)rpm_to_pulse_per_sec(speed_rpm);
//     uint32_t accel_pulse = accel_rpm_to_pulse(accel_rpm_s);
//     uint32_t decel_pulse = accel_rpm_to_pulse(decel_rpm_s);
    
//     printf("[FAST] 配置速度模式: %.1f rpm\n", speed_rpm);
    
//     uint8_t addrs[] = {
//         ADDR_WORK_MODE,
//         ADDR_ACCEL,
//         ADDR_DECEL,
//         ADDR_TARGET_SPEED
//     };
    
//     uint32_t values[] = {
//         MODE_SPEED,
//         accel_pulse,
//         decel_pulse,
//         (uint32_t)speed_pulse
//     };
    
//     return can_batch_fast_write(id, addrs, values, 4);
// }

// // 快速配置电流模式（力控）
// int fast_config_current_mode(uint32_t id, int32_t current_ma) {
//     printf("[FAST] 配置电流模式: %d mA\n", current_ma);
    
//     uint8_t addrs[] = {
//         ADDR_WORK_MODE,
//         ADDR_TARGET_CURRENT
//     };
    
//     uint32_t values[] = {
//         MODE_CURRENT,
//         (uint32_t)current_ma
//     };
    
//     return can_batch_fast_write(id, addrs, values, 2);
// }

// // 力位混合模式（位置+力矩限制）
// int fast_config_force_position_mode(uint32_t id, float target_degree, 
//                                   float speed_rpm, int32_t max_current_ma) {
//     int32_t target_pulse = degree_to_pulse(target_degree);
//     uint32_t speed_pulse = rpm_to_pulse_per_sec(speed_rpm);
    
//     printf("[FAST] 力位模式: %.1f° @ %.1f rpm, 限流 %d mA\n", 
//            target_degree, speed_rpm, max_current_ma);
    
//     uint8_t addrs[] = {
//         ADDR_WORK_MODE,
//         ADDR_TARGET_SPEED,
//         ADDR_TARGET_POS,
//         ADDR_MAX_CURRENT
//     };
    
//     uint32_t values[] = {
//         MODE_POSITION,
//         speed_pulse,
//         (uint32_t)target_pulse,
//         (uint32_t)max_current_ma
//     };
    
//     return can_batch_fast_write(id, addrs, values, 4);
// }

// // 快速使能/失能
// int fast_enable_motor(uint32_t id, int enable) {
//     printf("[FAST] %s电机 0x%X\n", enable ? "使能" : "失能", id);
//     return can_fast_write(id, ADDR_ENABLE, enable ? 0x01 : 0x00);
// }

// // 快速停止
// int fast_stop_motor(uint32_t id) {
//     printf("[FAST] 停止电机 0x%X\n", id);
//     return can_fast_write(id, ADDR_STOP_RUN, 0x01);
// }

// // 快速设置保护参数
// int fast_set_protection(uint32_t id, uint32_t max_current_ma, uint32_t max_speed_pulse) {
//     printf("[FAST] 设置保护: 电机 0x%X, 限流 %d mA, 限速 %d pps\n", 
//            id, max_current_ma, max_speed_pulse);
    
//     uint8_t addrs[] = {ADDR_MAX_CURRENT, ADDR_MAX_SPEED};
//     uint32_t values[] = {max_current_ma, max_speed_pulse};
    
//     return can_batch_fast_write(id, addrs, values, 2);
// }

// // -------------------------- 优化的角度控制函数 --------------------------

// // 快速角度控制（使用你的-180~180度范围，电机自动选择最短路径）
// int fast_move_to_angle(uint32_t id, float target_degree, float speed_rpm, 
//                       float accel_rpm_s, float decel_rpm_s) {
//     // 参数检查
//     if (target_degree < MIN_ANGLE || target_degree > MAX_ANGLE) {
//         printf("[ERROR] 角度超出范围 (%.1f°), 有效范围: %.0f~%.0f°\n", 
//                target_degree, MIN_ANGLE, MAX_ANGLE);
//         return -1;
//     }
    
//     if (speed_rpm <= 0 || accel_rpm_s <= 0 || decel_rpm_s <= 0) {
//         printf("[ERROR] 运动参数必须为正数\n");
//         return -1;
//     }
    
//     // 电机内部会自动选择最短路径，我们只需要发送目标位置
//     return fast_config_position_mode(id, target_degree, speed_rpm, accel_rpm_s, decel_rpm_s);
// }

// // 力控角度运动
// int force_move_to_angle(uint32_t id, float target_degree, float speed_rpm, int32_t max_current_ma) {
//     if (target_degree < MIN_ANGLE || target_degree > MAX_ANGLE) {
//         printf("[ERROR] 角度超出范围\n");
//         return -1;
//     }
    
//     return fast_config_force_position_mode(id, target_degree, speed_rpm, max_current_ma);
// }

// // 快速读取当前位置
// int fast_read_current_position(uint32_t id, float* angle_out, int32_t* pulse_out) {
//     uint32_t pulse;
//     if (can_fast_read(id, ADDR_POSITION, &pulse) != 0) {
//         return -1;
//     }
    
//     int32_t signed_pulse = (int32_t)pulse;
//     float angle = pulse_to_degree(signed_pulse);
    
//     if (angle_out) *angle_out = angle;
//     if (pulse_out) *pulse_out = signed_pulse;
    
//     return 0;
// }

// // 运动序列（快速连续运动）
// int execute_fast_motion_sequence(uint32_t id, float* angles, float* speeds, 
//                                float* accelerations, int count, int delay_ms) {
//     printf("[SEQ] 执行运动序列，%d个点\n", count);
    
//     for (int i = 0; i < count; i++) {
//         printf("  点%d: %.1f° @ %.1f rpm\n", i+1, angles[i], speeds[i]);
        
//         if (fast_move_to_angle(id, angles[i], speeds[i], 
//                               accelerations[i], accelerations[i]) != 0) {
//             printf("[SEQ] 第%d点失败\n", i+1);
//             return -1;
//         }
        
//         usleep(delay_ms * 1000);
//     }
    
//     return 0;
// }

// // -------------------------- 双电机协调控制 --------------------------

// // 双电机同步运动
// int dual_motor_fast_move(float angle1, float angle2, float speed_rpm) {
//     printf("[SYNC] 双电机同步: 电机1->%.1f°, 电机2->%.1f° @ %.1f rpm\n", 
//            angle1, angle2, speed_rpm);
    
//     // 同时发送（快速写模式）
//     int ret1 = fast_move_to_angle(SERVO_ID_1, angle1, speed_rpm, 1000.0f, 1000.0f);
//     int ret2 = fast_move_to_angle(SERVO_ID_2, angle2, speed_rpm, 1000.0f, 1000.0f);
    
//     return (ret1 == 0 && ret2 == 0) ? 0 : -1;
// }

// // 双电机力控同步
// int dual_motor_force_move(float angle1, float angle2, float speed_rpm, int32_t max_current) {
//     printf("[FORCE_SYNC] 双电机力控同步\n");
    
//     int ret1 = force_move_to_angle(SERVO_ID_1, angle1, speed_rpm, max_current);
//     int ret2 = force_move_to_angle(SERVO_ID_2, angle2, speed_rpm, max_current);
    
//     return (ret1 == 0 && ret2 == 0) ? 0 : -1;
// }

// // -------------------------- 告警处理 --------------------------

// int fast_alarm_check(uint32_t id) {
//     uint32_t alarm_code;
    
//     if (can_fast_read(id, ADDR_ALARM_CODE, &alarm_code) != 0) {
//         return -1;
//     }
    
//     if (alarm_code == 0) {
//         printf("[OK] 电机 0x%X 无告警\n", id);
//         return 0;
//     }
    
//     printf("[ALARM] 电机 0x%X 告警码: 0x%08X\n", id, alarm_code);
    
//     // 尝试清除告警
//     can_fast_write(id, ADDR_ALARM_CODE, 0x00);
    
//     return 1;  // 有告警
// }

// // -------------------------- 主函数演示 --------------------------
// int main() {
//     if (getuid() != 0) {
//         printf("错误：需root权限运行！\n");
//         return -1;
//     }
    
//     printf("=== 优化的电机控制系统 ===\n");
//     printf("角度范围: %.0f~%.0f度\n", MIN_ANGLE, MAX_ANGLE);
//     printf("编码器分辨率: %d 脉冲/转\n\n", PULSES_PER_REVOLUTION);
    
//     // 初始化CAN
//     printf("[INIT] 初始化CAN接口...\n");
//     can_init();
    
//     // 切换到快速模式
//     set_fast_mode(FAST_MODE_READ | FAST_MODE_WRITE);
    
//     // 1. 快速读取设备信息
//     printf("\n=== 快速读取设备信息 ===\n");
//     fast_read_device_info(SERVO_ID_1);
//     fast_read_device_info(SERVO_ID_2);
    
//     // 2. 设置保护参数（快速写，不等待）
//     printf("\n=== 设置保护参数 ===\n");
//     fast_set_protection(SERVO_ID_1, 1400, rpm_to_pulse_per_sec(200.0f));
//     fast_set_protection(SERVO_ID_2, 500, rpm_to_pulse_per_sec(150.0f));
    
//     // 3. 使能电机
//     printf("\n=== 使能电机 ===\n");
//     fast_enable_motor(SERVO_ID_1, 1);
//     fast_enable_motor(SERVO_ID_2, 1);
//     sleep(1);
    
//     // 4. 快速运动演示
//     printf("\n=== 快速运动演示 ===\n");
    
//     // 电机1运动序列
//     float angles1[] = {0.0f, 15.0f, -15.0f, 0.0f, -20.0f, 10.0f};
//     float speeds1[] = {50.0f, 100.0f, 100.0f, 80.0f, 80.0f, 50.0f};
//     float accels1[] = {1000.0f, 1000.0f, 1000.0f, 1000.0f, 1000.0f, 1000.0f};
    
//     printf("电机1运动序列:\n");
//     for (int i = 0; i < 6; i++) {
//         printf("  移动到 %.1f° @ %.1f rpm\n", angles1[i], speeds1[i]);
//         fast_move_to_angle(SERVO_ID_1, angles1[i], speeds1[i], accels1[i], accels1[i]);
//         sleep(1);
        
//         // 快速读取当前位置
//         float current_angle;
//         if (fast_read_current_position(SERVO_ID_1, &current_angle, NULL) == 0) {
//             printf("    当前位置: %.1f°\n", current_angle);
//         }
//     }
    
//     // // 5. 双电机同步演示
//     // printf("\n=== 双电机同步演示 ===\n");
//     // dual_motor_fast_move(30.0f, -30.0f, 80.0f);
//     // sleep(2);
//     // dual_motor_fast_move(0.0f, 0.0f, 100.0f);
//     // sleep(2);
    
//     // 6. 力控演示
//     printf("\n=== 力控演示 ===\n");
//     printf("电机1: 角度控制 + 力矩限制 (500mA)\n");
//     force_move_to_angle(SERVO_ID_1, 60.0f, 50.0f, 1400);
//     sleep(2);
    
//     // // 7. 连续旋转演示（速度模式）
//     // printf("\n=== 速度模式演示 ===\n");
//     // printf("电机2: 连续旋转 @ 30 rpm\n");
//     // fast_config_speed_mode(SERVO_ID_2, 30.0f, 500.0f, 500.0f);
//     // sleep(3);
    
//     // 停止旋转
//     // fast_stop_motor(SERVO_ID_2);
    
//     // 8. 电流模式演示（力控）
//     // printf("\n=== 电流模式演示 ===\n");
//     // printf("电机2: 恒力矩控制 @ 200 mA\n");
//     // fast_config_current_mode(SERVO_ID_2, 200);
//     // sleep(2);
    
//     // 9. 最终状态读取
//     printf("\n=== 最终状态 ===\n");
//     fast_read_real_time_status(SERVO_ID_1);
//     fast_read_real_time_status(SERVO_ID_2);
    
//     // 10. 告警检查
//     printf("\n=== 告警检查 ===\n");
//     fast_alarm_check(SERVO_ID_1);
//     fast_alarm_check(SERVO_ID_2);
    
//     // 11. 安全停止
//     printf("\n=== 安全停止 ===\n");
//     fast_stop_motor(SERVO_ID_1);
//     fast_stop_motor(SERVO_ID_2);
//     fast_enable_motor(SERVO_ID_1, 0);
//     fast_enable_motor(SERVO_ID_2, 0);
    
//     printf("\n=== 演示完成 ===\n");
//     return 0;
// }


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdint.h>
#include <stdarg.h>
#include <math.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>

// -------------------------- 基础配置 --------------------------
#define CAN_IF "can0"
#define BAUD "1000000" // 行星版本请改为 "500000"
#define SERVO_ID_1 0x001
#define SERVO_ID_2 0x002
#define SERVO_ID_3 0x003
// -------------------------- 协议指令定义 --------------------------
#define CMD_WRITE       0x01
#define CMD_WRITE_ACK   0x02
#define CMD_READ        0x03
#define CMD_READ_ACK    0x04

// -------------------------- 写指令返回状态 --------------------------
#define WRITE_SUCCESS       0x01
#define WRITE_FAIL          0x00
#define WRITE_DATA_ERR      0x03
#define WRITE_OUT_OF_RANGE  0x04
#define WRITE_CORRECTED     0x05

// -------------------------- 寄存器地址 --------------------------
#define ADDR_DEV_SN         0x02  // 设备序列号
#define ADDR_HW_VERSION     0x03  // 硬件版本号
#define ADDR_FW_VERSION     0x04  // 固件版本号
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

// -------------------------- 全局变量 --------------------------
static int g_debug_mode = 0;

// -------------------------- 工具函数 --------------------------
void run_cmd_fmt(const char* fmt, ...) {
    char cmd[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(cmd, sizeof(cmd)-1, fmt, args);
    cmd[sizeof(cmd)-1] = '\0';
    va_end(args);
    
    if (g_debug_mode) {
        printf("[CMD] %s\n", cmd);
    }
    
    system(cmd);
}

void set_debug_mode(int enable) {
    g_debug_mode = enable;
    printf("[INFO] 调试模式 %s\n", enable ? "开启" : "关闭");
}

// -------------------------- CAN基础操作 --------------------------
int can_init() {
    printf("[INIT] 初始化CAN接口 %s @ %s bps\n", CAN_IF, BAUD);
    
    // 关闭现有接口
    run_cmd_fmt("ip link set %s down 2>/dev/null", CAN_IF);
    
    // 设置CAN参数
    run_cmd_fmt("ip link set %s type can bitrate %s 2>/dev/null", CAN_IF, BAUD);
    
    // 启用接口
    run_cmd_fmt("ip link set %s up 2>/dev/null", CAN_IF);
    
    // 等待接口启动
    usleep(100000);
    
    // 检查接口状态
    int ret = system("ip link show can0 2>/dev/null | grep -q 'UP'");
    if (ret == 0) {
        printf("[OK] CAN接口初始化成功\n");
        return 0;
    } else {
        printf("[ERROR] CAN接口初始化失败\n");
        return -1;
    }
}

// 快速发送命令（不等待响应）
int can_fast_send(uint32_t id, uint8_t cmd, uint8_t addr, uint32_t data) {
    uint8_t can_data[6] = {
        cmd, 
        addr, 
        (data >> 24) & 0xFF,
        (data >> 16) & 0xFF,
        (data >> 8) & 0xFF,
        data & 0xFF
    };
    
    char cmd_str[256];
    snprintf(cmd_str, sizeof(cmd_str),
             "cansend %s %03X#%02X%02X%02X%02X%02X%02X >/dev/null 2>&1",
             CAN_IF, id,
             can_data[0], can_data[1], can_data[2],
             can_data[3], can_data[4], can_data[5]);
    
    if (g_debug_mode) {
        printf("[SEND] ID:0x%03X CMD:0x%02X ADDR:0x%02X DATA:0x%08X\n", 
               id, cmd, addr, data);
    }
    
    return system(cmd_str) == 0 ? 0 : -1;
}

// 批量快速写入
int can_batch_write(uint32_t id, uint8_t* addrs, uint32_t* values, int count) {
    if (!addrs || !values || count <= 0) {
        return -1;
    }
    
    for (int i = 0; i < count; i++) {
        if (can_fast_send(id, CMD_WRITE, addrs[i], values[i]) != 0) {
            printf("[ERROR] 批量写入第%d个失败 (ADDR=0x%02X)\n", i, addrs[i]);
            return -1;
        }
        usleep(3000);  // 3ms间隔，避免总线拥塞
    }
    
    return 0;
}

// -------------------------- 优化的读操作 --------------------------

// 创建临时监听文件
char* create_listen_file() {
    static char filename[64];
    snprintf(filename, sizeof(filename), "/tmp/can_read_%d_XXXXXX", getpid());
    
    int fd = mkstemp(filename);
    if (fd < 0) {
        perror("[ERROR] 创建临时文件失败");
        return NULL;
    }
    close(fd);
    
    return filename;
}

// 启动candump监听
pid_t start_candump(uint32_t id, const char* filename) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd),
             "candump %s,0x%X:0x%X -td -L -n 50 > %s 2>/dev/null & echo $!",
             CAN_IF, id, id, filename);
    
    FILE* fp = popen(cmd, "r");
    if (!fp) {
        return 0;
    }
    
    pid_t pid = 0;
    if (fscanf(fp, "%d", &pid) != 1) {
        pid = 0;
    }
    pclose(fp);
    
    // 等待candump启动
    usleep(30000);  // 30ms
    
    return pid;
}

// 解析candump输出的一行数据
int parse_can_line(const char* line, uint32_t* id, uint8_t* data, int* dlc) {
    if (!line || !id || !data || !dlc) return -1;
    
    char ifname[16];
    char data_str[64];
    
    // 解析格式: (时间戳) can0 ID#数据
    if (sscanf(line, "(%*f) %s %X#%s", ifname, id, data_str) < 3) {
        return -1;
    }
    
    // 计算数据长度
    int len = strlen(data_str);
    *dlc = (len + 1) / 2;  // 十六进制字符数转字节数
    
    // 转换十六进制数据
    for (int i = 0; i < *dlc && i < 8; i++) {
        if (i*2 + 1 < len) {
            sscanf(data_str + i*2, "%2hhx", &data[i]);
        } else {
            data[i] = 0;
        }
    }
    
    return 0;
}

// 快速读取单个寄存器
int can_fast_read(uint32_t id, uint8_t addr, uint32_t* result) {
    if (!result) return -1;
    
    char* filename = create_listen_file();
    if (!filename) return -1;
    
    pid_t pid = start_candump(id, filename);
    if (pid <= 0) {
        printf("[ERROR] 启动candump失败\n");
        unlink(filename);
        return -1;
    }
    
    // 发送读命令
    if (can_fast_send(id, CMD_READ, addr, 0) != 0) {
        kill(pid, SIGTERM);
        unlink(filename);
        return -1;
    }
    
    // 等待并解析响应
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);
    const long timeout_ns = 300000000;  // 300ms超时
    
    int found_send = 0;    // 是否找到发送的数据
    int found_response = 0; // 是否找到响应
    
    while (!found_response) {
        // 检查超时
        clock_gettime(CLOCK_MONOTONIC, &now);
        long elapsed = (now.tv_sec - start.tv_sec) * 1000000000L + 
                      (now.tv_nsec - start.tv_nsec);
        
        if (elapsed > timeout_ns) {
            printf("[TIMEOUT] 读取地址0x%02X超时\n", addr);
            break;
        }
        
        // 读取文件内容
        FILE* fp = fopen(filename, "r");
        if (!fp) {
            usleep(10000);
            continue;
        }
        
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            uint32_t resp_id;
            uint8_t data[8];
            int dlc;
            
            if (parse_can_line(line, &resp_id, data, &dlc) != 0) {
                continue;
            }
            
            if (resp_id != id) continue;
            
            // 检查是否是我们的发送（CMD_READ）
            if (data[0] == CMD_READ && data[1] == addr) {
                found_send = 1;
                if (g_debug_mode) {
                    printf("[READ] 找到发送回显: ADDR=0x%02X\n", addr);
                }
                continue;
            }
            
            // 检查是否是响应（CMD_READ_ACK）
            if (found_send && data[0] == CMD_READ_ACK && data[1] == addr && dlc >= 6) {
                *result = ((uint32_t)data[2] << 24) |
                         ((uint32_t)data[3] << 16) |
                         ((uint32_t)data[4] << 8) |
                         data[5];
                found_response = 1;
                
                if (g_debug_mode) {
                    printf("[READ] 收到响应: ADDR=0x%02X DATA=0x%08X\n", addr, *result);
                }
                break;
            }
        }
        
        fclose(fp);
        
        if (!found_response) {
            usleep(10000);  // 10ms间隔
        }
    }
    
    // 清理
    kill(pid, SIGTERM);
    usleep(10000);
    unlink(filename);
    
    return found_response ? 0 : -1;
}

// 批量读取多个寄存器
int can_batch_read(uint32_t id, uint8_t* addrs, uint32_t* results, int count) {
    if (!addrs || !results || count <= 0) {
        return -1;
    }
    
    char* filename = create_listen_file();
    if (!filename) return -1;
    
    pid_t pid = start_candump(id, filename);
    if (pid <= 0) {
        unlink(filename);
        return -1;
    }
    
    // 发送所有读请求
    for (int i = 0; i < count; i++) {
        if (can_fast_send(id, CMD_READ, addrs[i], 0) != 0) {
            printf("[ERROR] 发送读请求 %d (0x%02X) 失败\n", i, addrs[i]);
        }
        usleep(1000);  // 5ms间隔
    }
    
    // 等待所有响应
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    const long total_timeout_ns = 100000000;  // 500ms总超时
    
    int* received = (int*)calloc(count, sizeof(int));
    int total_received = 0;
    
    while (total_received < count) {
        // 检查超时
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        long elapsed = (now.tv_sec - start.tv_sec) * 1000000000L + 
                      (now.tv_nsec - start.tv_nsec);
        
        if (elapsed > total_timeout_ns) {
            printf("[TIMEOUT] 批量读取超时，完成 %d/%d\n", total_received, count);
            break;
        }
        
        // 读取响应
        FILE* fp = fopen(filename, "r");
        if (!fp) {
            usleep(10000);
            continue;
        }
        
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            uint32_t resp_id;
            uint8_t data[8];
            int dlc;
            
            if (parse_can_line(line, &resp_id, data, &dlc) != 0) {
                continue;
            }
            
            if (resp_id != id) continue;
            
            // 只处理响应数据（CMD_READ_ACK）
            if (data[0] != CMD_READ_ACK || dlc < 6) {
                continue;
            }
            
            uint8_t resp_addr = data[1];
            
            // 查找对应的寄存器
            for (int i = 0; i < count; i++) {
                if (!received[i] && addrs[i] == resp_addr) {
                    results[i] = ((uint32_t)data[2] << 24) |
                                ((uint32_t)data[3] << 16) |
                                ((uint32_t)data[4] << 8) |
                                data[5];
                    received[i] = 1;
                    total_received++;
                    
                    if (g_debug_mode) {
                        printf("[BATCH] 收到 %d/%d: ADDR=0x%02X DATA=0x%08X\n", 
                               total_received, count, resp_addr, results[i]);
                    }
                    break;
                }
            }
        }
        
        fclose(fp);
        usleep(10000);  // 10ms间隔
    }
    
    // 标记未收到的数据
    for (int i = 0; i < count; i++) {
        if (!received[i]) {
            results[i] = 0xFFFFFFFF;
            printf("[WARN] 地址0x%02X 读取失败\n", addrs[i]);
        }
    }
    
    // 清理
    free(received);
    kill(pid, SIGTERM);
    usleep(10000);
    unlink(filename);
    
    return (total_received == count) ? 0 : -1;
}

// -------------------------- 转换函数 --------------------------

// 角度转脉冲（-180~180度）
int32_t degree_to_pulse(float degree) {
    if (degree < MIN_ANGLE) degree = MIN_ANGLE;
    if (degree > MAX_ANGLE) degree = MAX_ANGLE;
    
    // 特殊处理180度
    if (degree == MAX_ANGLE) {
        return 0x00008000;
    }
    
    // -180~180度映射到-32768~32767
    return (int32_t)(((degree - MIN_ANGLE) / (MAX_ANGLE - MIN_ANGLE)) * PULSES_PER_REVOLUTION) - 0x8000;
}

// 脉冲转角度
float pulse_to_degree(int32_t pulse) {
    return ((float)pulse + 0x8000) * (MAX_ANGLE - MIN_ANGLE) / PULSES_PER_REVOLUTION + MIN_ANGLE;
}

// RPM转脉冲/秒
uint32_t rpm_to_pulse_per_sec(float rpm) {
    if (rpm < 0) rpm = 0;
    return (uint32_t)(rpm * PULSES_PER_REVOLUTION / 60.0f);
}

// 加速度RPM/s转脉冲/秒²
uint32_t accel_rpm_to_pulse(float accel_rpm_s) {
    if (accel_rpm_s < 0) accel_rpm_s = 0;
    return (uint32_t)(accel_rpm_s * PULSES_PER_REVOLUTION / 60.0f);
}

// -------------------------- 设备信息函数 --------------------------
void read_device_info(uint32_t id) {
    printf("\n=== 设备信息 (ID: 0x%X) ===\n", id);
    
    uint32_t sn, hw_ver, fw_ver, can_id;
    
    if (can_fast_read(id, ADDR_DEV_SN, &sn) == 0) {
        printf("序列号: 0x%08X\n", sn);
    }
    
    if (can_fast_read(id, ADDR_HW_VERSION, &hw_ver) == 0) {
        printf("硬件版本: V%d.%d\n", (hw_ver >> 16) & 0xFFFF, hw_ver & 0xFFFF);
    }
    
    if (can_fast_read(id, ADDR_FW_VERSION, &fw_ver) == 0) {
        printf("固件版本: V%d.%d\n", (fw_ver >> 16) & 0xFFFF, fw_ver & 0xFFFF);
    }
    
    if (can_fast_read(id, ADDR_CAN_ID, &can_id) == 0) {
        printf("CAN ID: 0x%X\n", can_id);
    }
}

// -------------------------- 状态读取函数 --------------------------
void read_real_time_status(uint32_t id) {
    printf("\n=== 实时状态 (ID: 0x%X) ===\n", id);
    
    // 批量读取关键状态
    uint8_t addrs[] = {
        ADDR_CURRENT,
        ADDR_SPEED,
        ADDR_POSITION,
        ADDR_BUS_VOLTAGE,
        ADDR_ALARM_CODE
    };
    
    uint32_t results[5];
    char* names[] = {"电流", "速度", "位置", "电压", "告警码"};
    
    if (can_batch_read(id, addrs, results, 5) == 0) {
        for (int i = 0; i < 5; i++) {
            switch (i) {
                case 0:  // 电流
                    printf("%s: %d mA\n", names[i], (int32_t)results[i]);
                    break;
                case 1:  // 速度
                    {
                        int32_t speed_pulse = (int32_t)results[i];
                        float rpm = (float)speed_pulse * 60.0f / PULSES_PER_REVOLUTION;
                        printf("%s: %d pps (%.1f rpm)\n", names[i], speed_pulse, rpm);
                    }
                    break;
                case 2:  // 位置
                    {
                        int32_t pos = (int32_t)results[i];
                        float angle = pulse_to_degree(pos);
                        printf("%s: %d pulse (%.1f°)\n", names[i], pos, angle);
                    }
                    break;
                case 3:  // 电压
                    printf("%s: %.1f V\n", names[i], results[i] * 0.1f);
                    break;
                case 4:  // 告警码
                    if (results[i] == 0) {
                        printf("%s: 正常\n", names[i]);
                    } else {
                        printf("%s: 0x%08X\n", names[i], results[i]);
                    }
                    break;
            }
        }
    }
}

// 快速读取当前位置和角度
int get_current_position(uint32_t id, int32_t* pulse, float* angle) {
    uint32_t pos;
    if (can_fast_read(id, ADDR_POSITION, &pos) != 0) {
        return -1;
    }
    
    int32_t signed_pos = (int32_t)pos;
    if (pulse) *pulse = signed_pos;
    if (angle) *angle = pulse_to_degree(signed_pos);
    
    return 0;
}

// -------------------------- 运动控制函数 --------------------------

// 快速配置位置模式（不等待响应）
int fast_position_mode(uint32_t id, float target_degree, float speed_rpm, 
                      float accel_rpm_s, float decel_rpm_s) {
    // 参数检查
    if (target_degree < MIN_ANGLE || target_degree > MAX_ANGLE) {
        printf("[ERROR] 目标角度 %.1f° 超出范围 (%.0f~%.0f°)\n", 
               target_degree, MIN_ANGLE, MAX_ANGLE);
        return -1;
    }
    
    if (speed_rpm <= 0) {
        printf("[ERROR] 速度必须为正数\n");
        return -1;
    }
    
    // 参数转换
    int32_t target_pulse = degree_to_pulse(target_degree);
    uint32_t speed_pulse = rpm_to_pulse_per_sec(speed_rpm);
    uint32_t accel_pulse = accel_rpm_to_pulse(accel_rpm_s);
    uint32_t decel_pulse = accel_rpm_to_pulse(decel_rpm_s);
    
    printf("[MOVE] ID:0x%X -> %.1f° @ %.1f rpm\n", id, target_degree, speed_rpm);
    
    // 批量写入参数
    uint8_t addrs[] = {
        ADDR_WORK_MODE,
        ADDR_TARGET_SPEED,
        ADDR_ACCEL,
        ADDR_DECEL,
        ADDR_TARGET_POS
    };
    
    uint32_t values[] = {
        MODE_POSITION,
        speed_pulse,
        accel_pulse,
        decel_pulse,
        (uint32_t)target_pulse
    };
    
    return can_batch_write(id, addrs, values, 5);
}

// 快速配置速度模式
int fast_speed_mode(uint32_t id, float speed_rpm, 
                   float accel_rpm_s, float decel_rpm_s) {
    if (speed_rpm == 0) {
        printf("[ERROR] 速度不能为0\n");
        return -1;
    }
    
    int32_t speed_pulse = (int32_t)rpm_to_pulse_per_sec(speed_rpm);
    uint32_t accel_pulse = accel_rpm_to_pulse(accel_rpm_s);
    uint32_t decel_pulse = accel_rpm_to_pulse(decel_rpm_s);
    
    printf("[SPEED] ID:0x%X -> %.1f rpm\n", id, speed_rpm);
    
    uint8_t addrs[] = {
        ADDR_WORK_MODE,
        ADDR_ACCEL,
        ADDR_DECEL,
        ADDR_TARGET_SPEED
    };
    
    uint32_t values[] = {
        MODE_SPEED,
        accel_pulse,
        decel_pulse,
        (uint32_t)speed_pulse
    };
    
    return can_batch_write(id, addrs, values, 4);
}

// 快速配置电流模式（力控）
int fast_current_mode(uint32_t id, int32_t current_ma) {
    printf("[FORCE] ID:0x%X -> %d mA\n", id, current_ma);
    
    uint8_t addrs[] = {
        ADDR_WORK_MODE,
        ADDR_TARGET_CURRENT
    };
    
    uint32_t values[] = {
        MODE_CURRENT,
        (uint32_t)current_ma
    };
    
    return can_batch_write(id, addrs, values, 2);
}

// 力位混合模式
int fast_force_position_mode(uint32_t id, float target_degree, 
                           float speed_rpm, int32_t max_current_ma) {
    if (target_degree < MIN_ANGLE || target_degree > MAX_ANGLE) {
        printf("[ERROR] 角度超出范围\n");
        return -1;
    }
    
    int32_t target_pulse = degree_to_pulse(target_degree);
    uint32_t speed_pulse = rpm_to_pulse_per_sec(speed_rpm);
    
    printf("[FORCE_POS] ID:0x%X -> %.1f° @ %.1f rpm, 限流 %d mA\n", 
           id, target_degree, speed_rpm, max_current_ma);
    
    // 先设置最大电流限制
    uint8_t addrs1[] = {ADDR_MAX_CURRENT};
    uint32_t values1[] = {(uint32_t)max_current_ma};
    can_batch_write(id, addrs1, values1, 1);
    
    // 再配置位置模式
    uint8_t addrs2[] = {
        ADDR_WORK_MODE,
        ADDR_TARGET_SPEED,
        ADDR_TARGET_POS
    };
    
    uint32_t values2[] = {
        MODE_POSITION,
        speed_pulse,
        (uint32_t)target_pulse
    };
    
    return can_batch_write(id, addrs2, values2, 3);
}

// 快速使能/失能电机
int fast_enable_motor(uint32_t id, int enable) {
    printf("[ENABLE] ID:0x%X %s\n", id, enable ? "使能" : "失能");
    return can_fast_send(id, CMD_WRITE, ADDR_ENABLE, enable ? 0x01 : 0x00);
}

// 快速停止电机
int fast_stop_motor(uint32_t id) {
    printf("[STOP] ID:0x%X\n", id);
    return can_fast_send(id, CMD_WRITE, ADDR_STOP_RUN, 0x01);
}

// 设置保护参数
int fast_set_protection(uint32_t id, uint32_t max_current_ma, uint32_t max_speed_pulse) {
    printf("[PROTECT] ID:0x%X 限流=%dmA 限速=%dpps\n", 
           id, max_current_ma, max_speed_pulse);
    
    uint8_t addrs[] = {ADDR_MAX_CURRENT, ADDR_MAX_SPEED};
    uint32_t values[] = {max_current_ma, max_speed_pulse};
    
    return can_batch_write(id, addrs, values, 2);
}

// -------------------------- 运动序列 --------------------------
int execute_motion_sequence(uint32_t id, float* angles, float* speeds, 
                           float* accels, int count, int delay_ms) {
    if (count <= 0 || !angles || !speeds) {
        return -1;
    }
    
    printf("[SEQ] 开始运动序列，%d个点\n", count);
    
    for (int i = 0; i < count; i++) {
        float accel = (accels && i < count) ? accels[i] : 1000.0f;
        
        printf("  点%d: %.1f° @ %.1f rpm\n", i+1, angles[i], speeds[i]);
        
        if (fast_position_mode(id, angles[i], speeds[i], accel, accel) != 0) {
            printf("[ERROR] 第%d点运动失败\n", i+1);
            return -1;
        }
        
        if (delay_ms > 0 && i < count-1) {
            usleep(delay_ms * 1000);
        }
    }
    
    printf("[SEQ] 运动序列完成\n");
    return 0;
}

// -------------------------- 双电机控制 --------------------------
int dual_motor_move(float angle1, float angle2, float speed_rpm) {
    printf("[DUAL] 双电机同步: %.1f°, %.1f° @ %.1f rpm\n", 
           angle1, angle2, speed_rpm);
    
    int ret1 = fast_position_mode(SERVO_ID_1, angle1, speed_rpm, 1000.0f, 1000.0f);
    int ret2 = fast_position_mode(SERVO_ID_2, angle2, speed_rpm, 1000.0f, 1000.0f);
    
    return (ret1 == 0 && ret2 == 0) ? 0 : -1;
}

// -------------------------- 告警处理 --------------------------
int check_alarm(uint32_t id) {
    uint32_t alarm_code;
    
    if (can_fast_read(id, ADDR_ALARM_CODE, &alarm_code) != 0) {
        return -1;
    }
    
    if (alarm_code == 0) {
        return 0;  // 正常
    }
    
    printf("[ALARM] ID:0x%X 告警码: 0x%08X\n", id, alarm_code);
    
    // 尝试清除告警
    can_fast_send(id, CMD_WRITE, ADDR_ALARM_CODE, 0x00);
    
    return 1;  // 有告警
}

// 在 motor.c 文件的末尾添加以下包装函数
// -------------------------- 包装函数（与motor_api.c兼容）--------------------------

// 电机API函数
int motor_can_init(void) {
    printf("[MOTOR] 调用 can_init()\n");
    return can_init();
}

float motor_rpm_to_pulse_per_sec(float rpm) {
    return (float)rpm_to_pulse_per_sec(rpm);
}

int motor_set_protection(uint32_t id, float max_current_ma, float max_speed_pulse) {
    return fast_set_protection(id, (uint32_t)max_current_ma, (uint32_t)max_speed_pulse);
}

int motor_read_device_info(uint32_t id) {
    read_device_info(id);
    return 0;
}

int motor_read_real_time_status(uint32_t id) {
    read_real_time_status(id);
    return 0;
}

int motor_batch_read(uint32_t id, uint8_t* addrs, uint32_t* results, int count) {
    return can_batch_read(id, addrs, results, count);
}

int motor_pulse_to_degree(int32_t pulse) {
    return (int)pulse_to_degree(pulse);
}

int motor_position_mode(uint32_t id, float target_degree, float speed_rpm, 
                       float accel_rpm_s, float decel_rpm_s) {
    return fast_position_mode(id, target_degree, speed_rpm, accel_rpm_s, decel_rpm_s);
}

int motor_speed_mode(uint32_t id, float speed_rpm, float accel_rpm_s, float decel_rpm_s) {
    return fast_speed_mode(id, speed_rpm, accel_rpm_s, decel_rpm_s);
}

int motor_current_mode(uint32_t id, int32_t current_ma) {
    return fast_current_mode(id, current_ma);
}

int motor_enable(uint32_t id, int enable) {
    return fast_enable_motor(id, enable);
}

int motor_stop(uint32_t id) {
    return fast_stop_motor(id);
}

int motor_dual_move(float angle1, float angle2, float speed_rpm) {
    return dual_motor_move(angle1, angle2, speed_rpm);
}
// -------------------------- 主函数 --------------------------
// int main() {
//     if (getuid() != 0) {
//         printf("[ERROR] 需要root权限运行！\n");
//         return -1;
//     }
    
//     printf("=== 优化的电机控制系统 ===\n");
//     printf("角度范围: %.0f~%.0f度\n", MIN_ANGLE, MAX_ANGLE);
//     printf("编码器: %d 脉冲/转\n\n", PULSES_PER_REVOLUTION);
    
//     // 初始化CAN
//     if (can_init() != 0) {
//         printf("[ERROR] CAN初始化失败，退出\n");
//         return -1;
//     }
    
//     // 可选：开启调试模式
//     // set_debug_mode(1);
    
//     // 1. 读取设备信息
//     printf("\n=== 读取设备信息 ===\n");
//     read_device_info(SERVO_ID_1);
//     read_device_info(SERVO_ID_2);
    
//     // 2. 设置保护参数
//     printf("\n=== 设置保护参数 ===\n");
//     fast_set_protection(SERVO_ID_1, 1400, rpm_to_pulse_per_sec(200.0f));
//     fast_set_protection(SERVO_ID_2, 500, rpm_to_pulse_per_sec(150.0f));
    
//     // 3. 使能电机
//     printf("\n=== 使能电机 ===\n");
//     fast_enable_motor(SERVO_ID_1, 1);
//     fast_enable_motor(SERVO_ID_2, 1);
//     sleep(1);
    
//     // 4. 读取初始状态
//     printf("\n=== 初始状态 ===\n");
//     read_real_time_status(SERVO_ID_1);
//     read_real_time_status(SERVO_ID_2);
    
//     // 5. 运动演示
//     printf("\n=== 运动演示 ===\n");
    
//     // 电机1：位置控制
//     printf("电机1 -> 45度 @ 100 rpm\n");
//     fast_position_mode(SERVO_ID_1, 45.0f, 100.0f, 1000.0f, 1000.0f);
//     sleep(2);
    
//     // 读取位置
//     float angle;
//     if (get_current_position(SERVO_ID_1, NULL, &angle) == 0) {
//         printf("当前位置: %.1f°\n", angle);
//     }
    
//     // printf("电机1 -> -30度 @ 150 rpm\n");
//     // fast_position_mode(SERVO_ID_1, -30.0f, 150.0f, 1500.0f, 1500.0f);
//     // sleep(2);
    
//     // // 电机2：速度控制
//     // printf("电机2 -> 连续旋转 @ 50 rpm\n");
//     // fast_speed_mode(SERVO_ID_2, 50.0f, 500.0f, 500.0f);
//     // sleep(3);
    
//     // // 停止旋转
//     // fast_stop_motor(SERVO_ID_2);
    
//     // // 6. 力控演示
//     // printf("\n=== 力控演示 ===\n");
//     // printf("电机1 -> 0度 @ 30 rpm，限流 300 mA\n");
//     // fast_force_position_mode(SERVO_ID_1, 0.0f, 30.0f, 300);
//     // sleep(2);
    
//     // 7. 运动序列演示
//     printf("\n=== 运动序列演示 ===\n");
//     float seq_angles[] = {0.0f, 20.0f, -20.0f, 15.0f, -5.0f};
//     float seq_speeds[] = {50.0f, 80.0f, 80.0f, 60.0f, 40.0f};
    
//     execute_motion_sequence(SERVO_ID_1, seq_angles, seq_speeds, NULL, 5, 200);
//     sleep(2);
    
//     // // 8. 双电机同步演示
//     // printf("\n=== 双电机同步 ===\n");
//     // dual_motor_move(45.0f, -45.0f, 80.0f);
//     // sleep(2);
//     // dual_motor_move(0.0f, 0.0f, 100.0f);
//     // sleep(2);
    
//     // 9. 最终状态
//     printf("\n=== 最终状态 ===\n");
//     read_real_time_status(SERVO_ID_1);
//     read_real_time_status(SERVO_ID_2);
    
//     // 10. 告警检查
//     printf("\n=== 告警检查 ===\n");
//     check_alarm(SERVO_ID_1);
//     check_alarm(SERVO_ID_2);
    
//     // 11. 安全停止
//     printf("\n=== 安全停止 ===\n");
//     fast_stop_motor(SERVO_ID_1);
//     fast_stop_motor(SERVO_ID_2);
//     fast_enable_motor(SERVO_ID_1, 0);
//     fast_enable_motor(SERVO_ID_2, 0);
    
//     printf("\n=== 演示完成 ===\n");
//     return 0;
// }