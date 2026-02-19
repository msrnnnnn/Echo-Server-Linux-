/**
 * @file InetAddress.h
 * @brief 封装 IPv4 地址操作
 * 
 * 该类封装了 socket 地址结构体 (sockaddr_in)，
 * 提供了方便的接口来处理 IP 地址和端口号。
 */

#pragma once
#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <stdexcept>

class InetAddress
{
private:
    struct sockaddr_in addr_; // 存放ip地址，端口号，网络协议的结构体
    socklen_t addr_len_;      // 结构体大小

public:
    /**
     * @brief 默认构造函数
     * 作为一个空容器，用于 accept 接收客户端 ip 地址，端口号，网络协议的数据
     */
    InetAddress() : addr_len_(sizeof(addr_)){
        memset(&addr_, 0, addr_len_);
    }

    /**
     * @brief 核心构造函数
     * 用于服务器 bind 绑定 ip 地址和端口等信息，或者作为客户端 connect 连接的目标
     * @param ip IP地址字符串 (e.g., "127.0.0.1")
     * @param port 端口号
     */
    explicit InetAddress(const char* ip, uint16_t port) : addr_len_(sizeof(addr_)){
        memset(&addr_, 0, addr_len_);             // 将addr_内存置为零防止随机值引发未定义的行为
        addr_.sin_family = AF_INET;               // IPV4协议族
        // 将点十分制字符串转换成网络字节序(转换的协议，ip，接收转换后ip的字符数组)
        if(inet_pton(AF_INET, ip, &addr_.sin_addr) <= 0){
            throw std::runtime_error("inet_pton error");
        }
        addr_.sin_port = htons(port);             // 将小端序转成大端序（网络序）
    }

    // 获取ip字符串
    std::string getIp() const{
        char buffer[64];
        memset(buffer, 0, sizeof(buffer));
        ::inet_ntop(AF_INET, &addr_.sin_addr, buffer, sizeof(buffer));
        return buffer;
    }

    // 获取端口号
    uint16_t getPort() const{
        return ntohs(addr_.sin_port); // s是16位(short)，l是32位(long)
    }

    // 获取ip:port
    std::string getIpPort() const{
        return getIp() + ":" + std::to_string(getPort()); // 用to_string进行类型强转
    }

    // 提供sockaddr接口给外界访问(只读版本)
    const struct sockaddr* getAddr() const{
        return (const struct sockaddr*)&addr_;
    }

    // 提供sockaddr接口给外界访问
    struct sockaddr* getAddr(){
        return (struct sockaddr*)&addr_;
    }

    // 获取addr_len_
    socklen_t& getLen(){
        return addr_len_;
    }

    // 获取addr_len_(只读版本)
    socklen_t getLen() const{
        return addr_len_;
    }
};