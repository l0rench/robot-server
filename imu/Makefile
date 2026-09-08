# ============================================================
# Makefile for IMU Project (build/src/include structure)
# ============================================================

# 编译器
CXX = g++

# 编译选项
CXXFLAGS = -std=c++11 -Wall -pthread -O2

# 调试选项（取消注释可开启调试）
# CXXFLAGS += -g -DDEBUG

# 目录定义
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build

# 目标程序
TARGET = $(BUILD_DIR)/imu

# 源文件（自动查找 src/ 下的所有 .cpp）
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)

# 自动生成目标文件列表（放在 build/ 目录下）
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))

# 头文件路径
INCLUDES = -I$(INCLUDE_DIR)

# 链接库
LIBS = -lm

# 完整编译参数
CFLAGS = $(CXXFLAGS) $(INCLUDES)
LDFLAGS = $(LIBS)

# ============================================================
# 编译规则
# ============================================================

# 默认目标
all: $(BUILD_DIR) $(TARGET)

# 创建 build 目录
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)
	@echo "  Created build directory"

# 链接
$(TARGET): $(OBJECTS)
	@echo ""
	@echo "========================================"
	@echo "  Linking $(TARGET)..."
	@echo "========================================"
	@echo ""
	$(CXX) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo ""
	@echo "========================================"
	@echo "  Build successful! ✓"
	@echo "  Run: ./$(TARGET)"
	@echo "========================================"
	@echo ""

# 编译 .cpp 到 .o（放在 build/ 目录下）
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	@echo "  Compiling $<..."
	$(CXX) $(CFLAGS) -c $< -o $@

# ============================================================
# 辅助目标
# ============================================================

# 清理
clean:
	@echo "Cleaning..."
	rm -rf $(BUILD_DIR)
	@echo "Done."

# 清理并重新编译
rebuild: clean all

# 运行程序
run: $(TARGET)
	@echo ""
	@echo "========================================"
	@echo "  Running $(TARGET)..."
	@echo "========================================"
	@echo ""
	./$(TARGET)

# 显示编译参数（调试用）
show:
	@echo "CXX:        $(CXX)"
	@echo "CXXFLAGS:   $(CXXFLAGS)"
	@echo "CFLAGS:     $(CFLAGS)"
	@echo "LDFLAGS:    $(LDFLAGS)"
	@echo "SOURCES:    $(SOURCES)"
	@echo "OBJECTS:    $(OBJECTS)"
	@echo "TARGET:     $(TARGET)"
	@echo "SRC_DIR:    $(SRC_DIR)"
	@echo "INCLUDE_DIR: $(INCLUDE_DIR)"
	@echo "BUILD_DIR:  $(BUILD_DIR)"

# ============================================================
# 安装到系统路径（可选）
# ============================================================

install:
	@echo "Installing to /usr/local/bin..."
	sudo cp $(TARGET) /usr/local/bin/
	@echo "Done."

uninstall:
	@echo "Removing from /usr/local/bin..."
	sudo rm -f /usr/local/bin/$(TARGET)
	@echo "Done."

# ============================================================
# 帮助信息
# ============================================================

help:
	@echo ""
	@echo "========================================"
	@echo "  Makefile for IMU Project"
	@echo "========================================"
	@echo ""
	@echo "  Directory structure:"
	@echo "    src/      - Source files (.cpp)"
	@echo "    include/  - Header files (.h)"
	@echo "    build/    - Compiled files (auto-generated)"
	@echo ""
	@echo "  Targets:"
	@echo "    all      - Build $(TARGET) (default)"
	@echo "    clean    - Remove build directory"
	@echo "    rebuild  - Clean and rebuild"
	@echo "    run      - Build and run the program"
	@echo "    show     - Show compile parameters"
	@echo "    install  - Install to /usr/local/bin"
	@echo "    uninstall- Remove from /usr/local/bin"
	@echo "    help     - Show this help"
	@echo ""
	@echo "  Examples:"
	@echo "    make          # Build the program"
	@echo "    make run      # Build and run"
	@echo "    make clean    # Clean build files"
	@echo "    make rebuild  # Clean and rebuild"
	@echo ""

# ============================================================
# 伪目标
# ============================================================

.PHONY: all clean rebuild run show install uninstall help