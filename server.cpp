#include "server.h"
#include "client.h"
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

    close(serv_sock->getFd());
    delete serv_sock;
    delete serv_addr;
    delete ep;
    return 0;
}

bool verifyChecksum(const char* buffer) {
    // 查找字符串中最后一个逗号后的检验和部分
    const char* checksumPos = std::strrchr(buffer, ',');
    if (!checksumPos) {
        return false;  // 找不到检验和部分
    }

    // 提取检验和部分（十六进制）
    unsigned char receivedChecksum;
    if (sscanf(checksumPos + 1, "%02hhX", &receivedChecksum) != 1) {
        return false;  // 提取检验和失败
    }

    // 计算数据部分的检验和（不包括检验和本身）
    unsigned char calculatedChecksum = 0;
    for (const char* p = buffer; p < checksumPos; ++p) {
        calculatedChecksum += *p;
    }
    calculatedChecksum = calculatedChecksum % 256;  // 取模256

    // 比较计算得到的检验和和接收到的检验和
        return calculatedChecksum == receivedChecksum;
}

void handReadEvent(int sockfd, Epoll* ep) {
    char buf[READ_BUFFER];
    bzero(&buf, sizeof(buf));
    ssize_t bytes_read = read(sockfd, buf, sizeof(buf));

    if (bytes_read > 0) {
        // 打印接受到的信息
        printf("message from client fd %d: %s\n", sockfd, buf);

        bool judge = verifyChecksum(buf);
        
        // 生成确认信息
        std::string confirmation = "Received:"; // 确认信息
        confirmation += buf;                    // 添加确认信息的内容
        confirmation += "\n";

        SensorData data;
        bool extract = (std::sscanf(buf, "%02d,%02d,%01d,%01d,%01d,%01d,%01d",
                           &data.methane, &data.temperature, &data.smokeDetected,
                           &data.powerStatus, &data.mainFanStatus, &data.backupFanStatus,
                           &data.buzzersStatus) == 7);  // 确保成功提取了7个数据项

        // 生成操作指令
        if (data.methane > 30 || data.temperature > 30) {
            if (data.mainFanStatus == 0) {
                data.mainFanStatus = 1;
            }
            if (data.mainFanStatus == 1) {
                data.backupFanStatus = 0;
            }
            if (data.methane > 65) {
                data.powerStatus = 0;
            }
        }
        if (data.powerStatus == 0) {
            data.buzzersStatus = 1;
        }

        bzero(&buf, sizeof(buf));
        std::snprintf(buf, sizeof(buf), "%01d,%01d,%01d,%01d",
                      data.powerStatus,data.mainFanStatus, data.backupFanStatus, data.buzzersStatus);
        std::string instruction = buf;
        
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
