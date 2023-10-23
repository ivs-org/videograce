/**
 * RTPPacket.cpp - Contains RTPPacket with buffer own its data impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <Transport/RTP/OwnedRTPPacket.h>

#include <new>

namespace Transport
{
    
OwnedRTPPacket::OwnedRTPPacket()
    : header(),
    data(nullptr),
    size(0),
    payload_ms(40),
    payload_type(Transport::RTPPayloadType::ptVP8)
{
}

OwnedRTPPacket::OwnedRTPPacket(const Transport::RTPPacket::RTPHeader &header_,
    const uint8_t *payload, 
    uint32_t size_,
    Transport::RTPPayloadType payload_type_)
        : header(header_),
        data(nullptr),
        size(size_),
        payload_ms(payload_type_ == Transport::RTPPayloadType::ptVP8 ? 40 : 20),
        payload_type(payload_type_)
{
    if (size)
    {
        data = new (std::nothrow) uint8_t[size];
        if (data && payload)
        {
            memcpy(data, payload, size);
        }
    }
}

OwnedRTPPacket::~OwnedRTPPacket()
{
    if (data)
    {
        delete[] data;
        data = nullptr;
    }
}

}
