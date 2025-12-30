#pragma once
#include <arpa/inet.h>
#include <cstring>

class InetAddress
{
public:
    struct sockaddr_in addr_;
    socklen_t addr_len_;
    //默认构造函数
    InetAddress() : addr_len_(sizeof(addr_)){
        memset(&addr_,0,addr_len_);
    }
    //核心构造函数
    explicit InetAddress(const char* ip, uint16_t port) : addr_len_(sizeof(addr_)){
        memset(&addr_,0,addr_len_);             //addr_内存置为零防止垃圾数据干扰
        addr_.sin_family = PF_INET;             //IPV4协议族
        addr_.sin_addr.s_addr = inet_addr(ip);      //字符串格式转成大端序（网络序）
        addr_.sin_port = htons(port);           //大端序（网络序）转成小端序
    }
    //接收构造函数
    explicit InetAddress(const sockaddr_in& addr, socklen_t len) : addr_(addr),addr_len_(len){}
};