// thread_modules/motor_control_thread.c
#include "shared_data.h"
#include "motor_api.h"
#include "ipc_msg_queue.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

// 电机控制线程
void* motor_control_thread(void* arg) {
    GlobalData* g_data = (GlobalData*)arg;
    
    // printf("[CTRL_THREAD] 电机控制线程启动\n");
    
    // 等待电机初始化
    int wait_count = 0;
    while (!g_data->motors_initialized && wait_count < 10 && g_data->system_running) {
        // printf("[CTRL_THREAD] 等待电机初始化... (%d/10)\n", wait_count + 1);
        sleep(1);
        wait_count++;
    }
    
    if (!g_data->motors_initialized) {
        // printf("[CTRL_THREAD] 电机未初始化，线程退出\n");
        return NULL;
    }
    
    // 使用 MotorCmdMsg 而不是 MsgUnion
    MotorCmdMsg cmd;
    while (g_data->system_running) {
        // 从消息队列接收控制命令（非阻塞）
        if (msg_queue_receive_cmd(g_data->msg_queues.cmd_mq, &cmd, false) > 0) {
            // printf("[CTRL_THREAD] 收到控制命令\n");
            
            // 执行电机控制
            // 控制电机1
            if (cmd.enable_1) {
                motor_enable_device(SERVO_ID_1, true);
                motor_enable_device(SERVO_ID_3, true);
                if (cmd.stop_1) {
                    motor_stop_device(SERVO_ID_1);
                    motor_stop_device(SERVO_ID_3);
                    // printf("[CTRL_THREAD] 停止电机1\n");
                } else if (cmd.target_angle_1 != 0 || cmd.target_speed_1 != 0) {
                    // printf("[CTRL_THREAD] 移动电机1到 %.1f° @ %.1f RPM\n",
                    //        cmd.target_angle_1, cmd.target_speed_1);
                    
                    motor_move_to_angle(SERVO_ID_1,cmd.target_angle_1,cmd.target_speed_1);
                    motor_move_to_angle(SERVO_ID_3,-(cmd.target_angle_1),cmd.target_speed_1);
                }
            } else {
                motor_enable_device(SERVO_ID_1, false);
                motor_enable_device(SERVO_ID_3, false);
                printf("[CTRL_THREAD] 失能俯仰电机\n");
            }
            
            // 控制电机2
            if (cmd.enable_2) {
                motor_enable_device(SERVO_ID_2, true);
                if (cmd.stop_2) {
                    motor_stop_device(SERVO_ID_2);
                    // printf("[CTRL_THREAD] 停止电机2\n");
                } else if (cmd.target_angle_2 != 0 || cmd.target_speed_2 != 0) {
                    // printf("[CTRL_THREAD] 移动电机2到 %.1f° @ %.1f RPM\n",
                    //        cmd.target_angle_2, cmd.target_speed_2);
                    
                    motor_move_to_angle(SERVO_ID_2, 
                                       cmd.target_angle_2,
                                       cmd.target_speed_2);
                }
            } else {
                motor_enable_device(SERVO_ID_2, false);
                printf("[CTRL_THREAD] 失能电机2\n");
            }
        }
        
        // 也检查TCP线程设置的控制命令
        pthread_mutex_lock(&g_data->cmd_mutex);
        MotorCmdMsg tcp_cmd = g_data->last_motor_cmd;
        pthread_mutex_unlock(&g_data->cmd_mutex);
        
        // 如果TCP有命令，也执行
        static MotorCmdMsg last_tcp_cmd = {0};
        if (memcmp(&tcp_cmd, &last_tcp_cmd, sizeof(MotorCmdMsg)) != 0 && 
            tcp_cmd.msg_type == MSG_TYPE_MOTOR_CMD) {
            
            // printf("[CTRL_THREAD] 执行TCP控制命令\n");
            
            // 直接执行TCP命令
            if (tcp_cmd.enable_1) {
                if (tcp_cmd.stop_1) {
                    motor_stop_device(SERVO_ID_1);
                    motor_stop_device(SERVO_ID_3);
                    // printf("[CTRL_THREAD] TCP: 停止电机1\n");
                } else if (tcp_cmd.target_angle_1 != 0 || tcp_cmd.target_speed_1 != 0) {
                    // printf("[CTRL_THREAD] TCP: 移动电机1到 %.1f° @ %.1f RPM\n",
                    //        tcp_cmd.target_angle_1, tcp_cmd.target_speed_1);
                    
                    motor_move_to_angle(SERVO_ID_1, tcp_cmd.target_angle_1,tcp_cmd.target_speed_1);
                    motor_move_to_angle(SERVO_ID_3, -(tcp_cmd.target_angle_1),tcp_cmd.target_speed_1);
                }
            }
            
            if (tcp_cmd.enable_2) {
                if (tcp_cmd.stop_2) {
                    motor_stop_device(SERVO_ID_2);
                    // printf("[CTRL_THREAD] TCP: 停止电机2\n");
                } else if (tcp_cmd.target_angle_2 != 0 || tcp_cmd.target_speed_2 != 0) {
                    // printf("[CTRL_THREAD] TCP: 移动电机2到 %.1f° @ %.1f RPM\n",
                    //        tcp_cmd.target_angle_2, tcp_cmd.target_speed_2);
                    
                    motor_move_to_angle(SERVO_ID_2, 
                                       tcp_cmd.target_angle_2,
                                       tcp_cmd.target_speed_2);
                }
            }
            
            last_tcp_cmd = tcp_cmd;
        }
        
        // 控制频率：10Hz
        usleep(100000);
    }
    
    printf("[CTRL_THREAD] 电机控制线程退出\n");
    
    return NULL;
}