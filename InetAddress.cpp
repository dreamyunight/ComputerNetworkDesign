#include <arpa/inet.h>
#include <cstring>
#include <strings.h>
#include <sys/socket.h>
#include "InetAddress.h"

InetAddress::InetAddress() : addr_len(sizeof(addr)) {
    bzero(&addr, sizeof(addr));
}
InetAddress::InetAddress(const char *ip, uint16_t port) {
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;//IPv4
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    addr_len = sizeof(addr);
}

InetAddress::~InetAddress() {
  
}