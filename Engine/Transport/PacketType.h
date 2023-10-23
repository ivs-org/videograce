/**
 * PacketType.h - Contains get packet type function header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <cstdint>

namespace Transport
{

enum class PacketType
{
    Undefined = 0,

    RTP,
    RTCP
};

PacketType GetPacketType(const uint8_t *data, uint32_t size);

}
