#include <iostream> 
#include <vector>
#include <sys/epoll.h>
#include <fcntl.h>      
#include <unistd.h>     
#include <cstring>      
#include <cerrno>       
#include <sys/socket.h>
#include "Socket.h"
#include "InetAddress.h"

static const int MAX_EVENTS = 1024;
static const int BUFFER_SIZE = 1024;
static const uint16_t PORT = 5008;
static const char *IP = "0.0.0.0";

void setNonBlocking(int fd) {
        int flags = fcntl(fd, F_GETFL);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);//file cntrl (fcntl())
    }

int main(){
    Socket listen_sock;
    InetAddress server_addr(IP, PORT);
    listen_sock.bind(server_addr);
    listen_sock.listen();
    listen_sock.setNonBlocking();//监听fd设为非阻塞的作用是让accept实现非阻塞

    int epfd = epoll_create1(EPOLL_CLOEXEC);//创建epoll实例，基本都是这样的写法(EPOLL_CLOEXEC)
    if (epfd == -1){
        perror("epoll_create1");
        return -1;
    }

    struct epoll_event ev;//epoll事件描述结构体
    ev.data.fd = listen_sock.fd();
    ev.events = EPOLLIN;//监听可读，LT水平。监听fd通常用水平触发
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, listen_sock.fd(), &ev) == -1){//将ev事件注册到epoll实例中
        perror("epoll_ctl: serv_sock");
        return -1;
    }

    std::cout << "v3 Epoll Server running..." << std::endl;

    std::vector<epoll_event> events(MAX_EVENTS);//用来存放返回的事件

    while(true){
        int nfds = epoll_wait(epfd, events.data(), MAX_EVENTS, -1);//(epoll实例，接收返回事件容器的首地址，数组大小，-1是永久阻塞，0是非阻塞，大于0是等待时间（毫秒）)返回的是有多少个事件写入了数组
        //返回0是超时，-1是出错
        if(nfds == -1){             
            perror("epoll_wait");
            break;
        }else if(nfds == 0) continue;
        //遍历处理每一个事件
        for(int i = 0;i < nfds; ++i){
            //如果是监听fd有事件说明连接队列里有连接可以accept
            if(events[i].data.fd == listen_sock.fd()) {
                InetAddress client_addr;//用来接收accept填充的客户端地址和端口号
                int client_fd = listen_sock.accept(client_addr);
                if(client_fd == -1){
                    perror("accept error");
                    continue;
                }

                //将与客户端通讯的fd设为非阻塞
                setNonBlocking(client_fd);

                //将客户端通讯的fd注册到epoll实例中监管
                struct epoll_event client_ev;
                client_ev.data.fd = client_fd;
                client_ev.events = EPOLLIN | EPOLLET;//通讯的fd通常要设为ET(边缘触发)
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &client_ev);

                std::cout << "New client: " << client_fd << std::endl;
            }
            //如果是处理通讯的fd说明缓冲区有数据可以读取
            else if(events[i].events & EPOLLIN){
                char buffer[BUFFER_SIZE];//接收缓冲区
                memset(buffer, 0, sizeof(buffer));
                while(true){
                    int len = ::recv(events[i].data.fd, buffer, sizeof(buffer), 0);//第二个参数不能传&buffer，这个是指向整个数组的指针，最后一个参数一般填0，返回读取数据的长度
                    if(len > 0){
                    std::cout << "服务器接收：" << buffer << std::endl;
                    ::send(events[i].data.fd, buffer, len, MSG_NOSIGNAL);//MSG_NOSIGNAL的作用是防止服务器程序因为客户端突然断开连接而被操作系统杀死(no signal)
                    memset(buffer, 0, sizeof(buffer));
                    }
                    else if(len == 0){//等于0表示连接断开了
                    std::cerr << "recv disconnect" << events[i].data.fd;
                    epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);//EPOLL_CTL_DEL不需要考虑最后一个参数
                    ::close(events[i].data.fd);
                    break;
                    }
                    else{
                    // len == -1
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        // 数据全部读完，缓冲区空了，退出循环等待下次事件
                        break; 
                    }
                    else{
                        // 发生真正的错误
                        perror("read");
                        epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
                        ::close(events[i].data.fd);
                         break;
                        }
                    }
                }
            }
        }
    }
    ::close(epfd);//释放epoll实例防止内存泄露
    return 0;
}


#if 0
//多线程版本
#include <iostream>
#include <thread>
#include "Socket.h"
#include "InetAddress.h"

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