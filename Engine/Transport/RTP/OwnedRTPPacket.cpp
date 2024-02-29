/**
 * RTPPacket.cpp - Contains RTPPacket with buffer own its data impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <Transport/RTP/OwnedRTPPacket.h>

#include <utility>
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

OwnedRTPPacket::OwnedRTPPacket(uint32_t size_)
    : header(),
    data(size_ ? new (std::nothrow) uint8_t[size_] : nullptr),
    size(size_),
    payload_ms(0),
    payload_type(Transport::RTPPayloadType::ptUndefined)
{
    if (data)
    {
        memset(data, 0, size);
    }
}

OwnedRTPPacket::OwnedRTPPacket(const Transport::RTPPacket::RTPHeader &header_,
    const uint8_t *payload, 
    uint32_t size_,
    Transport::RTPPayloadType payload_type_)
        : header(header_),
        data(size_ ? new (std::nothrow) uint8_t[size_] : nullptr),
        size(size_),
        payload_ms(payload_type_ == Transport::RTPPayloadType::ptVP8 ? 40 : 20),
        payload_type(payload_type_)
{
    if (data && payload)
    {
        memcpy(data, payload, size);
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

OwnedRTPPacket& OwnedRTPPacket::operator=(OwnedRTPPacket&& other) noexcept
{
    // Guard self assignment
    if (this == &other)
        return *this; // delete[]/size=0 would also be ok

    if (other.data)
    {
        delete[] data;                             // release resource in *this
        data = std::exchange(other.data, nullptr); // leave other in valid state
    }
    size = std::exchange(other.size, 0);
    header = std::exchange(other.header, {});
    payload_ms = std::exchange(other.payload_ms, 0);
    payload_type = std::exchange(other.payload_type, {});

    return *this;
}

}
