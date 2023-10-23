/**
 * Server.h - Contains TCP server impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <thread>
#include <boost/asio.hpp>

namespace Transport
{

class tcp_server;

class TCPServer
{
public:    
    TCPServer(uint16_t port_);
    ~TCPServer();
    
    void Start(bool useIPv6);
    void Stop();

    static constexpr bool WITH_TRACES = false;

private:
    std::string address;
    uint16_t port;

    std::unique_ptr <boost::asio::io_service> io_service;

    std::unique_ptr<tcp_server> tcp_server_;

    std::thread thread_;
};

}
