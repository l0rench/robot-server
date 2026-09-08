# ============================================================
# Makefile for Orbbec + GStreamer Video Streaming
# ============================================================

# 编译器
CXX = g++

# 编译选项
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra

# 调试选项（取消注释可开启调试）
# CXXFLAGS += -g -DDEBUG

# 目标程序
TARGET = stream_gst

# 源文件
SRC = 4vedio_processiong.cpp

# 头文件路径
INCLUDES = \
    -I/usr/local/include \
    -I/home/cat/Orbbec/include/libobsensor/h

# 链接库
LIBS = \
    -lOrbbecSDK \
    -lpthread \
    -ldl \
    -lrt \
    -lm

# 使用 pkg-config 获取 GStreamer 和 OpenCV 的编译参数
PKG_DEPS = gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0 opencv4

# 获取 pkg-config 参数
PKG_CFLAGS = $(shell pkg-config --cflags $(PKG_DEPS) 2>/dev/null)
PKG_LIBS = $(shell pkg-config --libs $(PKG_DEPS) 2>/dev/null)

# 如果 pkg-config 失败，使用默认路径
ifeq ($(PKG_CFLAGS),)
    PKG_CFLAGS = -I/usr/include/gstreamer-1.0 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include
    PKG_LIBS = -lgstreamer-1.0 -lgstapp-1.0 -lgstvideo-1.0 -lgobject-2.0 -lglib-2.0 -lopencv_core -lopencv_imgproc -lopencv_imgcodecs
endif

# 完整编译参数
CFLAGS = $(CXXFLAGS) $(PKG_CFLAGS) $(INCLUDES)
LDFLAGS = $(PKG_LIBS) $(LIBS)

# ============================================================
# 编译规则
# ============================================================

# 默认目标
all: $(TARGET)

# 链接
$(TARGET): $(SRC)
	@echo ""
	@echo "========================================"
	@echo "  Compiling $(TARGET)..."
	@echo "========================================"
	@echo ""
	$(CXX) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo ""
	@echo "========================================"
	@echo "  Build successful! ✓"
	@echo "  Run: ./$(TARGET)"
	@echo "========================================"
	@echo ""

# ============================================================
# 辅助目标
# ============================================================

# 清理
clean:
	@echo "Cleaning..."
	rm -f $(TARGET)
	rm -f *.o
	rm -f timestamp_sender.csv
	@echo "Done."

# 清理并重新编译
rebuild: clean all

# 显示编译参数（调试用）
show:
	@echo "CXX:       $(CXX)"
	@echo "CFLAGS:    $(CFLAGS)"
	@echo "LDFLAGS:   $(LDFLAGS)"
	@echo "PKG_DEPS:  $(PKG_DEPS)"

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
	@echo "  Makefile for Orbbec Stream Sender"
	@echo "========================================"
	@echo ""
	@echo "  Targets:"
	@echo "    all      - Build $(TARGET) (default)"
	@echo "    clean    - Remove compiled files"
	@echo "    rebuild  - Clean and rebuild"
	@echo "    show     - Show compile parameters"
	@echo "    install  - Install to /usr/local/bin"
	@echo "    uninstall- Remove from /usr/local/bin"
	@echo "    help     - Show this help"
	@echo ""

# ============================================================
# 伪目标
# ============================================================

.PHONY: all clean rebuild show install uninstall help