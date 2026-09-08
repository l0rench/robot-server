// // ipc_msg_queue.c
// #include "ipc_msg_queue.h"
// #include <stdio.h>
// #include <string.h>
// #include <errno.h>
// #include <fcntl.h>
// #include <sys/stat.h>
// #include <unistd.h>
// #include <time.h>

// // 初始化POSIX消息队列
// int msg_queue_init(MessageQueues* mqs) {
//     struct mq_attr attr = {
//         .mq_flags = 0,
//         .mq_maxmsg = MAX_MSGS,
//         .mq_msgsize = MAX_MSG_SIZE,
//         .mq_curmsgs = 0
//     };
    
//     // 删除可能存在的旧队列
//     mq_unlink(MQ_ENV_NAME);
//     mq_unlink(MQ_MOTOR_NAME);
//     mq_unlink(MQ_IMU_NAME);
//     mq_unlink(MQ_CMD_NAME);
    
//     // 创建环境数据消息队列
//     mqs->env_mq = mq_open(MQ_ENV_NAME, O_CREAT | O_RDWR, 0666, &attr);
//     if (mqs->env_mq == (mqd_t)-1) {
//         perror("[MSG] 创建环境数据消息队列失败");
//         return -1;
//     }
    
//     // 创建电机数据消息队列
//     mqs->motor_mq = mq_open(MQ_MOTOR_NAME, O_CREAT | O_RDWR, 0666, &attr);
//     if (mqs->motor_mq == (mqd_t)-1) {
//         perror("[MSG] 创建电机数据消息队列失败");
//         mq_close(mqs->env_mq);
//         return -1;
//     }
    
//     // 创建IMU数据消息队列
//     mqs->imu_mq = mq_open(MQ_IMU_NAME, O_CREAT | O_RDWR, 0666, &attr);
//     if (mqs->imu_mq == (mqd_t)-1) {
//         perror("[MSG] 创建IMU数据消息队列失败");
//         mq_close(mqs->env_mq);
//         mq_close(mqs->motor_mq);
//         return -1;
//     }
    
//     // 创建控制命令消息队列
//     mqs->cmd_mq = mq_open(MQ_CMD_NAME, O_CREAT | O_RDWR, 0666, &attr);
//     if (mqs->cmd_mq == (mqd_t)-1) {
//         perror("[MSG] 创建控制命令消息队列失败");
//         mq_close(mqs->env_mq);
//         mq_close(mqs->motor_mq);
//         mq_close(mqs->imu_mq);
//         return -1;
//     }
    
//     printf("[MSG] POSIX消息队列初始化成功\n");
//     return 0;
// }

// // // 发送环境数据
// // int msg_queue_send_env(mqd_t mq, const EnvMsg* msg) {
// //     if (!msg) return -1;
// //     return mq_send(mq, (const char*)msg, sizeof(EnvMsg), 0);
// // }

// // // 发送电机数据
// // int msg_queue_send_motor(mqd_t mq, const MotorMsg* msg) {
// //     if (!msg) return -1;
// //     return mq_send(mq, (const char*)msg, sizeof(MotorMsg), 0);
// // }

// // // 发送IMU数据
// // int msg_queue_send_imu(mqd_t mq, const ImuMsg* msg) {
// //     if (!msg) return -1;
// //     return mq_send(mq, (const char*)msg, sizeof(ImuMsg), 0);
// // }

// // // 发送控制命令
// // int msg_queue_send_cmd(mqd_t mq, const MotorCmdMsg* msg) {
// //     if (!msg) return -1;
// //     return mq_send(mq, (const char*)msg, sizeof(MotorCmdMsg), 0);
// // }

// // 发送环境数据
// int msg_queue_send_env(mqd_t mq, const EnvMsg* msg) {
//     if (!msg) return -1;
//     char buffer[MAX_MSG_SIZE] = {0}; // 清零
//     memcpy(buffer, msg, sizeof(EnvMsg));
//     return mq_send(mq, buffer, MAX_MSG_SIZE, 0);
// }

// // 发送电机数据
// int msg_queue_send_motor(mqd_t mq, const MotorMsg* msg) {
//     if (!msg) return -1;
//     char buffer[MAX_MSG_SIZE] = {0};
//     memcpy(buffer, msg, sizeof(MotorMsg));
//     return mq_send(mq, buffer, MAX_MSG_SIZE, 0);
// }

// // 发送IMU数据
// int msg_queue_send_imu(mqd_t mq, const ImuMsg* msg) {
//     if (!msg) return -1;
//     char buffer[MAX_MSG_SIZE] = {0};
//     memcpy(buffer, msg, sizeof(ImuMsg));
//     return mq_send(mq, buffer, MAX_MSG_SIZE, 0);
// }

// // 发送控制命令
// int msg_queue_send_cmd(mqd_t mq, const MotorCmdMsg* msg) {
//     if (!msg) return -1;
//     char buffer[MAX_MSG_SIZE] = {0};
//     memcpy(buffer, msg, sizeof(MotorCmdMsg));
//     return mq_send(mq, buffer, MAX_MSG_SIZE, 0);
// }

// // 通用接收函数
// // ipc_msg_queue.c - 修复通用接收函数
// // 通用接收函数
// // int msg_queue_receive(mqd_t mq, void* buf, size_t buf_size, unsigned int* prio, bool block) {
// //     if (!buf) return -1;
    
// //     unsigned int msg_prio = 0;
// //     ssize_t ret;
    
// //     if (block) {
// //         ret = mq_receive(mq, (char*)buf, buf_size, &msg_prio);
// //     } else {
// //         ret = mq_receive(mq, (char*)buf, buf_size, &msg_prio);
// //         if (ret == -1 && errno == EAGAIN) {
// //             return 0; // 没有消息
// //         }
// //     }
    
// //     if (prio && ret > 0) {
// //         *prio = msg_prio;
// //     }
    
// //     return (int)ret;
// // }



// // 通用接收函数（带超时实现非阻塞）
// int msg_queue_receive(mqd_t mq, void* buf, size_t buf_size, unsigned int* prio, bool block) {
//     if (!buf || buf_size < MAX_MSG_SIZE) {
//         errno = EINVAL;
//         return -1;
//     }

//     unsigned int msg_prio = 0;
//     ssize_t ret;

//     if (block) {
//         ret = mq_receive(mq, (char*)buf, buf_size, &msg_prio);
//     } else {
//         // 使用 timedreceive 实现非阻塞
//         struct timespec timeout = {0, 0}; // 立即超时
//         ret = mq_timedreceive(mq, (char*)buf, buf_size, &msg_prio, &timeout);
//         if (ret == -1 && (errno == ETIMEDOUT || errno == EAGAIN)) {
//             return 0; // 无消息
//         }
//     }

//     if (ret == -1) {
//         return -1; // 真正的错误
//     }

//     if (prio) {
//         *prio = msg_prio;
//     }

//     return (int)ret;
// }

// // // 接收环境数据
// // int msg_queue_receive_env(mqd_t mq, EnvMsg* msg, bool block) {
// //     if (!msg) return -1;
    
// //     unsigned int prio;
// //     int ret = msg_queue_receive(mq, msg, sizeof(EnvMsg), &prio, block);
// //     if (ret > 0) {
// //         // 验证消息类型
// //         if (msg->msg_type != MSG_TYPE_ENV_DATA) {
// //             printf("[MSG] 警告：接收到非环境数据类型: %d\n", msg->msg_type);
// //             return -1;
// //         }
// //     }
// //     return ret;
// // }

// // // 接收电机数据
// // int msg_queue_receive_motor(mqd_t mq, MotorMsg* msg, bool block) {
// //     if (!msg) return -1;
    
// //     unsigned int prio;
// //     int ret = msg_queue_receive(mq, msg, sizeof(MotorMsg), &prio, block);
// //     if (ret > 0) {
// //         if (msg->msg_type != MSG_TYPE_MOTOR_DATA) {
// //             printf("[MSG] 警告：接收到非电机数据类型: %d\n", msg->msg_type);
// //             return -1;
// //         }
// //     }
// //     return ret;
// // }

// // // 接收IMU数据
// // int msg_queue_receive_imu(mqd_t mq, ImuMsg* msg, bool block) {
// //     if (!msg) return -1;
    
// //     unsigned int prio;
// //     int ret = msg_queue_receive(mq, msg, sizeof(ImuMsg), &prio, block);
// //     if (ret > 0) {
// //         if (msg->msg_type != MSG_TYPE_IMU_DATA) {
// //             printf("[MSG] 警告：接收到非IMU数据类型: %d\n", msg->msg_type);
// //             return -1;
// //         }
// //     }
// //     return ret;
// // }

// // // 接收控制命令
// // int msg_queue_receive_cmd(mqd_t mq, MotorCmdMsg* msg, bool block) {
// //     if (!msg) return -1;
    
// //     unsigned int prio;
// //     int ret = msg_queue_receive(mq, msg, sizeof(MotorCmdMsg), &prio, block);
// //     if (ret > 0) {
// //         if (msg->msg_type != MSG_TYPE_MOTOR_CMD) {
// //             printf("[MSG] 警告：接收到非控制命令类型: %d\n", msg->msg_type);
// //             return -1;
// //         }
// //     }
// //     return ret;
// // }

// // 接收环境数据
// int msg_queue_receive_env(mqd_t mq, EnvMsg* msg, bool block) {
//     if (!msg) return -1;
    
//     char buffer[MAX_MSG_SIZE];
//     unsigned int prio;
//     int ret = msg_queue_receive(mq, buffer, sizeof(buffer), &prio, block);
    
//     if (ret > 0) {
//         // 验证长度（应为 MAX_MSG_SIZE）
//         if (ret != MAX_MSG_SIZE) {
//             fprintf(stderr, "[MSG] 警告：收到的消息长度 %d != %d\n", ret, MAX_MSG_SIZE);
//             return -1;
//         }
        
//         EnvMsg* received = (EnvMsg*)buffer;
//         if (received->msg_type != MSG_TYPE_ENV_DATA) {
//             printf("[MSG] 警告：接收到非环境数据类型: %d\n", received->msg_type);
//             return -1;
//         }
//         *msg = *received; // 拷贝有效结构
//     }
//     return ret;
// }

// // 接收电机数据
// int msg_queue_receive_motor(mqd_t mq, MotorMsg* msg, bool block) {
//     if (!msg) return -1;
    
//     char buffer[MAX_MSG_SIZE];
//     unsigned int prio;
//     int ret = msg_queue_receive(mq, buffer, sizeof(buffer), &prio, block);
    
//     if (ret > 0) {
//         if (ret != MAX_MSG_SIZE) {
//             fprintf(stderr, "[MSG] 警告：收到的消息长度 %d != %d\n", ret, MAX_MSG_SIZE);
//             return -1;
//         }
        
//         MotorMsg* received = (MotorMsg*)buffer;
//         if (received->msg_type != MSG_TYPE_MOTOR_DATA) {
//             printf("[MSG] 警告：接收到非电机数据类型: %d\n", received->msg_type);
//             return -1;
//         }
//         *msg = *received;
//     }
//     return ret;
// }

// // 接收IMU数据
// int msg_queue_receive_imu(mqd_t mq, ImuMsg* msg, bool block) {
//     if (!msg) return -1;
    
//     char buffer[MAX_MSG_SIZE];
//     unsigned int prio;
//     int ret = msg_queue_receive(mq, buffer, sizeof(buffer), &prio, block);
    
//     if (ret > 0) {
//         if (ret != MAX_MSG_SIZE) {
//             fprintf(stderr, "[MSG] 警告：收到的消息长度 %d != %d\n", ret, MAX_MSG_SIZE);
//             return -1;
//         }
        
//         ImuMsg* received = (ImuMsg*)buffer;
//         if (received->msg_type != MSG_TYPE_IMU_DATA) {
//             printf("[MSG] 警告：接收到非IMU数据类型: %d\n", received->msg_type);
//             return -1;
//         }
//         *msg = *received;
//     }
//     return ret;
// }

// // 接收控制命令
// int msg_queue_receive_cmd(mqd_t mq, MotorCmdMsg* msg, bool block) {
//     if (!msg) return -1;
    
//     char buffer[MAX_MSG_SIZE];
//     unsigned int prio;
//     int ret = msg_queue_receive(mq, buffer, sizeof(buffer), &prio, block);
    
//     if (ret > 0) {
//         if (ret != MAX_MSG_SIZE) {
//             fprintf(stderr, "[MSG] 警告：收到的消息长度 %d != %d\n", ret, MAX_MSG_SIZE);
//             return -1;
//         }
        
//         MotorCmdMsg* received = (MotorCmdMsg*)buffer;
//         if (received->msg_type != MSG_TYPE_MOTOR_CMD) {
//             printf("[MSG] 警告：接收到非控制命令类型: %d\n", received->msg_type);
//             return -1;
//         }
//         *msg = *received;
//     }
//     return ret;
// }

// // 清理消息队列
// void msg_queue_cleanup(MessageQueues* mqs) {
//     if (mqs->env_mq != (mqd_t)-1) {
//         mq_close(mqs->env_mq);
//         mq_unlink(MQ_ENV_NAME);
//         printf("[MSG] 清理环境数据队列\n");
//     }
    
//     if (mqs->motor_mq != (mqd_t)-1) {
//         mq_close(mqs->motor_mq);
//         mq_unlink(MQ_MOTOR_NAME);
//         printf("[MSG] 清理电机数据队列\n");
//     }
    
//     if (mqs->imu_mq != (mqd_t)-1) {
//         mq_close(mqs->imu_mq);
//         mq_unlink(MQ_IMU_NAME);
//         printf("[MSG] 清理IMU数据队列\n");
//     }
    
//     if (mqs->cmd_mq != (mqd_t)-1) {
//         mq_close(mqs->cmd_mq);
//         mq_unlink(MQ_CMD_NAME);
//         printf("[MSG] 清理控制命令队列\n");
//     }
// }

// // 获取消息队列中的消息数量
// int msg_queue_get_count(mqd_t mq) {
//     struct mq_attr attr;
//     if (mq_getattr(mq, &attr) == -1) {
//         return -1;
//     }
//     return (int)attr.mq_curmsgs;
// }


// ipc_msg_queue.c
#include "ipc_msg_queue.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

// 初始化POSIX消息队列
int msg_queue_init(MessageQueues* mqs) {
    struct mq_attr attr = {
        .mq_flags = 0,
        .mq_maxmsg = MAX_MSGS,
        .mq_msgsize = MAX_MSG_SIZE,
        .mq_curmsgs = 0
    };
    
    // 删除可能存在的旧队列
    mq_unlink(MQ_ENV_NAME);
    mq_unlink(MQ_MOTOR_NAME);
    mq_unlink(MQ_IMU_NAME);
    mq_unlink(MQ_CMD_NAME);
    mq_unlink(MQ_LOG_NAME);  // <-- 新增：删除旧的日志队列

    // 创建环境数据消息队列
    mqs->env_mq = mq_open(MQ_ENV_NAME, O_CREAT | O_RDWR, 0666, &attr);
    if (mqs->env_mq == (mqd_t)-1) {
        perror("[MSG] 创建环境数据消息队列失败");
        return -1;
    }
    
    // 创建电机数据消息队列
    mqs->motor_mq = mq_open(MQ_MOTOR_NAME, O_CREAT | O_RDWR, 0666, &attr);
    if (mqs->motor_mq == (mqd_t)-1) {
        perror("[MSG] 创建电机数据消息队列失败");
        mq_close(mqs->env_mq);
        return -1;
    }
    
    // 创建IMU数据消息队列
    mqs->imu_mq = mq_open(MQ_IMU_NAME, O_CREAT | O_RDWR, 0666, &attr);
    if (mqs->imu_mq == (mqd_t)-1) {
        perror("[MSG] 创建IMU数据消息队列失败");
        mq_close(mqs->env_mq);
        mq_close(mqs->motor_mq);
        return -1;
    }
    
    // 创建控制命令消息队列
    mqs->cmd_mq = mq_open(MQ_CMD_NAME, O_CREAT | O_RDWR, 0666, &attr);
    if (mqs->cmd_mq == (mqd_t)-1) {
        perror("[MSG] 创建控制命令消息队列失败");
        mq_close(mqs->env_mq);
        mq_close(mqs->motor_mq);
        mq_close(mqs->imu_mq);
        return -1;
    }

    // 创建日志消息队列（新增）
    mqs->log_mq = mq_open(MQ_LOG_NAME, O_CREAT | O_RDWR, 0666, &attr);
    if (mqs->log_mq == (mqd_t)-1) {
        perror("[MSG] 创建日志消息队列失败");
        mq_close(mqs->env_mq);
        mq_close(mqs->motor_mq);
        mq_close(mqs->imu_mq);
        mq_close(mqs->cmd_mq);
        return -1;
    }
    
    printf("[MSG] POSIX消息队列初始化成功\n");
    return 0;
}

// 发送环境数据
int msg_queue_send_env(mqd_t mq, const EnvMsg* msg) {
    if (!msg) return -1;
    char buffer[MAX_MSG_SIZE] = {0};
    memcpy(buffer, msg, sizeof(EnvMsg));
    return mq_send(mq, buffer, MAX_MSG_SIZE, 0);
}

// 发送电机数据
int msg_queue_send_motor(mqd_t mq, const MotorMsg* msg) {
    if (!msg) return -1;
    char buffer[MAX_MSG_SIZE] = {0};
    memcpy(buffer, msg, sizeof(MotorMsg));
    return mq_send(mq, buffer, MAX_MSG_SIZE, 0);
}

// 发送IMU数据
int msg_queue_send_imu(mqd_t mq, const ImuMsg* msg) {
    if (!msg) return -1;
    char buffer[MAX_MSG_SIZE] = {0};
    memcpy(buffer, msg, sizeof(ImuMsg));
    return mq_send(mq, buffer, MAX_MSG_SIZE, 0);
}

// 发送控制命令
int msg_queue_send_cmd(mqd_t mq, const MotorCmdMsg* msg) {
    if (!msg) return -1;
    char buffer[MAX_MSG_SIZE] = {0};
    memcpy(buffer, msg, sizeof(MotorCmdMsg));
    return mq_send(mq, buffer, MAX_MSG_SIZE, 0);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>> 新增：发送日志 <<<<<<<<<<<<<<<<<<<<<<<<<<
int msg_queue_send_log(mqd_t mq, const LogMsg* msg) {
    if (!msg) return -1;
    char buffer[MAX_MSG_SIZE] = {0};
    memcpy(buffer, msg, sizeof(LogMsg));
    return mq_send(mq, buffer, MAX_MSG_SIZE, 0);
}

// 通用接收函数（带超时实现非阻塞）
int msg_queue_receive(mqd_t mq, void* buf, size_t buf_size, unsigned int* prio, bool block) {
    if (!buf || buf_size < MAX_MSG_SIZE) {
        errno = EINVAL;
        return -1;
    }

    unsigned int msg_prio = 0;
    ssize_t ret;

    if (block) {
        ret = mq_receive(mq, (char*)buf, buf_size, &msg_prio);
    } else {
        struct timespec timeout = {0, 0}; // 立即超时
        ret = mq_timedreceive(mq, (char*)buf, buf_size, &msg_prio, &timeout);
        if (ret == -1 && (errno == ETIMEDOUT || errno == EAGAIN)) {
            return 0; // 无消息
        }
    }

    if (ret == -1) {
        return -1;
    }

    if (prio) {
        *prio = msg_prio;
    }

    return (int)ret;
}

// 接收环境数据
int msg_queue_receive_env(mqd_t mq, EnvMsg* msg, bool block) {
    if (!msg) return -1;
    char buffer[MAX_MSG_SIZE];
    unsigned int prio;
    int ret = msg_queue_receive(mq, buffer, sizeof(buffer), &prio, block);
    if (ret > 0) {
        if (ret != MAX_MSG_SIZE) {
            fprintf(stderr, "[MSG] 警告：收到的消息长度 %d != %d\n", ret, MAX_MSG_SIZE);
            return -1;
        }
        EnvMsg* received = (EnvMsg*)buffer;
        if (received->msg_type != MSG_TYPE_ENV_DATA) {
            printf("[MSG] 警告：接收到非环境数据类型: %d\n", received->msg_type);
            return -1;
        }
        *msg = *received;
    }
    return ret;
}

// 接收电机数据
int msg_queue_receive_motor(mqd_t mq, MotorMsg* msg, bool block) {
    if (!msg) return -1;
    char buffer[MAX_MSG_SIZE];
    unsigned int prio;
    int ret = msg_queue_receive(mq, buffer, sizeof(buffer), &prio, block);
    if (ret > 0) {
        if (ret != MAX_MSG_SIZE) {
            fprintf(stderr, "[MSG] 警告：收到的消息长度 %d != %d\n", ret, MAX_MSG_SIZE);
            return -1;
        }
        MotorMsg* received = (MotorMsg*)buffer;
        if (received->msg_type != MSG_TYPE_MOTOR_DATA) {
            printf("[MSG] 警告：接收到非电机数据类型: %d\n", received->msg_type);
            return -1;
        }
        *msg = *received;
    }
    return ret;
}

// 接收IMU数据
int msg_queue_receive_imu(mqd_t mq, ImuMsg* msg, bool block) {
    if (!msg) return -1;
    char buffer[MAX_MSG_SIZE];
    unsigned int prio;
    int ret = msg_queue_receive(mq, buffer, sizeof(buffer), &prio, block);
    if (ret > 0) {
        if (ret != MAX_MSG_SIZE) {
            fprintf(stderr, "[MSG] 警告：收到的消息长度 %d != %d\n", ret, MAX_MSG_SIZE);
            return -1;
        }
        ImuMsg* received = (ImuMsg*)buffer;
        if (received->msg_type != MSG_TYPE_IMU_DATA) {
            printf("[MSG] 警告：接收到非IMU数据类型: %d\n", received->msg_type);
            return -1;
        }
        *msg = *received;
    }
    return ret;
}

// 接收控制命令
int msg_queue_receive_cmd(mqd_t mq, MotorCmdMsg* msg, bool block) {
    if (!msg) return -1;
    char buffer[MAX_MSG_SIZE];
    unsigned int prio;
    int ret = msg_queue_receive(mq, buffer, sizeof(buffer), &prio, block);
    if (ret > 0) {
        if (ret != MAX_MSG_SIZE) {
            fprintf(stderr, "[MSG] 警告：收到的消息长度 %d != %d\n", ret, MAX_MSG_SIZE);
            return -1;
        }
        MotorCmdMsg* received = (MotorCmdMsg*)buffer;
        if (received->msg_type != MSG_TYPE_MOTOR_CMD) {
            printf("[MSG] 警告：接收到非控制命令类型: %d\n", received->msg_type);
            return -1;
        }
        *msg = *received;
    }
    return ret;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>> 新增：接收日志 <<<<<<<<<<<<<<<<<<<<<<<<<<
int msg_queue_receive_log(mqd_t mq, LogMsg* msg, bool block) {
    if (!msg) return -1;
    char buffer[MAX_MSG_SIZE];
    unsigned int prio;
    int ret = msg_queue_receive(mq, buffer, sizeof(buffer), &prio, block);
    if (ret > 0) {
        if (ret != MAX_MSG_SIZE) {
            fprintf(stderr, "[MSG] 警告：收到的日志消息长度 %d != %d\n", ret, MAX_MSG_SIZE);
            return -1;
        }
        LogMsg* received = (LogMsg*)buffer;
        if (received->msg_type != MSG_TYPE_LOG_MSG) {
            printf("[MSG] 警告：接收到非日志消息类型: %d\n", received->msg_type);
            return -1;
        }
        *msg = *received;
    }
    return ret;
}

// 清理消息队列
void msg_queue_cleanup(MessageQueues* mqs) {
    if (mqs->env_mq != (mqd_t)-1) {
        mq_close(mqs->env_mq);
        mq_unlink(MQ_ENV_NAME);
        printf("[MSG] 清理环境数据队列\n");
    }
    
    if (mqs->motor_mq != (mqd_t)-1) {
        mq_close(mqs->motor_mq);
        mq_unlink(MQ_MOTOR_NAME);
        printf("[MSG] 清理电机数据队列\n");
    }
    
    if (mqs->imu_mq != (mqd_t)-1) {
        mq_close(mqs->imu_mq);
        mq_unlink(MQ_IMU_NAME);
        printf("[MSG] 清理IMU数据队列\n");
    }
    
    if (mqs->cmd_mq != (mqd_t)-1) {
        mq_close(mqs->cmd_mq);
        mq_unlink(MQ_CMD_NAME);
        printf("[MSG] 清理控制命令队列\n");
    }

    // 新增：清理日志队列
    if (mqs->log_mq != (mqd_t)-1) {
        mq_close(mqs->log_mq);
        mq_unlink(MQ_LOG_NAME);
        printf("[MSG] 清理日志消息队列\n");
    }
}

// 获取消息队列中的消息数量
int msg_queue_get_count(mqd_t mq) {
    struct mq_attr attr;
    if (mq_getattr(mq, &attr) == -1) {
        return -1;
    }
    return (int)attr.mq_curmsgs;
}