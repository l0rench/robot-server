// serial_port.c - 在原有文件基础上添加IMU驱动功能
#include "serial_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <termios.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
// ==================== 原serial_port.c内容 ====================
struct MULTI_LONG_CMD_STRUCT data_cmd_long;
uint32_t err_num = 0;

// 全局解析结构体
ParseStruct _parse = {0};
ParseUnion rxdata_union = {0};

// IMU数据缓存
static IMUData imu_cache = {0};
static bool imu_data_ready = false;
static int imu_fd = -1;
static bool imu_initialized = false;

// ==================== CRC32表 ====================
static const uint32_t crc32_tab[] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
    0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
    0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
    0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
    0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
    0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
    0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
    0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

// ==================== CRC32计算函数 ====================
uint32_t crc_crc32(uint32_t crc, const uint8_t *buf, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        crc = crc32_tab[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
    }
    return crc;
}

// ==================== 串口相关函数 ====================
static int is_valid_baud(int baud_rate) {
    int valid_rates[] = {4800, 9600, 19200, 38400, 57600, 115200, 230400, 256000, 460800, 921600};
    int num_rates = sizeof(valid_rates) / sizeof(valid_rates[0]);

    for (int i = 0; i < num_rates; i++) {
        if (baud_rate == valid_rates[i]) {
            return 1;
        }
    }
    return 0;
}

int serial_port_open(const char *portname) {
    char fullPortname[64];

    if (strncmp(portname, "/dev/", 5) == 0) {
        strncpy(fullPortname, portname, sizeof(fullPortname));
    } else {
        snprintf(fullPortname, sizeof(fullPortname), "/dev/%s", portname);
    }

    int fd = open(fullPortname, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        fprintf(stderr, "Error opening %s: %s\n", fullPortname, strerror(errno));
    }
    return fd;
}

int serial_port_configure(int fd, int baud_rate) {
    struct termios options;

    if (fd == -1) {
        perror("open_port: Unable to open SerialPort");
        return -1;
    }

    if (fcntl(fd, F_SETFL, 0) < 0) {
        printf("fcntl failed\n");
        return -1;
    }

    tcgetattr(fd, &options);
    bzero(&options, sizeof(options));

    options.c_cflag = CS8 | CLOCAL | CREAD;

    switch (baud_rate) {
    case 9600:
        options.c_cflag |= B9600;
        break;
    case 115200:
        options.c_cflag |= B115200;
        break;
    case 230400:
        options.c_cflag |= B230400;
        break;
    case 460800:
        options.c_cflag |= B460800;
        break;
    case 921600:
        options.c_cflag |= B921600;
        break;
    default:
        printf("port baud input error!\r\n");
        return -1;
    }

    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    options.c_cc[VTIME] = 0;  // 非阻塞模式
    options.c_cc[VMIN] = 0;
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &options);
    return 0;
}

int serial_send_then_recv(int fd, const char *send_str, const char *expected, char *recv_buf, size_t recv_buf_size, int timeout_ms) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    ssize_t bytes_written = write(fd, send_str, strlen(send_str));
    if (bytes_written < 0) {
        fprintf(stderr, "Error writing to serial port: %s\n", strerror(errno));
        return -1;
    }

    tcflush(fd, TCIFLUSH);

    int total_bytes_read = 0;
    recv_buf[0] = '\0';
    int elapsed_time = 0;

    while (elapsed_time < timeout_ms) {
        ssize_t bytes_read = read(fd, recv_buf + total_bytes_read, recv_buf_size - total_bytes_read - 1);
        if (bytes_read > 0) {
            total_bytes_read += bytes_read;
            if (strlen(expected) > 0 && strstr(recv_buf, expected) != NULL) {
                fcntl(fd, F_SETFL, flags);
                return total_bytes_read;
            }
        }

        usleep(10000); // 10ms
        elapsed_time += 10;
    }

    fcntl(fd, F_SETFL, flags);
    return total_bytes_read;
}

int serial_port_write(int fd, char *buffer, int size) {
    return write(fd, buffer, size);
}

int serial_port_read(int fd, const char *buffer, int size) {
    return write(fd, buffer, size);
}

void serial_port_close(int fd) {
    if (fd >= 0) {
        close(fd);
    }
}

// ==================== IMU驱动函数 ====================

// 初始化IMU
int imu_init(const char* port_name, int baud_rate) {
    printf("[IMU] 初始化IMU传感器: %s, 波特率: %d\n", port_name, baud_rate);
    
    imu_fd = serial_port_open(port_name);
    if (imu_fd < 0) {
        fprintf(stderr, "[IMU] 无法打开串口\n");
        return -1;
    }
    
    if (serial_port_configure(imu_fd, baud_rate) < 0) {
        fprintf(stderr, "[IMU] 配置串口失败\n");
        serial_port_close(imu_fd);
        return -1;
    }
    
    printf("[IMU] IMU串口已成功打开\n");
    
    // 发送初始化命令
    Send_CMD_LONG(3, 1, 0, 0, 0, 0, 0); // 获取AHRS数据流
    for (int i = 0; i < 10; i++) {
        serial_port_write(imu_fd, (char *)&data_cmd_long, sizeof(data_cmd_long));
    }
    
    Send_CMD_LONG(5, 0, 0, 0, 0, 0, 0);
    for (int i = 0; i < 3; i++) {
        serial_port_write(imu_fd, (char *)&data_cmd_long, sizeof(data_cmd_long));
    }
    
    Send_CMD_LONG(3, 1, 0, 0, 0, 0, 0); // 再次获取AHRS数据流
    for (int i = 0; i < 10; i++) {
        serial_port_write(imu_fd, (char *)&data_cmd_long, sizeof(data_cmd_long));
    }
    
    imu_initialized = true;
    printf("[IMU] IMU初始化完成\n");
    return 0;
}

// 读取IMU数据
int imu_read(IMUData* data) {
    if (!imu_initialized || imu_fd < 0 || !data) {
        return -1;
    }
    
    // 读取串口数据
    uint8_t recv_buf[2048];
    int len = read(imu_fd, recv_buf, sizeof(recv_buf));
    
    if (len > 0) {
        if (len >= sizeof(recv_buf)) {
            len = sizeof(recv_buf);
        }
        
        // 解析数据
        for (int i = 0; i < len; i++) {
            imu_rx(recv_buf[i]);
        }
        
        // 如果有新数据，复制到输出参数
        if (imu_data_ready) {
            memcpy(data, &imu_cache, sizeof(IMUData));
            imu_data_ready = false;
            return 1;  // 成功读取新数据
        }
    }
    
    return 0;  // 没有新数据
}

// 关闭IMU
void imu_close(void) {
    if (imu_fd >= 0) {
        serial_port_close(imu_fd);
        imu_fd = -1;
    }
    imu_initialized = false;
    printf("[IMU] IMU已关闭\n");
}

// 检查IMU是否连接
bool imu_is_connected(void) {
    return imu_initialized && (imu_fd >= 0);
}

// ==================== 从原main.c移植的解析函数 ====================

// 欧拉角转换成四元数（暂时不需要，但保留）
static Quaternion eulerToQuaternion(double yaw, double pitch, double roll) {
    double yawRad = yaw * M_PI / 180.0;
    double pitchRad = pitch * M_PI / 180.0;
    double rollRad = roll * M_PI / 180.0;
    
    double cr = cos(rollRad * 0.5);
    double cp = cos(pitchRad * 0.5);
    double cy = cos(yawRad * 0.5);
    double sr = sin(rollRad * 0.5);
    double sp = sin(pitchRad * 0.5);
    double sy = sin(yawRad * 0.5);
    
    Quaternion q;
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;
    
    return q;
}

// 数据提取函数
static void data_extraction(void) {
    if (_parse.id == 0x02) {  // AHRS帧
        // 更新IMU缓存数据
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        
        // 加速度 (g转换为m/s²)
        imu_cache.accel_x = rxdata_union.AHRS_DATA_9axis.imu[0] * 9.80665f;
        imu_cache.accel_y = rxdata_union.AHRS_DATA_9axis.imu[1] * 9.80665f;
        imu_cache.accel_z = rxdata_union.AHRS_DATA_9axis.imu[2] * 9.80665f;
        
        // 角速度 (度/秒转换为弧度/秒)
        imu_cache.gyro_x = rxdata_union.AHRS_DATA_9axis.imu[3] * 0.01745329f;
        imu_cache.gyro_y = rxdata_union.AHRS_DATA_9axis.imu[4] * 0.01745329f;
        imu_cache.gyro_z = rxdata_union.AHRS_DATA_9axis.imu[5] * 0.01745329f;
        
        // 欧拉角 (度)
        imu_cache.roll = rxdata_union.AHRS_DATA_9axis.roll;
        imu_cache.pitch = rxdata_union.AHRS_DATA_9axis.pitch;
        imu_cache.yaw = rxdata_union.AHRS_DATA_9axis.yaw;
        
        // 时间戳 (微秒)
        imu_cache.timestamp = (uint64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
        imu_cache.data_valid = true;
        
        imu_data_ready = true;
        
        // 调试输出
        // printf("[IMU] 新数据: Accel(%.2f,%.2f,%.2f) Gyro(%.2f,%.2f,%.2f) Attitude(%.1f,%.1f,%.1f)\n",
        //        imu_cache.accel_x, imu_cache.accel_y, imu_cache.accel_z,
        //        imu_cache.gyro_x, imu_cache.gyro_y, imu_cache.gyro_z,
        //        imu_cache.roll, imu_cache.pitch, imu_cache.yaw);
    }
}

// 命令发送函数
void Send_CMD_LONG(uint16_t cmd_id, float cm1, float cm2, uint32_t cm3, uint32_t cm4, int32_t cm5, int32_t cm6) {
    uint32_t check_crc = 0;
    
    data_cmd_long.header1 = 0x55;
    data_cmd_long.header2 = 0xaa;
    data_cmd_long.id = cmd_id;
    data_cmd_long.length = sizeof(data_cmd_long) - 10;
    
    data_cmd_long.param1 = cm1;
    data_cmd_long.param2 = cm2;
    data_cmd_long.param3 = cm3;
    data_cmd_long.param4 = cm4;
    data_cmd_long.param5 = cm5;
    data_cmd_long.param6 = cm6;
    
    int len = sizeof(data_cmd_long) - 4;
    check_crc = 1;
    data_cmd_long.check_crc = crc_crc32(check_crc, (uint8_t *)(&data_cmd_long), len);
}

// 数据接收解析函数
void imu_rx(unsigned char data) {
    static uint32_t crcAccum = 1;
    
    switch (_parse.state) {
        case IMU_PARSE_STATE_WAIT_SYNC1:
            if (data == IMU_PARSE_STATE_SYNC1_ID) {
                _parse.state = IMU_PARSE_STATE_WAIT_SYNC2;
                crcAccum = 1;
                crcAccum = crc_crc32(crcAccum, (uint8_t*)&data, 1);
            }
            break;
            
        case IMU_PARSE_STATE_WAIT_SYNC2:
            if (data == IMU_PARSE_STATE_SYNC2_ID) {
                crcAccum = crc_crc32(crcAccum, (uint8_t*)&data, 1);
                _parse.state = IMU_PARSE_STATE_WAIT_ID1;
            } else {
                _parse.state = IMU_PARSE_STATE_WAIT_SYNC1;
            }
            break;
            
        case IMU_PARSE_STATE_WAIT_ID1:
            _parse.id_temp = data;
            crcAccum = crc_crc32(crcAccum, (uint8_t*)&data, 1);
            _parse.state = IMU_PARSE_STATE_WAIT_ID;
            break;
            
        case IMU_PARSE_STATE_WAIT_ID:
            _parse.id = (data << 8) | _parse.id_temp;
            crcAccum = crc_crc32(crcAccum, (uint8_t*)&data, 1);
            _parse.state = IMU_PARSE_STATE_WAIT_LENGTH1;
            break;
            
        case IMU_PARSE_STATE_WAIT_LENGTH1:
            _parse.length_temp = data;
            crcAccum = crc_crc32(crcAccum, (uint8_t*)&data, 1);
            _parse.state = IMU_PARSE_STATE_WAIT_LENGTH2;
            break;
            
        case IMU_PARSE_STATE_WAIT_LENGTH2:
            _parse.length = (data << 8) | _parse.length_temp;
            crcAccum = crc_crc32(crcAccum, (uint8_t*)&data, 1);
            if (_parse.length > 0 && _parse.length < 10000) {
                _parse.count = 0;
                _parse.state = IMU_PARSE_STATE_PAYLOAD;
            } else {
                _parse.state = IMU_PARSE_STATE_WAIT_SYNC1;
            }
            break;
            
        case IMU_PARSE_STATE_PAYLOAD:
            *((char*)(rxdata_union.payload) + _parse.count) = data;
            crcAccum = crc_crc32(crcAccum, (uint8_t*)&data, 1);
            if (++_parse.count == _parse.length) {
                _parse.state = IMU_PARSE_STATE_CHECK1;
            }
            break;
            
        case IMU_PARSE_STATE_CHECK1:
            _parse.check_temp1 = data;
            _parse.state = IMU_PARSE_STATE_CHECK2;
            break;
            
        case IMU_PARSE_STATE_CHECK2:
            _parse.check_temp2 = data;
            _parse.state = IMU_PARSE_STATE_CHECK3;
            break;
            
        case IMU_PARSE_STATE_CHECK3:
            _parse.check_temp3 = data;
            _parse.state = IMU_PARSE_STATE_CHECK4;
            break;
            
        case IMU_PARSE_STATE_CHECK4:
            _parse.check = (data << 24) | (_parse.check_temp3 << 16) | (_parse.check_temp2 << 8) | _parse.check_temp1;
            if (_parse.check == crcAccum) {
                data_extraction();
            } else {
                err_num++;
            }
            _parse.state = IMU_PARSE_STATE_WAIT_SYNC1;
            break;
            
        default:
            _parse.state = IMU_PARSE_STATE_WAIT_SYNC1;
            break;
    }
}