/**
 * UDPSocket.cpp - Contains UDP socket impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018, 2023
 */

#include <Transport/UDPSocket.h>

#include <mt/thread_priority.h>

namespace Transport
{

UDPSocket::UDPSocket(std::function<void(const uint8_t *data, uint16_t size, const Address &address, uint16_t socketPort)> receiver_)
    : receiver(receiver_),
    netsocket(0),
    bindedPort(0),
    thread(),
    runned(false),
    sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{}

    
UDPSocket::~UDPSocket()
{
    Stop();
}
    
void UDPSocket::Start(Address::Type type, uint16_t bindPort)
{
    if (runned)
    {
        return;
    }

    netsocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (netsocket == INVALID_SOCKET)
    {
        return errLog->critical("UDPSocket[null] Start() error: {0}, port: {1}", GET_LAST_ERROR, bindPort);
    }

    sockaddr_in bindAddr = { 0 };
    bindAddr.sin_addr.s_addr = INADDR_ANY;
    bindAddr.sin_family = AF_INET;
    bindAddr.sin_port = htons(bindPort);

    switch (type)
    {
        case Address::Type::IPv6:
        {
            netsocket = socket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP);
            if (netsocket == INVALID_SOCKET)
            {
                return errLog->critical("UDPSocket[null] IPv6 Start() error: {0}, port: {1}", GET_LAST_ERROR, bindPort);
            }

            int yes = 0; /// disabling only ipv6 to receive ipv4 traffic
            int ret = setsockopt(netsocket, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&yes, sizeof(yes));
            if (ret == SOCKET_ERROR)
            {
                return errLog->critical("UDPSocket[{0}] Start() IPv6 IPV6_V6ONLY set error: {1}, port: {2}", netsocket, GET_LAST_ERROR, bindPort);
            }

            sockaddr_in6 bindAddr = { 0 };
            bindAddr.sin6_addr = IN6ADDR_ANY_INIT;
            bindAddr.sin6_family = AF_INET6;
            bindAddr.sin6_port = htons(bindPort);

            ret = bind(netsocket, (sockaddr*)&bindAddr, sizeof(bindAddr));
            if (ret == SOCKET_ERROR)
            {
                return errLog->critical("UDPSocket[{0}] Start() IPv6 bind error: {1}, port: {2}", netsocket, GET_LAST_ERROR, bindPort);
            }
        }
        break;
        case Address::Type::IPv4:
        {
            netsocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (netsocket == INVALID_SOCKET)
            {
                return errLog->critical("UDPSocket[null] IPv4 Start() error: {0}, port: {1}", GET_LAST_ERROR, bindPort);
            }

            sockaddr_in bindAddr = { 0 };
            bindAddr.sin_addr.s_addr = INADDR_ANY;
            bindAddr.sin_family = AF_INET;
            bindAddr.sin_port = htons(bindPort);

            int ret = bind(netsocket, (sockaddr*)&bindAddr, sizeof(bindAddr));
            if (ret == SOCKET_ERROR)
            {
                return errLog->critical("UDPSocket[{0}] Start() IPv4 bind error: {1}, port: {2}", netsocket, GET_LAST_ERROR, bindPort);
            }
        }
        break;
        default:
        break;
    }

    struct sockaddr_storage sin;
    socklen_t len = sizeof(sin);
    int ret = getsockname(netsocket, (struct sockaddr *)&sin, &len);
    if (ret == SOCKET_ERROR)
    {
        return errLog->critical("UDPSocket[{0}] Start() getsockname error: {1}, port: {2}", netsocket, GET_LAST_ERROR, bindPort);
    }
    bindedPort = ntohs(((struct sockaddr_in *)&sin)->sin_port);

    int rcvBufSize = 1024 * 1024;
    ret = setsockopt(netsocket, SOL_SOCKET, SO_RCVBUF, (const char*)&rcvBufSize, sizeof(rcvBufSize));
    if (ret == SOCKET_ERROR)
    {
        return errLog->critical("UDPSocket[{0}] Start() SO_RCVBUF setting error: {1}, port: {2}", netsocket, GET_LAST_ERROR, bindedPort);
    }

    int sndBufSize = 1024 * 1024;
    ret = setsockopt(netsocket, SOL_SOCKET, SO_SNDBUF, (const char*)&sndBufSize, sizeof(sndBufSize));
    if (ret == SOCKET_ERROR)
    {
        return errLog->critical("UDPSocket[{0}] Start() SO_SNDBUF setting error: {1}, port: {2}", netsocket, GET_LAST_ERROR, bindedPort);
    }

#ifndef _WIN32
    int tos = IPTOS_LOWDELAY;
    ret = setsockopt(netsocket, IPPROTO_IP, IP_TOS, (const char*)&tos, sizeof(tos));
    if (ret == SOCKET_ERROR)
    {
        return errLog->critical("UDPSocket[{0}] Start() IP_TOS setting error: {1}, port: {2}", netsocket, GET_LAST_ERROR, bindedPort);
    }
#endif

    runned = true;
    thread = std::thread(&UDPSocket::run, this);
    mt::set_thread_priority(thread, mt::priority_type::real_time);

    sysLog->info("UDPSocket[{0}] started, port: {1}", netsocket, bindedPort);
}
    
void UDPSocket::Stop()
{
    if (runned)
    {
        runned = false;
#ifndef _WIN32
        shutdown(netsocket, SHUT_RDWR);
        close(netsocket);
#else
        closesocket(netsocket);
#endif
        if (thread.joinable()) thread.join();

        sysLog->info("UDPSocket[{0}] ended, port: {1}", netsocket, bindedPort);

        netsocket = 0;
    }
}

void UDPSocket::Send(const uint8_t *data, uint16_t size, const Address &address, uint16_t socketPort)
{
    if (!runned)
    {
        return;
    }
        
    switch (address.type)
    {
        case Address::Type::IPv4:
        {
            const int ret = sendto(netsocket, (const char*)data, size, 0, (sockaddr*)&address.v4addr, sizeof(sockaddr_in));
            if (ret == SOCKET_ERROR)
            {
                errLog->critical("UDPSocket[{0}] Send() IPv4 error: {1}, receiver: {2}, socket port: {3}", 
                    netsocket, GET_LAST_ERROR, address.toString(), bindedPort);
            }
        }
        break;
        case Address::Type::IPv6:
        {
            const int ret = sendto(netsocket, (const char*)data, size, 0, (sockaddr*)&address.v6addr, sizeof(sockaddr_in6));
            if (ret == SOCKET_ERROR)
            {
                errLog->critical("UDPSocket[{0}] Send() IPv6 error: {1}, receiver: {2}, socket port: {3}",
                    netsocket, GET_LAST_ERROR, address.toString(), bindedPort);
            }
        }
        break;
        default:
        break;
    }

    if (WITH_TRACES)
    {
        sysLog->trace("UDPSocket[{0}] sended, size: {1}, to: {2}, socket port: {3}",
            netsocket, size, address.toString(), bindedPort);
    }
}

bool UDPSocket::Runned() const
{
    return runned;
}

uint16_t UDPSocket::GetBindedPort() const
{
    return bindedPort;
}

uint64_t UDPSocket::GetSocketNumber() const
{
    return netsocket;
}

void UDPSocket::run()
{
    while (runned)
    {
        sockaddr_storage senderAddr = { 0 };
        socklen_t senderAddrSize = sizeof(senderAddr);

        uint8_t recvBuf[MAX_DATAGRAM_SIZE] = { 0 };
        const int recvSize = recvfrom(netsocket, (char*)recvBuf, MAX_DATAGRAM_SIZE, 0, (struct sockaddr*)&senderAddr, &senderAddrSize);
        if (recvSize == SOCKET_ERROR)
        {
            errLog->critical("UDPSocket[{0}] recvfrom() error: {1}", netsocket, GET_LAST_ERROR);
            continue;
        }

        Address addr(&senderAddr);

        if (WITH_TRACES)
        {
            sysLog->trace("UDPSocket[{0}] receive, size: {1}, from: {2}, socket port: {3}",
                netsocket, recvSize, addr.toString(), bindedPort);
        }

        receiver(recvBuf, recvSize, addr, bindedPort);
    }
}

}
