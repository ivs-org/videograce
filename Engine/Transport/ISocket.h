/**
 * ISocket.h - Contains Transport's socket interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014, 2023
 */

#pragma once

namespace Transport
{

class IPacket;

struct Address;

class ISocket
{
public:
    virtual void Send(const IPacket &packet, const Address *address = nullptr) = 0;

    virtual ~ISocket() {}
};

}
