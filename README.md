# High-Performance Echo Server (Linux) (基于Epoll的高性能回声服务器)

![Language](https://img.shields.io/badge/language-C%2B%2B11-blue.svg)
![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

> **Note**: 本项目仅作为 **学习 Demo**，旨在深入理解 Linux 高并发网络编程核心概念（Epoll, 非阻塞I/O, Reactor模型等），不建议直接用于生产环境。

## 📖 项目简介 (Introduction)

这是一个基于 **C++11** 标准和 **Epoll (Edge Triggered)** 模式开发的高性能 TCP Echo Server。

在 Linux 网络编程中，**Epoll** 是实现高并发的关键技术。本项目通过实现一个精简的 Echo Server，展示了如何使用 Epoll 的边缘触发 (ET) 模式配合非阻塞 I/O (Non-blocking I/O) 来处理大量并发连接。

本项目采用了 **RAII机制** 封装 Socket 资源，代码结构清晰，适合作为学习 Linux 网络编程的入门参考。

## ✨ 核心特性 (Key Features)

*   **⚡ 高性能 I/O 多路复用**：
    *   使用 **Epoll** 的 **ET (Edge Triggered)** 边缘触发模式，减少 epoll_wait 系统调用的返回次数，提高并发处理效率。
    *   相比传统的 Select/Poll 机制，Epoll 在处理海量并发连接时具有显著的性能优势。

*   **🚀 非阻塞 I/O (Non-blocking I/O)**：
    *   所有 Socket 均设置为非阻塞模式。
    *   在 ET 模式下，采用 `while(true)` 循环读取数据，确保缓冲区数据被一次性读空，避免数据丢失或处理延迟。

*   **🛡️ 智能资源管理 (RAII)**：
    *   封装 `Socket` 类和 `InetAddress` 类，利用 C++ 析构函数自动关闭文件描述符 (File Descriptor)。
    *   有效防止资源泄露，简化了错误处理逻辑。

*   **⚙️ 健壮性设计**：
    *   **优雅退出**：通过捕获 `SIGINT` 和 `SIGTERM` 信号，确保服务器在终止时能正确关闭资源。
    *   **错误处理**：完善的错误检查机制，针对 `EAGAIN` / `EWOULDBLOCK` 等 errno 进行特殊处理。
    *   **参数配置**：支持通过命令行参数动态指定监听端口和 IP 地址。

*   **🛠️ 标准化构建**：
    *   使用 **CMake** 构建系统，支持一键编译，方便移植和集成。

## 🛠️ 技术栈 (Tech Stack)

*   **开发语言**：C++11 (使用了 `std::vector`, `std::string`, `try-catch` 异常处理等特性)
*   **网络模型**：Epoll (ET Mode) + Non-blocking I/O
*   **构建工具**：CMake
*   **开发环境**：Linux / WSL2

## 🚀 快速开始 (Getting Started)

### 1. 环境准备

*   Linux 操作系统 或 Windows Subsystem for Linux (WSL2)
*   C++ 编译器 (GCC/G++)
*   CMake 3.10+

### 2. 编译项目

```bash
# 克隆仓库
git clone https://github.com/msrnnnnn/Echo-Server-Linux-.git
cd Echo-Server-Linux-

# 创建构建目录
mkdir build
cd build

# 生成 Makefile
cmake ..

# 编译
make
```

### 3. 运行服务器

```bash
# 默认监听 0.0.0.0:5008
./EchoServer

# 或者指定端口和IP
# 用法: ./EchoServer [port] [ip]
./EchoServer 8888 127.0.0.1
```

### 4. 运行客户端 (测试)

项目包含一个简单的测试客户端，用于验证服务器功能。

```bash
# 在另一个终端运行
# 用法: ./Client [port] [ip]
./Client 8888 127.0.0.1
```

## 📂 项目结构

```text
.
├── CMakeLists.txt          # CMake 构建脚本
├── README.md               # 项目文档
├── LICENSE                 # MIT 许可证
├── Echo Server(Linux)/     # 源代码目录
│   ├── server.cpp          # 服务器主程序 (Epoll ET Loop)
│   ├── client.cpp          # 客户端主程序
│   ├── Socket.h/cpp        # Socket RAII 封装
│   └── InetAddress.h/cpp   # 地址信息封装
└── .gitignore              # Git 忽略文件配置
```

## 📄 许可证 (License)

本项目采用 [MIT License](LICENSE) 许可证。
