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
        if((fd_ = socket(AF_INET,SOCK_STREAM,0)) == -1){        //第三个参数是协议，填0就行，会根据前两个参数推导出第三个参数的协议
            throw std::runtime_error("Create socket error");
        }
    }

    ~Socket(){
        if(fd_ != -1) close(fd_);
    }

    //禁用拷贝，只能移动
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket &&other)noexcept : fd_(other.fd_){other.fd_ = -1;}

    void bind(const InetAddress& server_addr){
        //端口复用
        int opt = 1;
        if(setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1){
            throw std::runtime_error("setsockopt SO_REUSEADDR failed");
        }
        //绑定ip地址和端口号
        if(::bind(fd_, server_addr.getAddr(), server_addr.getLen()) == -1){
            throw std::runtime_error("Bind error");
        }
    }

    void listen(){
        if(::listen(fd_, SOMAXCONN) == -1){
            throw std::runtime_error("listen error");
        }
    }

    int accept(InetAddress&client_addr){
        int client_fd = ::accept(fd_, client_addr.getAddr(), &client_addr.getLen());
        return client_fd;
    }

    int connect(InetAddress&server_addr){
        int server_fd = ::connect(fd_, server_addr.getAddr(), server_addr.getLen());
        if(server_fd == -1){
            throw std::runtime_error("connect error");
        }
        return server_fd;
    }

    //设置非阻塞
    void setNonBlocking() {
        int flags = fcntl(fd_, F_GETFL);
        fcntl(fd_, F_SETFL, flags | O_NONBLOCK);
    }

    int fd(){
        return fd_;
    }
};