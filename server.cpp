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
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }

int main(){
    Socket listen_sock;
    InetAddress server_addr(IP, PORT);
    listen_sock.bind(server_addr);
    listen_sock.listen();
    listen_sock.setNonBlocking();

    int epfd = epoll_create1(EPOLL_CLOEXEC);
    if (epfd == -1){
        perror("epoll_create1");
        return -1;
    }

    struct epoll_event ev;
    ev.data.fd = listen_sock.fd();
    ev.events = EPOLLIN;
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, listen_sock.fd(), &ev) == -1){
        perror("epoll_ctl: serv_sock");
        return -1;
    }

    std::cout << "v3 Epoll Server running..." << std::endl;

    std::vector<epoll_event> events(MAX_EVENTS);

    while(true){
        int nfds = epoll_wait(epfd, events.data(), MAX_EVENTS, -1);
        if(nfds == -1){
            perror("epoll_wait");
            break;
        }else if(nfds == 0) continue;
        for(int i = 0;i < nfds; ++i){
            if(events[i].data.fd == listen_sock.fd()) {
                InetAddress client_addr;
                int client_fd = listen_sock.accept(client_addr);
                if(client_fd == -1){
                    perror("accept error");
                    break;
                }
                setNonBlocking(client_fd);
                struct epoll_event client_ev;
                client_ev.data.fd = client_fd;
                client_ev.events = EPOLLIN | EPOLLET;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &client_ev);
                std::cout << "New client: " << client_fd << std::endl;
            }else if(events[i].events & EPOLLIN){
                char buffer[BUFFER_SIZE];
                while(true){
                    int len = recv(events[i].data.fd, buffer, sizeof(buffer), 0);
                    if(len > 0){
                    send(events[i].data.fd, buffer, len, MSG_NOSIGNAL);
                    memset(buffer, 0, sizeof(buffer));
                }else if(len == 0){
                    std::cerr << "recv disconnect" << events[i].data.fd;
                    epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
                    close(events[i].data.fd);
                    break;
                }else{
                    // len == -1
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        // 数据全部读完，缓冲区空了，退出循环等待下次事件
                        break; 
                    } else {
                        // 发生真正的错误
                        perror("read");
                        epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
                        close(events[i].data.fd);
                         break;
                        }
                    }
                }
            }
        }
    }
    close(epfd);
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