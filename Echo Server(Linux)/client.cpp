/**
 * @file client.cpp
 * @brief Echo Client 测试程序
 * 
 * 连接服务器并发送/接收消息。
 */

#include <iostream>
#include <string>
#include <stdexcept>
#include "Socket.h"
#include "InetAddress.h"

static const int BUFFER_SIZE = 1024;

// 默认IP和端口
static const uint16_t DEFAULT_PORT = 5008;
static const char *DEFAULT_IP = "127.0.0.1";

int main(int argc, char *argv[]) {
    // 处理命令行参数
    const char *ip = DEFAULT_IP;
    uint16_t port = DEFAULT_PORT;

    if (argc > 1) {
        ip = argv[1];
    }
    if (argc > 2) {
        port = static_cast<uint16_t>(std::atoi(argv[2]));
    }

    try {
        InetAddress server_addr(ip, port);
        Socket client_sock;
        if (client_sock.connect(server_addr) == -1) {
            return -1;
        }
        std::cout << "Connected to server " << ip << ":" << port << std::endl;
        
        std::string buffer;
        
        while(true){
            std::cout << "请输入：";
            getline(std::cin, buffer);
            if (buffer == "exit") break; // 增加退出机制
            
            std::cout << std::endl;
            ssize_t len = ::send(client_sock.fd(), buffer.c_str(), buffer.size(), 0);
            if(len == -1){
                throw std::runtime_error("send error");
            }
            char buffer2[BUFFER_SIZE];
            len = ::recv(client_sock.fd(), buffer2, sizeof(buffer2), 0);
            if(len == -1){
                throw std::runtime_error("recv error");
            }else if(len == 0){
                std::cout << "Server disconnected" << std::endl;
                break;
            }
            std::cout << "服务器回应：" << buffer2 << std::endl;
        }
    }catch(const std::exception& e){
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    
    return 0;
}