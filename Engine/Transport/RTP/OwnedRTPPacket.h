/**
 * RTPPacket.h - Contains RTPPacket with buffer own its data
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#pragma once

#include <Transport/RTP/RTPPacket.h>
#include <Transport/RTP/RTPPayloadType.h>

#include <cstring>

namespace Transport
{

class OwnedRTPPacket
{

public:
    Transport::RTPPacket::RTPHeader header;
    uint8_t*                        data;
    uint32_t                        size;
    uint16_t                        payload_ms;
    Transport::RTPPayloadType       payload_type;
    
    OwnedRTPPacket();
    OwnedRTPPacket(uint32_t size);
    OwnedRTPPacket(const Transport::RTPPacket::RTPHeader &header_,
        const uint8_t *payload,
        uint32_t size_,
        Transport::RTPPayloadType payload_type_);
    
    ~OwnedRTPPacket();

    OwnedRTPPacket& operator=(OwnedRTPPacket&& other) noexcept;
};

}
