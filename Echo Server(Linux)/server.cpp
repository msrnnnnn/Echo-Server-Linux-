/**
 * @file server.cpp
 * @brief Epoll Echo Server 主程序
 *
 * 使用 Epoll (ET模式) 实现的高并发 Echo Server。
 */

#include "InetAddress.h"
#include "Socket.h"
#include <cerrno>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

static const int MAX_EVENTS = 1024;
static const int BUFFER_SIZE = 1024;

// 默认IP和端口
static const uint16_t DEFAULT_PORT = 5008;
static const char *DEFAULT_IP = "0.0.0.0";

// 全局标志位，用于控制服务器运行状态
volatile std::sig_atomic_t stop_server = 0;

// 信号处理函数
void handle_signal(int sig)
{
    if (sig == SIGINT || sig == SIGTERM)
    {
        stop_server = 1;
    }
}

// 处理新连接
void handleNewConnection(Socket &listen_sock, int epfd)
{
    InetAddress client_addr;
    int client_fd = listen_sock.accept(client_addr);
    if (client_fd == -1)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            perror("accept error");
        }
        return;
    }

    // 设置非阻塞
    Socket::setNonBlocking(client_fd);

    // 注册到 epoll (ET模式)
    struct epoll_event client_ev;
    client_ev.data.fd = client_fd;
    client_ev.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &client_ev) == -1)
    {
        perror("epoll_ctl: client");
        close(client_fd);
        return;
    }

    std::cout << "New client: " << client_fd << " (" << client_addr.getIpPort() << ")" << std::endl;
}

// 处理客户端消息
void handleClientMessage(int client_fd, int epfd)
{
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    while (true)
    {
        int len = ::recv(client_fd, buffer, sizeof(buffer), 0);
        if (len > 0)
        {
            std::cout << "服务器接收：" << buffer << std::endl;
            ::send(client_fd, buffer, len, MSG_NOSIGNAL);
            memset(buffer, 0, sizeof(buffer));
        }
        else if (len == 0)
        {
            std::cout << "Client disconnected: " << client_fd << std::endl;
            epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, nullptr);
            ::close(client_fd);
            break;
        }
        else
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // 数据读取完毕
                break;
            }
            else
            {
                perror("recv error");
                epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, nullptr);
                ::close(client_fd);
                break;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    // 注册信号处理
    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);

    // 处理命令行参数
    uint16_t port = DEFAULT_PORT;
    const char *ip = DEFAULT_IP;

    if (argc > 1)
    {
        port = static_cast<uint16_t>(std::atoi(argv[1]));
    }
    if (argc > 2)
    {
        ip = argv[2];
    }

    try
    {
        Socket listen_sock;
        InetAddress server_addr(ip, port);
        listen_sock.bind(server_addr);
        listen_sock.listen();
        listen_sock.setNonBlocking();

        int epfd = epoll_create1(EPOLL_CLOEXEC);
        if (epfd == -1)
        {
            perror("epoll_create1");
            return -1;
        }

        struct epoll_event ev;
        ev.data.fd = listen_sock.fd();
        ev.events = EPOLLIN | EPOLLET; // 监听 socket 也可以设为 ET
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, listen_sock.fd(), &ev) == -1)
        {
            perror("epoll_ctl: serv_sock");
            return -1;
        }

        std::cout << "v3 Epoll Server running on " << ip << ":" << port << " ..." << std::endl;

        std::vector<epoll_event> events(MAX_EVENTS);

        while (!stop_server)
        {
            int nfds = epoll_wait(epfd, events.data(), MAX_EVENTS, -1);

            if (nfds == -1)
            {
                if (errno == EINTR)
                    continue; // 被信号中断，继续循环检查 stop_server
                perror("epoll_wait");
                break;
            }

            for (int i = 0; i < nfds; ++i)
            {
                if (events[i].data.fd == listen_sock.fd())
                {
                    handleNewConnection(listen_sock, epfd);
                }
                else if (events[i].events & EPOLLIN)
                {
                    handleClientMessage(events[i].data.fd, epfd);
                }
            }
        }

        std::cout << "Server shutting down..." << std::endl;
        ::close(epfd);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Server Exception: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}

#if 0
//多线程版本
#include "InetAddress.h"
#include "Socket.h"
#include <iostream>
#include <thread>

void communication(Socket socket){
    char buffer[1024];
    while(true){
        if(recv(socket.fd(), &buffer, sizeof(buffer), 0) > 0){
            send(socket.fd(), &buffer, sizeof(buffer), MSG_NOSIGNAL);
            memset(&buffer, 0, sizeof(buffer));
        }else{
            std::cerr << "recv error or disconnect";
            break;
        }
    }
}

int main(){
    Socket sock;
    InetAddress addr("0.0.0.0", 5008);
    sock.bind(addr);
    sock.listen();

    while(true){
        InetAddress addr_client;
        int fd_client = sock.accept(addr_client);
        if(fd_client == -1){
            continue;
        }

        Socket socket_client(fd_client);
        std::thread t(communication, std::move(socket_client));
        t.detach();
    }

    return 0;
}
#endif