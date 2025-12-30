#pragma once
#include <sys/socket.h>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>
#include "InetAddress.h"

class Socket
{
private:
    int fd_;//文件描述符

public:
    Socket(){
        if((fd_ = socket(AF_INET,SOCK_STREAM,0)) == -1){
            throw std::runtime_error("Create socket error");
        }
    }

    explicit Socket(int fd) : fd_(fd){
        if(fd == -1) throw std::runtime_error("Invalid fd");
    }

    ~Socket(){
        if(fd_ != -1) close(fd_);
    }

    //禁用拷贝，只能移动
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket &&other)noexcept : fd_(other.fd_){other.fd_ = -1;}

    void bind(const InetAddress& addr){
        //端口复用
        int opt = 1;
        if(setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1){
            throw std::runtime_error("setsockopt SO_REUSEADDR failed");
        }
        //绑定ip地址和端口号
        if(::bind(fd_, (sockaddr*)&addr.addr_, addr.addr_len_) == -1){
            throw std::runtime_error("Bind error");
        }
    }

    void listen(){
        if(::listen(fd_, SOMAXCONN) == -1){
            throw std::runtime_error("listen error");
        }
    }

    int accept(InetAddress&client_addr){
        int client_fd = ::accept(fd_, (sockaddr*)&client_addr.addr_, &client_addr.addr_len_);
        return client_fd;
    }

    //设置非阻塞
    void setNonBlocking() {
        int flags = fcntl(fd_, F_GETFL);
        flags |= O_NONBLOCK;
        fcntl(fd_, F_SETFL, flags);
    }

    int fd(){
        return fd_;
    }
};