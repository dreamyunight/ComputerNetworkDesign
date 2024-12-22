#pragma once
#include <cstdint>
#include <sys/epoll.h>
#include <vector>

class Epoll {
private:
    int epfd;
    struct epoll_event *events;

public:
    Epoll();
    ~Epoll();

    // fd：服务器监听套接字，即server中的socket中的fd
    // op：模式
    void addFd(int dataFd, uint32_t eventOp);
    void modFd(int dataFd, uint32_t eventOp);
    std::vector<epoll_event> pull(int timeout = -1);
};