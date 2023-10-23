/**
 * SocketSplitter.h - Contains simple two channel socket splitter
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <Transport/ISocket.h>

namespace Transport
{

class SocketSplitter : public Transport::ISocket
{
public:
    virtual void Send(const Transport::IPacket &packet, const Transport::Address *address = nullptr) final
    {
        if (receiver0)
        {
            receiver0->Send(packet, address);
        }
        if (receiver1)
        {
            receiver1->Send(packet, address);
        }
    }

    void SetReceiver0(Transport::ISocket *receiver0_)
    {
        receiver0 = receiver0_;
    }

    void SetReceiver1(Transport::ISocket *receiver1_)
    {
        receiver1 = receiver1_;
    }

    SocketSplitter()
        : receiver0(nullptr), receiver1(nullptr)
    {
    }

    ~SocketSplitter() {}

private:
    Transport::ISocket *receiver0, *receiver1;
        
};

}
