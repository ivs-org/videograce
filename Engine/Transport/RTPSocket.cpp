/**
 * RTPSocket.cpp - Contains RTP socket impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018, 2023
 */

#include <Transport/RTPSocket.h>

#include <Transport/RTP/RTPPacket.h>
#include <Transport/RTP/RTCPPacket.h>
#include <Transport/PacketType.h>

#include <Transport/UDPSocket.h>

#include <mt/thread_priority.h>

#include <spdlog/spdlog.h>

namespace Transport
{

class RTPSocketImpl
{
    UDPSocket socket;

    Address defaultAddress;

    ISocket *rtpReceiver, *rtcpReceiver;
    
    std::shared_ptr<spdlog::logger> sysLog, errLog;

public:
    RTPSocketImpl()
        : socket(std::bind(&RTPSocketImpl::receive, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)),
        defaultAddress(),
        rtpReceiver(nullptr), rtcpReceiver(nullptr),
        sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
    {
    }
    
    ~RTPSocketImpl()
    {
        Stop();
    }
    
    void SetDefaultAddress(std::string_view address, uint16_t port)
    {
        defaultAddress = Address(address, port);
    }

    void Start(Address::Type type, uint16_t bindPort)
    {
        if (type == Address::Type::Auto)
        {
            type = defaultAddress.type;
        }
        
        socket.Start(type, bindPort);

        sysLog->info("RTPSocket[{0}] started, port: {1}", socket.GetSocketNumber(), socket.GetBindedPort());
    }
    
    void Stop()
    {
        if (socket.Runned())
        {
            sysLog->info("RTPSocket[{0}] ended, port: {1}", socket.GetSocketNumber(), socket.GetBindedPort());
            socket.Stop();
        }
    }
    
    void SetReceiver(ISocket *rtpReceiver_, ISocket *rtcpReceiver_)
    {
        rtpReceiver = rtpReceiver_;
        rtcpReceiver = rtcpReceiver_;
    }
    
    void Send(const IPacket &packet_, const Address *address)
    {
        if (!socket.Runned())
        {
            return;
        }

        uint8_t sendBuf[UDPSocket::MAX_DATAGRAM_SIZE] = { 0 };
        uint32_t serializedSize = UDPSocket::MAX_DATAGRAM_SIZE;
        
        switch (packet_.GetType())
        {
            case PacketType::RTP:
            {
                const Transport::RTPPacket &packet = *static_cast<const Transport::RTPPacket*>(&packet_);
                packet.Serialize(sendBuf, &serializedSize);
            }
            break;
            case PacketType::RTCP:
            {
                const Transport::RTCPPacket &packet = *static_cast<const Transport::RTCPPacket*>(&packet_);
                packet.Serialize(sendBuf, &serializedSize);
            }
            break;
            default:
            break;
        }

        socket.Send(sendBuf, serializedSize, address ? *address : defaultAddress, 0);
    }

private:
    void receive(const uint8_t *data, uint16_t size, const Address &address, uint16_t socketPort)
    {
        const PacketType packetType = GetPacketType(data, size);

        switch (packetType)
        {
            case PacketType::RTP:
            {
                Transport::RTPPacket rtpPacket;
                if (rtpReceiver != nullptr && rtpPacket.Parse(data, size))
                {
                    if (RTPSocket::WITH_TRACES)
                    {
                        sysLog->trace("RTPSocket[{0}] receive RTP packet, size: {1}, from: {2}, socket port: {3}",
                            socket.GetSocketNumber(), size, address.toString(), socket.GetBindedPort());
                    }
                    
                    rtpReceiver->Send(rtpPacket, &address);
                }
            }
            break;
            case PacketType::RTCP:
            {
                Transport::RTCPPacket rtcpPacket;
                if (rtcpReceiver != nullptr && rtcpPacket.Parse(data, size))
                {
                    if (RTPSocket::WITH_TRACES)
                    {
                        sysLog->trace("RTPSocket[{0}] receive RTCP packet, size: {1}, from: {2}, socket port: {3}",
                            socket.GetSocketNumber(), size, address.toString(), socket.GetBindedPort());
                    }

                    rtcpReceiver->Send(rtcpPacket, &address);
                }
            }
            break;
            default:
            break;
        }
    }
};

RTPSocket::RTPSocket()
    : impl(new RTPSocketImpl())
{
}

RTPSocket::~RTPSocket()
{
}
    
void RTPSocket::SetDefaultAddress(std::string_view address, uint16_t port)
{
    impl->SetDefaultAddress(address, port);
}

void RTPSocket::Start(Address::Type type, uint16_t bindPort)
{
    impl->Start(type, bindPort);
}

void RTPSocket::Stop()
{
    impl->Stop();
}
    
void RTPSocket::SetReceiver(ISocket *rtpReceiver, ISocket *rtcpReceiver)
{
    impl->SetReceiver(rtpReceiver, rtcpReceiver);
}

void RTPSocket::Send(const IPacket &packet, const Address *address)
{
    impl->Send(packet, address);
}

}
