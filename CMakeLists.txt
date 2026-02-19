cmake_minimum_required(VERSION 3.10)
project(EchoServer_Linux)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含当前目录的头文件
include_directories(${CMAKE_SOURCE_DIR}/Echo\ Server(Linux))

# 查找所有源文件
file(GLOB SERVER_SOURCES "Echo Server(Linux)/server.cpp")
file(GLOB CLIENT_SOURCES "Echo Server(Linux)/client.cpp")

# 添加可执行文件
add_executable(server ${SERVER_SOURCES})
add_executable(client ${CLIENT_SOURCES})
