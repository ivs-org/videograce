/**
 * RTPSocket.h - Contains RTP socket interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018, 2023
 */

#pragma once

#include <Transport/ISocket.h>
#include <Transport/Address.h>
#include <memory>

namespace Transport
{

class RTPSocketImpl;

class RTPSocket : public ISocket
{
public:
    RTPSocket();
    ~RTPSocket();
    
    void SetDefaultAddress(const char *address, uint16_t port);
    void Start(Address::Type type = Address::Type::Auto, uint16_t bindPort = 0);
    void Stop();
    
    void SetReceiver(ISocket *rtpReceiver, ISocket *rtcpReceiver);

    /// ISocket impl
    virtual void Send(const IPacket &packet, const Address *address = nullptr) final;

    static constexpr bool WITH_TRACES = false;

private:
    std::unique_ptr<RTPSocketImpl> impl;
};

}
