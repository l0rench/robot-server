# 	Linux例程

在Linux环境下接收原极IMU 二进制数据帧并显示

* 测试环境： Ubuntu 20.04 / 树莓派4B
* 支持硬件: 	所有原极IMU产品

## 文件说明

| 文件                          | 位置 | 说明                             |
| ----------------------------- | ---- | -------------------------------- |
| serial_port.cpp/ serial_port.h | .    | linux C串口驱动封装              |

                 

## 使用

1. 查找串口设备,  确定Linux可以找到你的tty串口，命令行切换到本目录下执行make，生成可执行文件main
```shell
 $ ls /dev/ttyUSB*
```

2. 编译example, 切到本目录下执行make. 编译为可执行文件main

```
$ make
gcc -I../lib -c main.c -o main.o
gcc -I../lib -c serial_port.c -o serial_port.o
gcc -I../lib -c ../lib/ch_serial.c -o ../lib/ch_serial.o
gcc -I../lib main.o serial_port.o ../lib/ch_serial.o -o main
Cleaning up...
```

3. 运行main
```
$ sudo ./main ttyUSB0 115200
isatty success!
* ttyUSB0 successfully open with 115200.


```

