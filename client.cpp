#include "InetAddress.h"
#include "util.h"
#include "Socket.h"
#include <cstdio>
#include <strings.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <cerrno>
#include <thread>
#include <atomic>

#define BUFFER_SIZE 1024

std::atomic<bool> running(true);
void receiveMessages(int fd);
int main() {
    // 创建套接字
    Socket *clnt_sock = new Socket();
    // 设置服务器地址
    InetAddress *clnt_addr = new InetAddress("127.0.0.1", 8888);

    // 连接到服务器
    errif(
        connect(clnt_sock->getFd(), (sockaddr *)&clnt_addr->addr, sizeof(*clnt_addr)),
        "Socket connect error");

    // 创建接收消息线程
    std::thread receiver(receiveMessages,clnt_sock->getFd());

    // 主线程：发送数据
    while (running) {
        char buf[BUFFER_SIZE];
        bzero(buf, sizeof(buf));
        printf("Message:");
        scanf("%s", buf);

        ssize_t bytes_write = write(clnt_sock->getFd(), buf, sizeof(buf));
        if (bytes_write == -1) {
            printf("Socket already disconnected, can't write any more!\n");
            running = false;
            break;
        }

        // 输入 exit 退出
        if (strcmp(buf, "exit") == 0) {
            printf("Exiting client...\n");
            running = false;
            break;
        }
    }
    // 等待接收线程结束
    receiver.join();

    close(clnt_sock->getFd());
    return 0;
}
// 子线程：接收数据
void receiveMessages(int fd) {
    char buf[BUFFER_SIZE];
    while (running) {
        bzero(buf, sizeof(buf));
        ssize_t bytes_read = read(fd, buf, sizeof(buf));
        if (bytes_read > 0) {
            printf("Message from server: %s\n", buf);

            // 解析服务器消息
            if (strcmp(buf, "exit") == 0) {  // 服务器指令：退出
                printf("Server requested to close connection.\n");
                running = false;  // 停止客户端
                break;
            }
        } else
        // EOF：停止客户端
        if (bytes_read == 0) {
            printf("Server disconnected\n");
            running = false;
            break;
        } else
        // 连接失败：停止客户端
        if (bytes_read == -1) {
            errif(bytes_read, "Client read error");
            running = false;
            break;
        }
    }
}