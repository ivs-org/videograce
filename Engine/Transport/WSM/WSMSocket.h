/**
 * WSMSocket.h - Contains WebSocket Media socket interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2024
 */

#pragma once

#include <Transport/ISocket.h>
#include <Transport/Address.h>
#include <memory>

namespace Transport
{

class WSMSocketImpl;

class WSMSocket : public ISocket
{
public:
    WSMSocket();
    ~WSMSocket();
    
    void Start(std::string_view address_, std::string_view accessToken_);
    void Stop();
    
    void SetReceiver(ISocket *rtpReceiver, ISocket *rtcpReceiver);

    /// ISocket impl
    virtual void Send(const IPacket &packet, const Address *address = nullptr) final;

    static constexpr bool WITH_TRACES = false;

private:
    std::unique_ptr<WSMSocketImpl> impl;
};

}
