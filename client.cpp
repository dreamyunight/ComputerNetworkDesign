#include "InetAddress.h"
#include "util.h"
#include "Socket.h"
#include "client.h"
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <random>
// #include <random>
#include <cstring>
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

SensorData data;

std::atomic<bool> running(true); // 控制程序运行状态
std::mutex mtx;                     // 互斥锁
std::condition_variable cv;         // 条件变量实现线程通知
bool urgent = false;                // 是否有需要发送的紧急信息
void receiveMessages(int fd);       // 接受信息专用线程
void periodicSender(int fd);        // 定时发送数据线程
void generateSensorData();          // 动态修改线程

std::random_device rd;              // 随机数生成器
std::mt19937 gen(rd());             // 使用 Mersenne Twister 算法
//  随机数范围
std::uniform_int_distribution<> dist0_70(0, 70);// 0-99
std::uniform_int_distribution<> dist0_50(0,40); // 
std::uniform_int_distribution<> dist0_1(0,1);   // 0-1

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

    // 创建定时发送消息线程
    std::thread sender(periodicSender, clnt_sock->getFd());

    // 创建定时生成传感器数据线程
    std::thread generator(generateSensorData);

    // 主线程：发送数据
    while (running) {
        // char buf[BUFFER_SIZE];
        // bzero(buf, sizeof(buf));
        // printf("Message:");
        // scanf("%s", buf);

        // 模拟生成传感器数据
        // generateSensorData();

        // 甲烷超标，立即发送
        if (data.methane > 65) {
            {
                std::lock_guard<std::mutex> lock(mtx);
                urgent = true;
            }
            cv.notify_one();    // 唤醒定时线程
        }
        // 紧急 断电退出
        else if (data.methane > 69) {
            running = false;
            printf("Exiting client...\n");
            cv.notify_one(); // 唤醒定时线程以终止
            break;
        }
        // ssize_t bytes_write = write(clnt_sock->getFd(), buf, sizeof(buf));
        // if (bytes_write == -1) {
        //     printf("Socket already disconnected, can't write any more!\n");
        //     running = false;
        //     break;
        // }
        
    }
    // 等待接收线程结束
    receiver.join();
    sender.join();
    generator.join();

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
// 定时发送数据线程
void periodicSender(int fd) {
    while (running) {
        char buf[BUFFER_SIZE];
        bzero(buf, sizeof(buf));
        std::unique_lock<std::mutex> lock(mtx);
        if (!cv.wait_for(lock, std::chrono::seconds(5),
                         [] { return urgent; })) {
            ssize_t bytes_write = write(fd, buf, sizeof(buf));
            if (bytes_write == -1) {
                errif(bytes_write, "Periodic send error");
                running = false;
                break;
            }
            // 发送的消息
            printf("Periodic message sent:\n");
            //  详细消息：
        } else
        // 紧急唤醒消息
        if (urgent) {
            ssize_t bytes_write = write(fd, buf, sizeof(buf));
            if (bytes_write == -1) {
                errif(bytes_write, "Urgent send error");
                running = false;
                break;
            }
            printf("Urgent message sent:\n");
            urgent = false; // 重置紧急标志
        }
    }
}

void generateSensorData() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1)); // 每秒生成一次数据
        // 更新数据
        {
            std::lock_guard<std::mutex> lock(mtx);
            data.methane        = dist0_70(gen);  // 随机生成甲烷浓度
            data.temperature    = dist0_50(gen);  // 随机生成温度
            data.smokeDetected  = dist0_1(gen);   // 随机生成烟雾状态
            data.powerStatus    = dist0_1(gen);   // 随机生成馈电状态
        }

        printf(
            "methane: %d, temperature:%d, smokeDetected:%d, powerStatus:%d\n",
            data.methane, data.temperature, data.smokeDetected, data.powerStatus);
        
    }
}