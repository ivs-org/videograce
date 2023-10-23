/**
 * IPacket.h - Contains common packet's interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <Transport/PacketType.h>

namespace Transport
{

class IPacket
{
public:
    virtual void Serialize(uint8_t *outputBuffer, uint32_t *size) const = 0;
    virtual bool Parse(const uint8_t *inputData, uint32_t size) = 0;

    virtual PacketType GetType() const = 0;

    virtual ~IPacket() {}
};

}
