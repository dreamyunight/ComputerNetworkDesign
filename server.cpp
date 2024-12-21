#include "server.h"
#include "InetAddress.h"
#include "util.h"
#include "Socket.h"
#include "Epoll.h"
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>

#define MAX_EVENTS 1024
#define READ_BUFFER 1024

int main() {
    Server *serv = new Server("127.0.0.1", 8888);
    Epoll *ep = new Epoll();
    ep->addFd(serv->getSocket()->getFd(), EPOLLIN | EPOLLOUT);

    while (true) {
        auto events = ep->pull();
        int ndfs = events.size();
        
    }
}


Server::Server() {}
Server::Server(const char *ip, uint16_t port) {
    sock = new Socket();
    addr = new InetAddress(ip, port);
    sock->bind(addr);
    sock->listen();
}
Server::~Server() {}

Socket *Server::getSocket() {
    return sock;
}