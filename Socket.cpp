#include "Socket.h"
#include "InetAddress.h"
#include "util.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

Socket::Socket() : fd(-1) {
    fd = socket(AF_INET, SOCK_STREAM, 0);
    errif(fd, "Socket create error");
}
Socket::Socket(int fd) : fd(fd) {
    errif(fd, "Socket create error");
}
Socket::~Socket() {
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
}

void Socket::bind(InetAddress *addr) {
    //::bind明确调用全局作用域中的bind函数
    errif(::bind(fd, (sockaddr *)&addr->addr, addr->addr_len), "Socket bind error");
}
void Socket::listen() {
    errif(::listen(fd, SOMAXCONN),"Socket listen error");
}
int Socket::accept(InetAddress *addr) {
    int clnt_sockfd = ::accept(fd, (sockaddr *)&addr->addr, &addr->addr_len);
    errif(clnt_sockfd, "Socket accept error");
    return clnt_sockfd;
}
int Socket::getFd() {
    return fd;
}
