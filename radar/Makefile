# ============================================================
# Makefile for Radar Project (with Livox SDK)
# ============================================================

CXX = g++
CXXFLAGS = -std=c++11 -Wall -pthread -O2

# 头文件路径（包含自己的 include 和 Livox SDK 头文件）
INCLUDES = -Iinclude -Iinclude/livox

# 库路径（使用项目内的 lib 目录）
LIB_DIR = lib
LIBS = -L$(LIB_DIR) -llivox_lidar_sdk_static -lpthread -lrt -lm -ldl

# 目录定义
SRC_DIR = src
BUILD_DIR = build
CONFIG_DIR = config
TARGET = $(BUILD_DIR)/radar

# 源文件
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)
	@echo "  Created build directory"

$(TARGET): $(OBJECTS)
	@echo ""
	@echo "========================================"
	@echo "  Linking $(TARGET)..."
	@echo "========================================"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)
	@echo ""
	@echo "  Build successful! ✓"
	@echo "  Run: ./$(TARGET) -c $(CONFIG_DIR)/config.json"
	@echo ""

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "  Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

run: $(TARGET)
	./$(TARGET) -c $(CONFIG_DIR)/config.json

run-hap: $(TARGET)
	./$(TARGET) -c $(CONFIG_DIR)/hap_config.json

run-mid360: $(TARGET)
	./$(TARGET) -c $(CONFIG_DIR)/mid360_config.json

clean:
	@echo "Cleaning..."
	rm -rf $(BUILD_DIR)
	@echo "Done."

rebuild: clean all

help:
	@echo ""
	@echo "  Targets:"
	@echo "    all        - Build (default)"
	@echo "    clean      - Remove build directory"
	@echo "    rebuild    - Clean and rebuild"
	@echo "    run        - Build and run with config.json"
	@echo "    run-hap    - Build and run with hap_config.json"
	@echo "    run-mid360 - Build and run with mid360_config.json"
	@echo ""

.PHONY: all clean rebuild run run-hap run-mid360 help