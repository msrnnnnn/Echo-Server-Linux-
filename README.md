# High-Performance Echo Server (Linux)

## 简介 (Introduction)

这是一个基于 C++ 开发的高性能 TCP 回显服务器（Echo Server），专为 Linux 环境设计。它采用了 **Epoll** IO多路复用技术（Edge Triggered 模式）和 **非阻塞 I/O**，能够高效地处理大量并发连接。

本项目对底层的 Socket API 进行了现代 C++ 的封装（RAII 风格），使得代码更加简洁、安全且易于维护。

## 核心特性 (Features)

*   **高并发 (High Concurrency)**: 使用 Linux Epoll (ET模式) 处理并发连接，性能远超传统的 select/poll 模型。
*   **非阻塞 I/O (Non-blocking I/O)**: 所有 socket 操作均设置为非阻塞模式，最大限度提升系统吞吐量。
*   **RAII 资源管理**: 通过 `Socket` 和 `InetAddress` 类封装底层资源，利用析构函数自动释放文件描述符，防止内存泄漏。
*   **清晰的架构**: 将网络底层细节与业务逻辑分离，代码结构清晰。

## 环境要求 (Requirements)

*   **Operating System**: Linux (Kernel 2.6+) 或 Windows Subsystem for Linux (WSL)
*   **Compiler**: g++ (支持 C++11 或更高版本)

## 编译与运行 (Build & Run)

### 1. 编译 (Build)

本项目支持使用 CMake 构建 (推荐)，也可以直接使用 g++ 编译。

#### 方式一：使用 CMake (推荐)

```bash
mkdir build
cd build
cmake ..
make
```

#### 方式二：使用 g++ 直接编译

```bash
cd "Echo Server(Linux)"
g++ server.cpp -o server -std=c++11
g++ client.cpp -o client -std=c++11
```

### 2. 运行 (Run)

首先启动服务器 (默认监听 0.0.0.0:5008)：

```bash
# 默认端口 5008
./server

# 或者指定端口
./server 8080

# 或者指定端口和IP
./server 8080 127.0.0.1
```

然后在一个或多个新的终端窗口中启动客户端：

```bash
# 连接默认地址 127.0.0.1:5008
./client

# 或者连接指定地址
./client 127.0.0.1 8080
```

## 代码结构 (Project Structure)

*   `server.cpp`: 服务器主程序，包含 Epoll 事件循环和连接处理逻辑。
*   `client.cpp`: 客户端程序，用于测试连接和发送消息。
*   `Socket.h`: Socket 资源管理类，封装了 socket、bind、listen、accept、connect 等操作。
*   `InetAddress.h`: IP 地址和端口封装类，处理网络字节序转换。

## 许可证 (License)

本项目采用 MIT 许可证，详情请参阅 [LICENSE](LICENSE) 文件。
