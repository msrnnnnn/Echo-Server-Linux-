/**
 * @file Socket.h
 * @brief Socket 资源管理类 (RAII)
 * 
 * 封装了 socket 文件描述符，利用析构函数自动关闭 socket，
 * 并提供了 bind, listen, accept, connect 等常用操作。
 */

#pragma once
#include <sys/socket.h>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>
#include "InetAddress.h"

class Socket
{
private:
    int fd_; // 文件描述符

public:
    /**
     * @brief 默认构造函数
     * 创建一个 TCP socket
     */
    Socket(){
        if((fd_ = socket(AF_INET, SOCK_STREAM, 0)) == -1){        // 第三个参数是协议，填0就行，会根据前两个参数推导出第三个参数的协议
            throw std::runtime_error("Create socket error");
        }
    }

    /**
     * @brief 析构函数
     * 自动关闭 socket 文件描述符
     */
    ~Socket(){
        if(fd_ != -1) close(fd_);
    }

    // 禁用拷贝，只能移动
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket &&other)noexcept : fd_(other.fd_){other.fd_ = -1;}

    /**
     * @brief 绑定地址
     */
    void bind(const InetAddress& server_addr){
        // 端口复用
        int opt = 1;
        if(setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1){
            throw std::runtime_error("setsockopt SO_REUSEADDR failed");
        }
        // 绑定ip地址和端口号
        if(::bind(fd_, server_addr.getAddr(), server_addr.getLen()) == -1){
            throw std::runtime_error("Bind error");
        }
    }

    /**
     * @brief 开始监听
     */
    void listen(){
        if(::listen(fd_, SOMAXCONN) == -1){
            throw std::runtime_error("listen error");
        }
    }

    /**
     * @brief 接受连接
     */
    int accept(InetAddress& client_addr){
        int client_fd = ::accept(fd_, client_addr.getAddr(), &client_addr.getLen());
        return client_fd;
    }

    /**
     * @brief 发起连接
     */
    int connect(InetAddress& server_addr){
        int server_fd = ::connect(fd_, server_addr.getAddr(), server_addr.getLen());
        if(server_fd == -1){
            throw std::runtime_error("connect error");
        }
        return server_fd;
    }

    // 设置非阻塞
    void setNonBlocking() {
        setNonBlocking(fd_);
    }

    /**
     * @brief 设置文件描述符为非阻塞模式 (静态方法)
     * @param fd 文件描述符
     */
    static void setNonBlocking(int fd) {
        int flags = fcntl(fd, F_GETFL);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }

    int fd(){
        return fd_;
    }
};