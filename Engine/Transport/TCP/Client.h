/**
 * Client.h - Contains TCP client impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <memory>
#include <string>
#include <cstdint>

namespace Transport
{

class TCPClientImpl;

class TCPClient
{
public:
    TCPClient();
    ~TCPClient();

    void SetServerAddress(const std::string &address_, uint16_t port_);

    uint16_t CreatePipe(uint16_t serverTCPPort); /// Return the local UDP port and start the component if needed

    void EndSession(); /// On end conference flush the work

    static constexpr bool WITH_TRACES = false;

private:
    std::unique_ptr <TCPClientImpl> impl;

};

}
