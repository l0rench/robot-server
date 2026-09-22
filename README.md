
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
- UDP 时间戳同步信号发送（端口 6001，目标 IP `192.168.7.2`）
- 时间戳日志记录（CSV 格式输出至 `build/timestamp_sender.csv`）
- 信号处理（SIGINT/SIGTERM 优雅退出）

**UDP 时间戳数据包结构：**

| 字段 | 类型 | 说明 |
|------|------|------|
| pts_ns | uint64_t | 帧捕获时间戳（纳秒，Unix） |
| send_time_ns | uint64_t | 发送时间戳（纳秒，Unix） |
| id | char[8] | 流标识符（cam1/cam2） |

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
| tcp_report_thread | TCP 客户端连接上位机，上报传感器和电机数据 |
| motor_control_thread | 接收控制命令，驱动电机（位置/速度/力矩模式） |

**关键配置：**

- 默认 TCP 服务器地址：`127.0.0.1:8080`
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
| 9005 | UDP | IMU 数据转发目标 | imu/ |
| 9999 | UDP | 激光雷达点云转发目标 | radar/ |
| 6001 | UDP | 视频时间戳同步信号 | vedio_processing/ |
| 8080 | TCP | 机器人数据上报（主控→上位机） | server/ |

---

## 注意事项

1. **实时性优化**：`radar/src/main.cpp` 绑定了 CPU 核心 2 并使用 FIFO 实时调度，需要在具有实时内核支持的系统上运行以获得最佳性能。
2. **串口权限**：运行 `imu/src/main.cpp` 需要串口设备（如 `/dev/ttyS5`）的读写权限。
3. **Orbbec 驱动**：`vedio_processing/src/4vedio_processiong.cpp` 需要安装 Orbbec SDK 及 USB 权限规则。
4. **网络配置**：所有组件的目标 IP 默认均为 `192.168.7.2`，请根据实际网络环境修改。
5. **优雅退出**：所有组件均支持通过 SIGINT/SIGTERM（Ctrl+C）优雅退出并释放资源。
6. **时间同步**：建议部署 `ntptp/` 中的 PTP/NTP 配置以确保多传感器时间对齐。
7. **环境变量**：项目根目录的 `.env` 文件可能包含敏感配置（如密钥、路径等），请勿提交到公开仓库。