/**
 * Address.h - Contains Transport's address inpl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#include <Transport/Address.h>

#include <Common/Common.h>

#include <sstream>
#include <random> 
#include <ctime>

#include <memory.h>

#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <arpa/inet.h>
#else
#include <ws2tcpip.h>
#endif

namespace Transport
{

Address::Address()
    : type(Type::Undefined), v4addr{ 0 }, v6addr{ 0 }
{
}

Address::Address(std::string_view addr, uint16_t port)
    : type(Type::Undefined), v4addr{ 0 }, v6addr{ 0 }
{
    struct addrinfo hints = { 0 }, *res = nullptr, *p = nullptr;

    int status = 0;
    
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if ((status = getaddrinfo(addr.data(), NULL, &hints, &res)) != 0)
    {
        DBGTRACE("getaddrinfo error for address: %s\n", addr);
        return;
    }

    for (p = res; p != NULL; p = p->ai_next)
    {
        if (p->ai_family == AF_INET)
        {
            memcpy(&v4addr, p->ai_addr, sizeof(struct sockaddr_in));
            v4addr.sin_port = htons(port);
            type = Type::IPv4;

            break;
        }
        else
        {
            memcpy(&v6addr, p->ai_addr, sizeof(struct sockaddr_in6));
            v6addr.sin6_port = htons(port);
            type = Type::IPv6;

            break;
        }
    }

    freeaddrinfo(res);
}

Address::Address(const sockaddr_storage *addr)
    : type(Type::Undefined), v4addr{ 0 }, v6addr{ 0 }
{
    if (addr->ss_family == AF_INET)
    {
        memcpy(&v4addr, (struct sockaddr_in*)addr, sizeof(struct sockaddr_in));
        type = Type::IPv4;
    }
    else if(addr->ss_family == AF_INET6)
    {
        memcpy(&v6addr, (struct sockaddr_in6*)addr, sizeof(struct sockaddr_in6));
        type = Type::IPv6;
    }
}

std::string toStr(sockaddr *addr, socklen_t size)
{
    char host[NI_MAXHOST] = { 0 };
    char serv[NI_MAXSERV] = { 0 };
    getnameinfo((sockaddr*)addr, size, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);

    std::stringstream ss_msg;
    ss_msg << host << ":" << serv;
    return ss_msg.str();
}

std::string Address::toString() const
{
    switch (type)
    {
        case Type::IPv4:
            return toStr((sockaddr*)&v4addr, sizeof(v4addr));
        break;
        case Type::IPv6:
            return toStr((sockaddr*)&v6addr, sizeof(v6addr));
        break;
        default: break;
    }
    return "";
}

bool Address::operator==(const Address& lv) const
{
    return lv.type == type &&
        memcmp(&lv.v4addr, &v4addr, sizeof(v4addr)) == 0 &&
        memcmp(&lv.v6addr, &v6addr, sizeof(v6addr)) == 0;
}

}
