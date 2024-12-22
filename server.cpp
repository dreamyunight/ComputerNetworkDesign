#include "server.h"
#include "InetAddress.h"
#include "util.h"
#include "Socket.h"
#include "Epoll.h"
#include <queue>
#include <map>
#include <cstdio>
#include <strings.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <cerrno>

#define MAX_EVENTS 1024
#define READ_BUFFER 1024

// int对应需要需要回信的客户端，使用队列存储需要回应的信息
std::map<int, std::queue<std::string>> client_buffers;
void handReadEvent(int sockfd, Epoll *ep);
void handWriteEvent(int clntfd, Epoll *ep);
int main() {    
    Socket *serv_sock = new Socket();
    InetAddress *serv_addr = new InetAddress("127.0.0.1", 8888);
    serv_sock->bind(serv_addr);
    serv_sock->listen();

    Epoll *ep = new Epoll();
    ep->addFd(serv_sock->getFd(), EPOLLIN | EPOLLOUT);
    
    while (true) {
        auto events = ep->pull();
        int nfds = events.size();
        for (int i = 0; i < nfds; i++) {
            // 监听套接字触发事件，处理新客户端连接
            if (events[i].data.fd == serv_sock->getFd()) {
                InetAddress *clnt_addr = new InetAddress();
                Socket *clnt_sock = new Socket(serv_sock->accept(clnt_addr));
                printf("new client fd %d! IP: %s Port: %d\n", clnt_sock->getFd(), inet_ntoa(clnt_addr->addr.sin_addr), ntohs(clnt_addr->addr.sin_port));
                
                ep->addFd(clnt_sock->getFd(), EPOLLIN | EPOLLOUT);
            }else
            // 某客户端有可读事件，处理数据
            if (events[i].events & EPOLLIN) {
                handReadEvent(events[i].data.fd, ep);
            } else
            // 某客户端有可写事件，发送数据
            if (events[i].events & EPOLLOUT) {
                handWriteEvent(events[i].data.fd, ep);
            } else {
            // 其他事件，暂未处理
                printf("Something else happened\n");
            }
        }
    }

    delete serv_sock;
    delete serv_addr;
    delete ep;
    return 0;
}
void handReadEvent(int sockfd, Epoll* ep) {
    char buf[READ_BUFFER];
    bzero(&buf, sizeof(buf));
    ssize_t bytes_read = read(sockfd, buf, sizeof(buf));

    if (bytes_read > 0) {
        // 打印接受到的信息
        printf("message from client fd %d: %s\n", sockfd, buf);

        // 生成确认信息
        std::string confirmation = "Received:"; // 确认信息
        confirmation += buf;// 添加确认信息的内容

        // 生成操作指令
        std::string instruction = "Server command: Perform Action 1";
        client_buffers[sockfd].push(confirmation);
        client_buffers[sockfd].push(instruction);
        
        ep->modFd(sockfd, EPOLLOUT);
    } else
    // EOF
    if (bytes_read == 0) {
        printf("EOF, client fd %d disconnected\n", sockfd);
        close(sockfd);

        // 将对应客户端清出列表
        client_buffers.erase(sockfd);
    } else
    // 读取出错
    if (bytes_read == -1) {
        errif(bytes_read, "Event read error");
    }
}

void handWriteEvent(int clntfd, Epoll *ep) {
    if (!client_buffers[clntfd].empty()) {
        std::string data = client_buffers[clntfd].front();
        client_buffers[clntfd].pop();
        ssize_t bytes_write = write(clntfd, data.c_str(), data.size());
        errif(bytes_write, "Event write error");
        printf("Sent to client fd %d: %s\n", clntfd, data.c_str());
    }
    if (client_buffers[clntfd].empty()) {
        ep->modFd(clntfd, EPOLLIN);// 修改只监听可读事件
    }
}


// Server::Server(int fd) {
//     sock = new Socket(fd);
//     addr = new InetAddress();
// }
// Server::Server(const char *ip, uint16_t port) {
//     sock = new Socket();
//     addr = new InetAddress(ip, port);
//     sock->bind(addr);
//     sock->listen();
// }
// Server::~Server() {}

// Socket *Server::getSocket() { return sock; }
// InetAddress *Server::getAddr() { return addr; }
