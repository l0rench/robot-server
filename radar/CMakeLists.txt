cmake_minimum_required(VERSION 3.10)
project(livox_lidar_quick_start)

set(CMAKE_CXX_STANDARD 14)

set(DEMO_NAME livox_lidar_quick_start)

# 添加源文件（包含 synchro.cpp）
add_executable(${DEMO_NAME} 
    main.cpp 
    synchro.cpp  # ← 必须添加这个
)

# 添加头文件搜索路径
target_include_directories(${DEMO_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接库
target_link_libraries(${DEMO_NAME}
    PUBLIC
    livox_lidar_sdk_static
    pthread
    dl
    rt
)
