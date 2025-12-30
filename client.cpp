#include <iostream>
#include <string>
#include <stdexcept>
#include "Socket.h"
#include "InetAddress.h"

static const int BUFFER_SIZE = 1024;
static const uint16_t PORT = 5008;
static const char *IP = "127.0.0.1";


int main(){
    try{
        InetAddress server_addr(IP, PORT);
        Socket client_sock;
        if(client_sock.connect(server_addr) == -1){
            return -1;
        }
        std::string buffer;
        
        while(true){
            std::cout << "请输入：";
            getline(std::cin, buffer);
            std::cout << std::endl;
            ssize_t len = ::send(client_sock.fd(), buffer.c_str(), buffer.size(), 0);
            if(len == -1){
                throw std::runtime_error("send error");
                break;
            }
            char buffer2[BUFFER_SIZE];
            len = ::recv(client_sock.fd(), buffer2, sizeof(buffer2), 0);
            if(len == -1){
                throw std::runtime_error("recv error");
                break;
            }
            std::cout << "服务器回应：" << buffer2 << std::endl;
        }
    }catch(const std::exception& e){
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    
    return 0;
}