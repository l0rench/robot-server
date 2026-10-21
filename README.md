
# Robot Server

机器人多传感器数据汇聚与控制系统。本项目采用模块化架构，包含雷达、IMU、视频处理、主控服务器及时间同步五个独立子模块，分别负责不同传感器数据的采集、转发与机器人整体控制。

---

## 项目结构

```
robot/
├── README.md                   # 项目总说明文档（本文件）
├── .env                        # 环境变量配置
├── .gitignore
├── .vscode/                    # VS Code 工作区配置
│   └── settings.json
├── radar/                      # Livox 激光雷达点云转发服务
│   ├── CMakeLists.txt          # CMake 构建配置
│   ├── Makefile                # Make 构建配置
│   ├── src/
│   │   ├── main.cpp            # 雷达数据读取与 UDP 转发主程序
│   │   └── synchro.cpp         # 雷达时间戳同步逻辑
│   ├── include/
│   │   ├── livox_lidar_def.h   # Livox 雷达定义头文件
│   │   ├── livox_lidar_cfg.h   # Livox 雷达配置头文件
│   │   ├── livox_lidar_api.h   # Livox SDK API 头文件
│   │   └── synchro.h           # 同步模块头文件
│   ├── config/
│   │   ├── config.json         # 通用配置文件
│   │   ├── hap_config.json     # HAP 配置文件
│   │   └── mid360_config.json  # Mid360 雷达配置文件
│   ├── lib/
│   │   └── liblivox_lidar_sdk_static.a  # Livox SDK 静态库
│   └── build/                  # 编译输出目录
│       ├── radar               # 可执行文件
│       ├── main.o
│       ├── synchro.o
│       └── pointcloud_with_latency.ply
├── vedio_processing/           # Orbbec 深度相机视频流处理服务
│   ├── Makefile                # Make 构建配置
│   ├── src/
│   │   ├── 4vedio_processiong.cpp    # 视频流处理与时间戳同步主程序
│   │   └── 4vedio_processiong.cpp.bak # 备份文件
│   ├── include/
│   │   ├── ObSensor.h          # Orbbec SDK C 接口
│   │   ├── ObSensor.hpp        # Orbbec SDK C++ 接口
│   │   └── libobsensor/        # Orbbec SDK 内部头文件
│   ├── lib/
│   │   ├── libOrbbecSDK.so     # Orbbec SDK 动态库（软链）
│   │   ├── libOrbbecSDK.so.2   # Orbbec SDK 动态库（主版本）
│   │   └── libOrbbecSDK.so.2.4.11  # Orbbec SDK 动态库（完整版本）
│   ├── config/                 # 配置文件目录
│   └── build/                  # 编译输出目录
│       ├── stream_gst          # 可执行文件
│       └── timestamp_sender.csv # 时间戳日志
├── imu/                        # IMU/AHRS 数据串口读取与 UDP 转发服务
│   ├── README.md               # IMU 模块说明
│   ├── Makefile                # Make 构建配置
│   ├── src/
│   │   ├── main.cpp            # IMU 数据读取与 UDP 转发主程序
│   │   ├── imu_config.cpp      # IMU 配置与数据解析
│   │   └── serial_port.cpp     # 串口通信封装
│   ├── include/
│   │   ├── imu_config.h        # IMU 配置头文件
│   │   └── serial_port.h       # 串口通信头文件
│   ├── build/                  # 编译输出目录
│   │   ├── imu                 # 可执行文件
│   │   ├── main.o
│   │   ├── imu_config.o
│   │   └── serial_port.o
│   └── backup/                 # 代码备份
│       ├── main.cpp.backup_20260902_150641
│       └── main.backup_20260902_150641
├── server/                     # 机器人主控服务
│   ├── Makefile                # Make 构建配置
│   ├── src/
│   │   ├── main.c              # 主控主程序（多线程架构）
│   │   ├── ipc/                # 进程间通信模块
│   │   ├── sensor_modules/     # 传感器采集模块
│   │   └── thread_modules/     # 线程管理模块
│   ├── include/
│   │   ├── shared_data.h       # 共享数据结构定义
│   │   └── motor_api.h         # 电机控制 API 头文件
│   └── build/                  # 编译输出目录
│       ├── industrial_gateway  # 可执行文件
│       ├── main.o
│       ├── ipc/
│       ├── sensor_modules/
│       └── thread_modules/
└── ntptp/                      # NTP/PTP 时间同步服务
    ├── chrony.conf             # Chrony NTP 客户端配置
    ├── ptp4l-master.cfg        # PTP4l 主时钟配置
    └── ptp4l.service           # PTP4l systemd 服务单元
```

---

## 模块说明

### 1. 激光雷达模块 (`radar/`)

基于 Livox Mid360 激光雷达的点云数据读取与 UDP 转发服务。

**核心文件：**

| 文件 | 说明 |
|------|------|
| `src/main.cpp` | 主程序：初始化 Livox SDK、连接雷达、注册点云回调、UDP 转发点云数据 |
| `src/synchro.cpp` | 时间戳同步逻辑 |
| `include/livox_lidar_api.h` | Livox SDK API 接口 |
| `include/livox_lidar_def.h` | 雷达数据类型定义 |
| `include/livox_lidar_cfg.h` | 雷达配置定义 |
| `include/synchro.h` | 同步模块接口 |

**配置文件：**

| 文件 | 说明 |
|------|------|
| `config/mid360_config.json` | Mid360 雷达设备配置 |
| `config/hap_config.json` | HAP 协议配置 |
| `config/config.json` | 通用配置 |

**功能特性：**

- 使用 Livox Lidar SDK 连接 Mid360 雷达设备
- 读取点云以太网数据包并通过 UDP 转发到上位机（默认 `192.168.7.2:9999`）
- 绑定 CPU 核心 2 运行，设置 FIFO 实时调度优先级（优先级 80）
- 增大 UDP 发送缓冲区（8MB）、禁用校验和以提升吞吐量
- 自定义 UDP 转发协议：雷达时间戳 + 接收时间戳 + 发送时间戳 + 帧计数器 + 数据长度 + 原始数据

**构建：**

```bash
cd radar
make
# 或
mkdir -p build && cd build && cmake .. && make
```

**运行：**

```bash
./build/radar ./config/mid360_config.json
```

---

### 2. 视频处理模块 (`vedio_processing/`)

基于 Orbbec 深度相机的视频流处理程序，实现多路视频帧的时间戳对齐与 UDP 同步信号发送。

**核心文件：**

| 文件 | 说明 |
|------|------|
| `src/4vedio_processiong.cpp` | 主程序：Orbbec 相机连接、GStreamer 管道构建、视频流采集、时间戳同步 |
| `include/ObSensor.hpp` | Orbbec SDK C++ 接口 |
| `include/ObSensor.h` | Orbbec SDK C 接口 |

**功能特性：**

- 连接 Orbbec 深度相机设备，枚举可用深度流配置
- GStreamer 管道构建与视频流采集
- 多格式图像转换（YUYV/UYVY/NV12/NV21/I420/YV12/Y16/RGB/BGR 等转 NV12）
- GStreamer Pad Probe 捕获 v4l2 流时间戳
- 四路视频流 RTP/H.264 UDP 发送（深度/彩色/两路 v4l2）
- UDP 时间戳同步信号发送（端口 6001）
- 时间戳日志记录（CSV 格式输出至 `build/timestamp_sender.csv`）
- 信号处理（SIGINT/SIGTERM 优雅退出）

**GStreamer Pipeline 输出端口（视频流）：**

| 流名称 | 设备/来源 | 端口 | 编码格式 | 分辨率 | 帧率 | 说明 |
|--------|-----------|------|----------|--------|------|------|
| Depth | Orbbec 深度相机 | UDP :9000 | H.264 (rtph264pay) | 1280x720 | 30fps | 深度相机视频流 |
| Color | Orbbec 彩色相机 | UDP :9001 | H.264 (rtph264pay) | 1280x720 (自适应) | 30fps | 彩色相机视频流 |
| Cam1 | v4l2src /dev/video22 | UDP :9002 | H.264 (rtph264pay) | 1280x720 | 30fps | 外部摄像头1 |
| Cam2 | v4l2src /dev/video31 | UDP :9003 | H.264 (rtph264pay) | 1280x720 | 30fps | 外部摄像头2 |

> 注意：Cam1 和 Cam2 的 v4l2 设备节点通过 `access()` 检测，如果设备不存在则不会创建对应管道。

**UDP 时间戳数据包结构（发给上位机的时间同步信号，不包含视频图像数据）：**

| 字段 | 类型 | 说明 |
|------|------|------|
| pts_ns | uint64_t | 帧捕获时间戳（纳秒，Unix） |
| send_time_ns | uint64_t | 发送时间戳（纳秒，Unix） |
| id | char[8] | 流标识符（depth/color/cam1/cam2） |

**构建：**

```bash
cd vedio_processing
make
```

**运行：**

```bash
./build/stream_gst
```

---

### 3. IMU 模块 (`imu/`)

IMU/AHRS 传感器数据串口读取与 UDP 转发服务。

**核心文件：**

| 文件 | 说明 |
|------|------|
| `src/main.cpp` | 主程序：串口读取 IMU 数据、解析 AHRS 帧、UDP 转发 |
| `src/imu_config.cpp` | IMU 配置与数据解析逻辑 |
| `src/serial_port.cpp` | 串口通信封装（打开、配置、读写） |
| `include/imu_config.h` | IMU 配置头文件 |
| `include/serial_port.h` | 串口通信头文件 |

**功能特性：**

- 串口读取 IMU 原始数据（默认串口 `ttyS5`，波特率 921600）
- 解析 AHRS 帧（横滚角 roll、俯仰角 pitch、偏航角 yaw、角速度 gx/gy/gz、加速度 ax/ay/az）
- 通过 UDP 打包发送（帧序列号 + 发送时间戳 + 9 个浮点数据）
- 支持命令行参数指定目标 IP 和端口（默认 `192.168.7.2:9005`）
- 支持配置 IMU 采样率为 1000Hz（取消注释配置代码即可）

**构建：**

```bash
cd imu
make
```

**运行：**

```bash
# 默认串口 ttyS5，默认目标 192.168.7.2:9005
./build/imu

# 指定串口和目标地址
./build/imu /dev/ttyS0 192.168.7.100 9005
```

---

### 4. 主控服务器 (`server/`)

多架构机器人主控程序，通过 POSIX 消息队列实现线程间通信，统一管理环境传感器、IMU、电机控制和 TCP 上报。

**核心文件：**

| 文件 | 说明 |
|------|------|
| `src/main.c` | 主控主程序：多线程初始化、线程创建、主循环 |
| `src/ipc/` | 进程间通信模块（POSIX 消息队列封装） |
| `src/sensor_modules/` | 传感器采集模块（环境传感器、IMU 等） |
| `src/thread_modules/` | 线程管理模块（显示、TCP 上报、电机控制等） |
| `include/shared_data.h` | 全局共享数据结构定义 |
| `include/motor_api.h` | 电机控制 API 定义 |

**线程模型：**

| 线程 | 功能 |
|------|------|
| env_sensor_thread | 采集 BME68X 环境传感器数据（温度、湿度、气压、气体电阻） |
| imu_collector_thread | 采集 IMU 数据（加速度、角速度） |
| motor_status_thread | 读取电机状态（转速、电流、位置、电压、告警码） |
| display_thread | 终端 UI 显示实时系统状态 |
| tcp_report_thread | TCP 服务端监听上位机连接，上报传感器和电机数据，接收控制命令 |
| motor_control_thread | 接收控制命令，驱动电机（位置/速度/力矩模式） |

**关键配置：**

- 默认 TCP 监听地址：`0.0.0.0:8888`（监听所有网卡）
- 电机 1 最大电流：1400mA，最大速度：200 RPM
- 电机 2 最大电流：1400mA，最大速度：200 RPM
- 默认加速度：1000 RPM/s²，减速度：1000 RPM/s²
- CAN 波特率：1Mbps

**构建：**

```bash
cd server
make
```

**运行：**

```bash
./build/industrial_gateway
```

---

### 5. 时间同步服务 (`ntptp/`)

NTP/PTP 网络时间同步配置，用于整个机器人系统的多节点时间对齐。

| 文件 | 说明 |
|------|------|
| `chrony.conf` | Chrony NTP 客户端配置文件 |
| `ptp4l-master.cfg` | PTP4l 主时钟（IEEE 1588）配置文件 |
| `ptp4l.service` | PTP4l systemd 服务单元文件 |

---

## 通信协议详解

本节详细描述所有网络通信接口的协议格式，供 Client 端开发参考。

> **重要说明**：所有二进制协议字段默认采用**小端序（Little Endian）**排列。UDP 数据包以原始二进制形式发送，TCP 以文本（UTF-8）形式传输，以换行符 `\n` 分隔。

### 协议总览

| 端口 | 协议 | 方向 | 用途 | 所属模块 |
|------|------|------|------|----------|
| 6001 | UDP | 板卡→上位机 | 视频时间戳同步信号 | vedio_processing |
| 9000 | UDP (RTP/H.264) | 板卡→上位机 | 深度相机视频流 | vedio_processing |
| 9001 | UDP (RTP/H.264) | 板卡→上位机 | 彩色相机视频流 | vedio_processing |
| 9002 | UDP (RTP/H.264) | 板卡→上位机 | 外部摄像头1视频流 | vedio_processing |
| 9003 | UDP (RTP/H.264) | 板卡→上位机 | 外部摄像头2视频流 | vedio_processing |
| 9005 | UDP (二进制) | 板卡→上位机 | IMU/AHRS 姿态数据 | imu |
| 9999 | UDP (二进制) | 板卡→上位机 | Livox 激光雷达点云数据 | radar |
| 8888 | TCP (文本) | 板卡↔上位机 | 环境/电机数据上报 + 电机控制命令 | server |

---

### 一、视频时间戳同步（UDP :6001）

视频处理模块每采集一帧视频，都会通过 UDP 发送一个时间戳包，记录帧捕获时间和发送时间，用于上位机做时间对齐和延时计算。

**数据包结构（固定 24 字节，C 结构体）：**

```c
struct TimestampPacket {
    uint64_t pts_ns;        // 偏移0:  8字节 - 帧捕获时间戳（纳秒，Unix epoch）
    uint64_t send_time_ns;  // 偏移8:  8字节 - UDP发送时间戳（纳秒，Unix epoch）
    char id[8];             // 偏移16: 8字节 - 流标识符，字符串，'\0'结尾
};                           // 总计：24字节
```

**字段说明：**

| 字段 | 偏移 | 类型 | 大小 | 说明 |
|------|------|------|------|------|
| pts_ns | 0 | uint64_t | 8 bytes | 帧被相机捕获时的 Unix 时间戳（纳秒）。由 GStreamer PTS 通过 v4l2 probe 获取并转换为 Unix 时间 |
| send_time_ns | 8 | uint64_t | 8 bytes | 数据包通过 sendto 发出时的 Unix 时间戳（纳秒）。可用于计算端到端延时 |
| id | 16 | char[8] | 8 bytes | 流标识符。可能的值：depth、color、cam1、cam2 |

**Python 接收示例：**

```python
import socket
import struct

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('0.0.0.0', 6001))

while True:
    data, addr = sock.recvfrom(24)
    pts_ns, send_time_ns, id_bytes = struct.unpack('<QQ8s', data)
    stream_id = id_bytes.decode('ascii').rstrip('\0')
    latency_ns = send_time_ns - pts_ns
    print(f"[{stream_id}] pts={pts_ns}ns  send={send_time_ns}ns  latency={latency_ns}ns")
```

---

### 二、视频流接收（UDP :9000 ~ :9003）

视频处理模块通过 GStreamer 的 udpsink 组件，将四路视频流编码为 H.264 后以 RTP 包形式从四个 UDP 端口发出。

**四路视频流端口：**

| 流 | 端口 | 编码 | 分辨率 | 帧率 |
|----|------|------|--------|------|
| 深度相机 | :9000 | H.264 (RTP) | 1280x720 | 30fps |
| 彩色相机 | :9001 | H.264 (RTP) | 自适应（优先1280x720） | 30fps |
| 摄像头1 | :9002 | H.264 (RTP) | 1280x720 | 30fps |
| 摄像头2 | :9003 | H.264 (RTP) | 1280x720 | 30fps |

这些端口发送的是标准 RTP over UDP 包（payload type = 96/97/98）。

**接收方式1：GStreamer（推荐）**

```bash
# 接收深度相机流并播放
gst-launch-1.0 udpsrc port=9000 ! application/x-rtp,payload=96 ! rtph264depay ! avdec_h264 ! autovideosink

# 接收彩色相机流
gst-launch-1.0 udpsrc port=9001 ! application/x-rtp,payload=96 ! rtph264depay ! avdec_h264 ! autovideosink

# 接收摄像头1流
gst-launch-1.0 udpsrc port=9002 ! application/x-rtp,payload=97 ! rtph264depay ! avdec_h264 ! autovideosink

# 接收摄像头2流
gst-launch-1.0 udpsrc port=9003 ! application/x-rtp,payload=98 ! rtph264depay ! avdec_h264 ! autovideosink
```

**接收方式2：VLC 直接播放**

```
vlc rtp://@:9000
vlc rtp://@:9001
vlc rtp://@:9002
vlc rtp://@:9003
```

**接收方式3：Python (GStreamer Python bindings)**

```python
import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst

Gst.init(None)

# 为每个流创建 udpsrc pipeline
def create_pipeline(port, payload=96):
    pipeline = Gst.parse_launch(
        f'udpsrc port={port} ! '
        f'application/x-rtp,payload={payload} ! '
        f'rtph264depay ! '
        f'avdec_h264 ! '
        f'videoconvert ! '
        f'appsink name=sink'
    )
    return pipeline

# 深度相机
pipe_depth = create_pipeline(9000)
pipe_depth.get_by_name('sink').set_property('emit-signals', True)
pipe_depth.set_state(Gst.State.PLAYING)
```

---

### 三、IMU/AHRS 姿态数据（UDP :9005）

IMU 模块通过串口读取 AHRS 传感器数据，每帧打包为固定 64 字节的二进制包通过 UDP 发送。

**数据包结构（固定 64 字节）：**

| 字段 | 偏移 | 类型 | 大小 | 说明 |
|------|------|------|------|------|
| frame_seq | 0 | uint64_t | 8 bytes | 帧序列号，从0开始递增，可用于检测丢包 |
| send_time_us | 8 | uint64_t | 8 bytes | 发送时间戳（微秒，Unix epoch）。通过 clock_gettime(CLOCK_REALTIME) 获取 |
| roll | 16 | float (IEEE 754) | 4 bytes | 横滚角（弧度） |
| pitch | 20 | float (IEEE 754) | 4 bytes | 俯仰角（弧度） |
| yaw | 24 | float (IEEE 754) | 4 bytes | 偏航角（弧度） |
| gx | 28 | float (IEEE 754) | 4 bytes | 角速度 X 轴（rad/s） |
| gy | 32 | float (IEEE 754) | 4 bytes | 角速度 Y 轴（rad/s） |
| gz | 36 | float (IEEE 754) | 4 bytes | 角速度 Z 轴（rad/s） |
| ax | 40 | float (IEEE 754) | 4 bytes | 加速度 X 轴（m/s²） |
| ay | 44 | float (IEEE 754) | 4 bytes | 加速度 Y 轴（m/s²） |
| az | 48 | float (IEEE 754) | 4 bytes | 加速度 Z 轴（m/s²） |
| (padding) | 52 | — | 12 bytes | 填充至64字节（接收端可忽略） |

> 注意：所有浮点数均为小端序（Little-Endian），与 x86/aarch64 一致。

**Python 接收与解析示例：**

```python
import socket
import struct
import math

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('0.0.0.0', 9005))

print("等待IMU数据...")
while True:
    data, addr = sock.recvfrom(64)

    # 解析固定64字节包（实际数据在前52字节）
    frame_seq, send_time_us, roll, pitch, yaw, gx, gy, gz, ax, ay, az = struct.unpack('<QQfffffffff', data[:52])

    # 转换为可读单位
    roll_deg = math.degrees(roll)
    pitch_deg = math.degrees(pitch)
    yaw_deg = math.degrees(yaw)

    send_time_s = send_time_us / 1e6
    print(f"seq={frame_seq}  time={send_time_s:.6f}s  "
          f"roll={roll_deg:.2f}deg pitch={pitch_deg:.2f}deg yaw={yaw_deg:.2f}deg  "
          f"gx={gx:.3f} gy={gy:.3f} gz={gz:.3f}  "
          f"ax={ax:.3f} ay={ay:.3f} az={az:.3f}")
```

**C 语言接收端结构体定义：**

```c
#pragma pack(push, 1)
struct ImuPacket {
    uint64_t frame_seq;
    uint64_t send_time_us;
    float roll, pitch, yaw;
    float gx, gy, gz;
    float ax, ay, az;
};
#pragma pack(pop)
// sizeof(struct ImuPacket) == 52，实际UDP包为64字节（含尾部padding）
```

---

### 四、激光雷达点云数据（UDP :9999）

雷达模块通过 Livox SDK 读取 Mid360 激光雷达的点云数据，每包加上时间戳元数据后通过 UDP 转发。

**ForwardPacket 结构体（packed，变长）：**

```c
#pragma pack(push, 1)
struct ForwardPacket {
    uint64_t radar_timestamp_us;  // 偏移0:  8字节 - 雷达时间戳（微秒）。从Livox数据包中的8字节时间戳数组提取，除以1000转为微秒
    uint64_t receive_time_us;     // 偏移8:  8字节 - 板卡接收时间戳（微秒，CLOCK_REALTIME）
    uint64_t send_time_us;        // 偏移16: 8字节 - 板卡发送时间戳（微秒，与receive_time_us相同）
    uint32_t frame_counter;       // 偏移24: 4字节 - 雷达帧计数器（data->frame_cnt）
    uint32_t data_length;         // 偏移28: 4字节 - 后续raw_data的实际字节数
    uint8_t  raw_data[1];         // 偏移32: 变长  - LivoxLidarEthernetPacket的原始数据
};
#pragma pack(pop)
// 头部固定36字节 + 变长 raw_data
```

**字段说明：**

| 字段 | 偏移 | 类型 | 大小 | 说明 |
|------|------|------|------|------|
| radar_timestamp_us | 0 | uint64_t | 8 bytes | 雷达时间戳（微秒）。Livox SDK回调中timestamp是uint8_t[8]数组（小端序），提取为纳秒后除以1000转为微秒 |
| receive_time_us | 8 | uint64_t | 8 bytes | 板卡接收时间戳（微秒，CLOCK_REALTIME） |
| send_time_us | 16 | uint64_t | 8 bytes | 板卡发送时间戳（微秒，与receive_time_us相同） |
| frame_counter | 24 | uint32_t | 4 bytes | 雷达帧计数器 |
| data_length | 28 | uint32_t | 4 bytes | 后续raw_data的实际字节数 |
| raw_data | 32 | uint8_t[] | 变长 | Livox原始点云数据，具体格式取决于Livox SDK版本和雷达配置 |

**raw_data 内容说明：**

raw_data 是 Livox SDK 回调函数 PointCloudCallback 中 LivoxLidarEthernetPacket* data 指向的原始点云数据包。具体点结构定义请参考项目中的 `livox_lidar_def.h` 头文件，常见的点格式包括 LivoxLidarCartesianHighRawPoint 和 LivoxLidarCartesianLowRawPoint。

**时间戳提取逻辑（源码）：**

```c
// Livox SDK 中 timestamp 是 uint8_t[8] 数组（小端序）
uint64_t ExtractTimestampNs(const uint8_t* timestamp_array) {
    uint64_t timestamp = 0;
    for (int i = 0; i < 8; i++) {
        timestamp |= ((uint64_t)timestamp_array[i]) << (i * 8);
    }
    return timestamp;  // 纳秒
}
// 转发时转为微秒：radar_timestamp_us = ExtractTimestampNs(data->timestamp) / 1000;
```

**Python 接收示例：**

```python
import socket
import struct

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 8 * 1024 * 1024)  # 8MB接收缓冲
sock.bind(('0.0.0.0', 9999))

while True:
    # 先接收头部（36字节）
    header_data, addr = sock.recvfrom(36)
    radar_ts_us, recv_ts_us, send_ts_us, frame_cnt, data_len = struct.unpack('<QQQII', header_data)

    # 再接收变长数据
    if data_len > 0:
        raw_data, _ = sock.recvfrom(data_len)
        print(f"frame={frame_cnt} len={data_len} "
              f"radar_ts={radar_ts_us}us recv={recv_ts_us}us send={send_ts_us}us")
        # raw_data 包含 Livox 原始点云数据
        # 具体解析需参考 livox_lidar_def.h 中的点结构定义
```

---

### 五、TCP 机器人数据与电机控制（TCP :8888）

主控服务器以 TCP 服务端模式监听 `0.0.0.0:8888`，上位机（Client）连接后实现双向通信：板卡主动上报传感器/电机数据，上位机下发电机控制命令。

**通信模式：**

- 板卡作为 TCP Server，上位机作为 TCP Client 发起连接
- 板卡以约 10Hz 的频率主动上报数据
- 上位机可随时发送命令（基于文本协议，每条约以换行符结尾）
- 板卡每帧上报后发送一个心跳包（单个换行符）维持连接

#### 5.1 板卡 → 上位机（数据上报）

**环境传感器数据：**

```
ENV:TEMP=25.3,HUM=60.1,PRESS=1013.2,GAS=1500,TS=1697012345.123456
```

| 字段 | 说明 | 单位 |
|------|------|------|
| TEMP | 温度 | 摄氏度 |
| HUM | 湿度 | %RH |
| PRESS | 气压 | hPa |
| GAS | 气体电阻 | 欧姆 |
| TS | 时间戳 | 秒（浮点，Unix epoch） |

**电机状态数据：**

```
MOTOR:M1_SPD=120.5,M1_CUR=800,M1_POS=45.2,M1_V=24.5,M2_SPD=0.0,M2_CUR=0,M2_POS=0.0,M2_V=24.5,M1_ALARM=0x00000000,M2_ALARM=0x00000000,TS=1697012345.123456
```

| 字段 | 说明 | 单位 |
|------|------|------|
| M1_SPD | 电机1转速 | RPM |
| M1_CUR | 电机1电流 | mA |
| M1_POS | 电机1位置 | 度 |
| M1_V | 电机1电压 | V |
| M2_SPD | 电机2转速 | RPM |
| M2_CUR | 电机2电流 | mA |
| M2_POS | 电机2位置 | 度 |
| M2_V | 电机2电压 | V |
| M1_ALARM | 电机1告警码 | 十六进制 |
| M2_ALARM | 电机2告警码 | 十六进制 |
| TS | 时间戳 | 秒（浮点，Unix epoch） |

#### 5.2 上位机 → 板卡（控制命令）

**电机控制命令（MOTOR_CMD）：**

```
MOTOR_CMD:M1_ANGLE=45.0,M1_SPEED=100,M1_ENABLE=1,M1_STOP=0,M2_ANGLE=0,M2_SPEED=0,M2_ENABLE=0,M2_STOP=0,MODE=0
```

| 参数 | 类型 | 说明 | 可选值 |
|------|------|------|--------|
| M1_ANGLE | float | 电机1目标角度 | 度 |
| M1_SPEED | float | 电机1目标速度 | RPM |
| M1_ENABLE | int | 电机1使能 | 0=关, 1=开 |
| M1_STOP | int | 电机1急停 | 0=正常, 1=停止（触发后强制关闭Enable） |
| M2_ANGLE | float | 电机2目标角度 | 度 |
| M2_SPEED | float | 电机2目标速度 | RPM |
| M2_ENABLE | int | 电机2使能 | 0=关, 1=开 |
| M2_STOP | int | 电机2急停 | 0=正常, 1=停止 |
| MODE | int | 控制模式 | 0=位置模式, 1=速度模式, 2=力矩模式 |

> 增量更新机制：未指定的参数将保持上一次的有效值不变。例如只发 `MOTOR_CMD:M1_ENABLE=1` 只会使能电机1，其他参数不改变。

**日志消息（LOG_MSG）：**

```
LOG_MSG:system startup complete
```

板卡收到后写入内部日志队列，可用于远程调试。

**系统命令（SYS_CMD）：**

```
SYS_CMD:SHUTDOWN    # 关机
SYS_CMD:STATUS      # 查询系统状态
SYS_CMD:RESET       # 复位（重置电机使能记忆）
```

**NTP 时间同步请求：**

上位机发送 `NTP_REQ` 后，板卡回复当前时间：

```
上位机发送: NTP_REQ
板卡回复:   NTP_RESP:TIMESTAMP=1697012345.123456
```

#### 5.3 Python TCP Client 完整示例

```python
import socket
import time
import threading

class RobotClient:
    def __init__(self, host='192.168.7.1', port=8888):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((host, port))
        self._running = True
        self._recv_thread = threading.Thread(target=self._recv_loop, daemon=True)
        self._recv_thread.start()

    def _recv_loop(self):
        buffer = b''
        while self._running:
            try:
                data = self.sock.recv(4096)
                if not data:
                    break
                buffer += data
                while b'\n' in buffer:
                    line, buffer = buffer.split(b'\n', 1)
                    line = line.decode('utf-8', errors='replace').strip()
                    if line:
                        print(f"[SERVER] {line}")
                        if line.startswith('ENV:'):
                            self._parse_env(line)
                        elif line.startswith('MOTOR:'):
                            self._parse_motor(line)
                        elif line.startswith('NTP_RESP:'):
                            self._parse_ntp(line)
            except socket.error:
                break

    def _parse_env(self, line):
        fields = {}
        for item in line[5:].split(','):
            if '=' in item:
                k, v = item.split('=', 1)
                fields[k] = v
        print(f"  温度={fields.get('TEMP')}C  湿度={fields.get('HUM')}%  气压={fields.get('PRESS')}hPa")

    def _parse_motor(self, line):
        fields = {}
        for item in line[7:].split(','):
            if '=' in item:
                k, v = item.split('=', 1)
                fields[k] = v
        print(f"  M1: speed={fields.get('M1_SPD')}rpm  pos={fields.get('M1_POS')}deg")
        print(f"  M2: speed={fields.get('M2_SPD')}rpm  pos={fields.get('M2_POS')}deg")

    def _parse_ntp(self, line):
        ts = line.split('=')[1]
        print(f"  板卡时间: {ts}")

    def send_motor_cmd(self, m1_angle=0, m1_speed=0, m1_enable=0, m1_stop=0,
                       m2_angle=0, m2_speed=0, m2_enable=0, m2_stop=0, mode=0):
        cmd = (f"MOTOR_CMD:M1_ANGLE={m1_angle},M1_SPEED={m1_speed},"
               f"M1_ENABLE={m1_enable},M1_STOP={m1_stop},"
               f"M2_ANGLE={m2_angle},M2_SPEED={m2_speed},"
               f"M2_ENABLE={m2_enable},M2_STOP={m2_stop},"
               f"MODE={mode}\n")
        self.sock.sendall(cmd.encode('utf-8'))

    def send_log(self, msg):
        cmd = f"LOG_MSG:{msg}\n"
        self.sock.sendall(cmd.encode('utf-8'))

    def send_sys_cmd(self, cmd):
        self.sock.sendall(f"SYS_CMD:{cmd}\n".encode('utf-8'))

    def request_ntp(self):
        self.sock.sendall(b"NTP_REQ\n")

    def close(self):
        self._running = False
        self.sock.close()

# 使用示例
if __name__ == '__main__':
    client = RobotClient('192.168.7.1')
    time.sleep(1)
    client.send_motor_cmd(m1_angle=45.0, m1_enable=1)
    time.sleep(2)
    client.request_ntp()
    client.close()
```

---

### 通信协议总结表

| 端口 | 协议 | 方向 | 数据格式 | 用途 |
|------|------|------|----------|------|
| 6001 | UDP | 板→上位机 | 二进制（24字节固定） | 视频时间戳同步 |
| 9000 | UDP (RTP) | 板→上位机 | H.264 RTP包 | 深度相机视频流 |
| 9001 | UDP (RTP) | 板→上位机 | H.264 RTP包 | 彩色相机视频流 |
| 9002 | UDP (RTP) | 板→上位机 | H.264 RTP包 | 摄像头1视频流 |
| 9003 | UDP (RTP) | 板→上位机 | H.264 RTP包 | 摄像头2视频流 |
| 9005 | UDP | 板→上位机 | 二进制（64字节固定） | IMU/AHRS姿态数据 |
| 9999 | UDP | 板→上位机 | 二进制（36字节头+变长） | 激光雷达点云数据 |
| 8888 | TCP | 双向 | 文本协议（换行分隔） | 传感器上报 + 电机控制 |

---

## 依赖库汇总

| 库名 | 用途 | 适用模块 |
|------|------|----------|
| Livox Lidar SDK | 激光雷达驱动与点云处理 | radar/ |
| OrbbecSDK | 深度相机驱动 | vedio_processing/ |
| GStreamer | 视频流处理管道 | vedio_processing/ |
| OpenCV4 | 图像格式转换 | vedio_processing/ |
| BME68X | 环境传感器驱动 | server/ |
| POSIX msg queue | 线程间通信 | server/ |
| serial_port | 串口通信 | imu/ |
| imu_config | IMU 配置与数据解析 | imu/ |

---

## 网络端口汇总

| 端口 | 协议 | 用途 | 所属模块 |
|------|------|------|----------|
| 6001 | UDP | 视频时间戳同步信号 | vedio_processing/ |
| 9000 | UDP (RTP/H.264) | 深度相机视频流 | vedio_processing/ |
| 9001 | UDP (RTP/H.264) | 彩色相机视频流 | vedio_processing/ |
| 9002 | UDP (RTP/H.264) | 外部摄像头1视频流 | vedio_processing/ |
| 9003 | UDP (RTP/H.264) | 外部摄像头2视频流 | vedio_processing/ |
| 9005 | UDP | IMU 数据转发目标 | imu/ |
| 9999 | UDP | 激光雷达点云转发目标 | radar/ |
| 8888 | TCP | 机器人数据上报与电机控制 | server/ |

---

## 注意事项

1. **实时性优化**：`radar/src/main.cpp` 绑定了 CPU 核心 2 并使用 FIFO 实时调度，需要在具有实时内核支持的系统上运行以获得最佳性能。
2. **串口权限**：运行 `imu/src/main.cpp` 需要串口设备（如 `/dev/ttyS5`）的读写权限。
3. **Orbbec 驱动**：`vedio_processing/src/4vedio_processiong.cpp` 需要安装 Orbbec SDK 及 USB 权限规则。
4. **网络配置**：所有组件的目标 IP 默认均为 `192.168.7.2`，请根据实际网络环境修改。
5. **优雅退出**：所有组件均支持通过 SIGINT/SIGTERM（Ctrl+C）优雅退出并释放资源。
6. **时间同步**：建议部署 `ntptp/` 中的 PTP/NTP 配置以确保多传感器时间对齐。
7. **环境变量**：项目根目录的 `.env` 文件可能包含敏感配置（如密钥、路径等），请勿提交到公开仓库。