#include <cstdint>
#include <strings.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include "Epoll.h"
#include "util.h"

#define MAX_EVENTS 1000

Epoll::Epoll() : epfd(-1), events(nullptr) {
    epfd = epoll_create1(0);
    errif(epfd, "Epoll create error");
    events = new epoll_event[MAX_EVENTS]; // 等价于：struct epoll_event events[MAX_EVENTS]
    bzero(events, sizeof(epoll_event) * MAX_EVENTS);
}

Epoll::~Epoll() {
    if (epfd != -1) {
        close(epfd);
        epfd = -1;
    }
    delete[] events;
}

void Epoll::addFd(int dataFd, uint32_t eventOp) {
    // ev 配置单个文件描述符的监听事件。
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));

    ev.data.fd  = dataFd;   // 将 sockfd（服务器监听套接字）绑定到 ev
    ev.events   = eventOp;  // 配置触发模式

    errif(epoll_ctl(epfd, EPOLL_CTL_ADD, dataFd, &ev), "Epoll add error");
}

void Epoll::modFd(int dataFd, uint32_t eventOp) {
    // ev 配置单个文件描述符的监听事件。
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));

    ev.data.fd  = dataFd;   // 将 sockfd（服务器监听套接字）绑定到 ev
    ev.events   = eventOp;  // 配置触发模式

    errif(epoll_ctl(epfd, EPOLL_CTL_MOD, dataFd, &ev), "Epoll mod error");
}

std::vector<epoll_event> Epoll::pull(int timeout) {
    std::vector<epoll_event> activeEvents;
    int nfds = epoll_wait(epfd, events, MAX_EVENTS, timeout);
    errif(nfds, "Epoll wait error");
    for (int i = 0; i < nfds; i++) {
        activeEvents.push_back(events[i]);
    };
    return activeEvents;
}