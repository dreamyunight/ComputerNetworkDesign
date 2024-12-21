#pragma once
#include "InetAddress.h"
#include "Socket.h"
#include <sys/epoll.h>
#include <vector>

class Server {
private:
    Socket      *sock;
    InetAddress *addr;
public:
    Server();
    Server(const char *ip, uint16_t port);
    ~Server();

    Socket *getSocket();
};