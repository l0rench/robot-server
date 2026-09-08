# ============================================================
# Makefile for TCP Industrial Gateway
# ============================================================

CC = gcc
CXX = g++
CFLAGS = -Wall -Wextra -O2 -pthread
CXXFLAGS = -Wall -Wextra -O2 -pthread
LIBS = -lm -lrt

# 目录定义
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build

# 头文件路径
INCLUDES = -I$(INCLUDE_DIR) \
           -I$(SRC_DIR) \
           -I$(SRC_DIR)/ipc \
           -I$(SRC_DIR)/sensor_modules/motor \
           -I$(SRC_DIR)/sensor_modules/imu \
           -I$(SRC_DIR)/sensor_modules/bme68x \
           -I$(SRC_DIR)/thread_modules

# 自动查找所有 .c 文件
C_SRCS = $(shell find $(SRC_DIR) -name "*.c")
CXX_SRCS = $(shell find $(SRC_DIR) -name "*.cpp")

# 生成目标文件列表（放在 build/ 下，保持目录结构）
C_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(C_SRCS))
CXX_OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(CXX_SRCS))
OBJS = $(C_OBJS) $(CXX_OBJS)

# 目标程序
TARGET = $(BUILD_DIR)/industrial_gateway

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/ipc
	@mkdir -p $(BUILD_DIR)/sensor_modules/motor
	@mkdir -p $(BUILD_DIR)/sensor_modules/imu
	@mkdir -p $(BUILD_DIR)/sensor_modules/bme68x
	@mkdir -p $(BUILD_DIR)/thread_modules
	@echo "  Created build directory"

$(TARGET): $(OBJS)
	@echo ""
	@echo "========================================"
	@echo "  Linking $(TARGET)..."
	@echo "========================================"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)
	@echo ""
	@echo "  Build successful! ✓"
	@echo "  Run: sudo ./$(TARGET)"
	@echo ""

# C 文件编译规则
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "  Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# C++ 文件编译规则
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "  Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

run: $(TARGET)
	sudo ./$(TARGET)

debug: CFLAGS += -g -DDEBUG -DBME68X_DEBUG
debug: CXXFLAGS += -g -DDEBUG -DBME68X_DEBUG
debug: clean all

clean:
	@echo "Cleaning..."
	rm -rf $(BUILD_DIR)
	@echo "Done."

rebuild: clean all

install:
	@echo "Installing to /usr/local/bin..."
	sudo cp $(TARGET) /usr/local/bin/
	@echo "Done."

help:
	@echo ""
	@echo "  Targets:"
	@echo "    all      - Build (default)"
	@echo "    clean    - Remove build directory"
	@echo "    rebuild  - Clean and rebuild"
	@echo "    run      - Build and run (with sudo)"
	@echo "    debug    - Build with debug symbols"
	@echo "    install  - Install to /usr/local/bin"
	@echo "    help     - Show this help"
	@echo ""

.PHONY: all clean rebuild run debug install help