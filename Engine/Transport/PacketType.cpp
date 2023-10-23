/**
 * PacketType.cpp - Contains get packet type function impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <Transport/PacketType.h>
#include <Transport/RTP/RTPPayloadType.h>

#include <Common/BitHelpers.h>

namespace Transport
{

PacketType GetPacketType(const uint8_t *data, uint32_t)
{
    uint8_t pt = *(static_cast<const uint8_t*>(data) + 1);
    bool m = BitIsSet(pt, 7);
    ClearBit(pt, 7);

    if (!m && (pt == static_cast<uint8_t>(RTPPayloadType::ptVP8) || pt == static_cast<uint8_t>(RTPPayloadType::ptOpus) || pt == static_cast<uint8_t>(RTPPayloadType::ptData)))
    {
        return PacketType::RTP;
    }

    return PacketType::RTCP;
}

}
