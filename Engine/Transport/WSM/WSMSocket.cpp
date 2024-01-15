/**
 * WSMSocket.h - Contains WebSocket Media socket impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018, 2023
 */

#include <Transport/WSM/WSMSocket.h>

#include <Transport/WS/WebSocket.h>

#include <Transport/RTP/RTPPacket.h>
#include <Transport/RTP/RTCPPacket.h>
#include <Transport/PacketType.h>

#include <Transport/UDPSocket.h>

#include <Proto/CommandType.h>
#include <Proto/CmdMedia.h>
#include <Proto/CmdConnectRequest.h>
#include <Proto/CmdConnectResponse.h>
#include <Proto/CmdPing.h>
#include <Proto/CmdDisconnect.h>

#include <queue>

#include <Common/Base64.h>

#include <spdlog/spdlog.h>

namespace Transport
{

class WSMSocketImpl
{
    std::string address;
    std::string accessToken;
    std::string destAddr;

    int64_t connectionId;

    WebSocket webSocket;

    ISocket *rtpReceiver, *rtcpReceiver;

    std::queue<std::string> offlineQueue;
    
    std::shared_ptr<spdlog::logger> sysLog, errLog;

public:
    WSMSocketImpl()
        : address(), accessToken(), destAddr(),
        connectionId(-1),
        webSocket(std::bind(&WSMSocketImpl::OnWebSocket, this, std::placeholders::_1, std::placeholders::_2)),
        rtpReceiver(nullptr), rtcpReceiver(nullptr),
        offlineQueue(),
        sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
    {
    }
    
    ~WSMSocketImpl()
    {
        Stop();
    }
    
    void Start(std::string_view address_, std::string_view accessToken_, std::string_view destAddr_)
    {
        address = address_;
        accessToken = accessToken_;
        destAddr = destAddr_;

        if (!webSocket.IsConnected())
        {
            webSocket.Connect("http://" + address); /// We don't need https, because media payload is already encrypted
        }

        spdlog::get("System")->trace("WSMSocket :: Connected, server address {0}, access token: {1}", address_, accessToken_);
    }
    
    void Stop()
    {
        if (webSocket.IsConnected())
        {
            webSocket.Send(Proto::DISCONNECT::Command().Serialize());
        }
        webSocket.Disconnect();
    }
    
    void SetReceiver(ISocket *rtpReceiver_, ISocket *rtcpReceiver_)
    {
        rtpReceiver = rtpReceiver_;
        rtcpReceiver = rtcpReceiver_;
    }

    void Logon()
    {
        if (webSocket.IsConnected())
        {
            Proto::CONNECT_REQUEST::Command cmd;

            cmd.type = Proto::CONNECT_REQUEST::Type::WSMedia;
            cmd.access_token = accessToken;

            webSocket.Send(cmd.Serialize());
        }
    }
    
    void Send(const IPacket &packet_, const Address *address)
    {
        uint8_t sendBuf[UDPSocket::MAX_DATAGRAM_SIZE] = { 0 };
        uint32_t serializedSize = UDPSocket::MAX_DATAGRAM_SIZE;
        
        switch (packet_.GetType())
        {
            case PacketType::RTP:
            {
                const Transport::RTPPacket &packet = *static_cast<const Transport::RTPPacket*>(&packet_);
                packet.Serialize(sendBuf, &serializedSize);

                auto cmd = Proto::MEDIA::Command(Proto::MEDIA::MediaType::RTP,
                    packet.rtpHeader.ssrc,
                    destAddr,
                    Common::toBase64(std::string((const char*)sendBuf, serializedSize))).Serialize();

                if (webSocket.IsConnected())
                {
                    webSocket.Send(cmd);
                }
                else
                {
                    offlineQueue.push(cmd);
                }

                if (WSMSocket::WITH_TRACES)
                {
                    sysLog->trace("WSMSocket[{0}] :: Send :: RTP -> WSM (size: {1}, ssrc: {2})",
                        connectionId, serializedSize, packet.rtpHeader.ssrc);
                }
            }
            break;
            case PacketType::RTCP:
            {
                const Transport::RTCPPacket &packet = *static_cast<const Transport::RTCPPacket*>(&packet_);
                packet.Serialize(sendBuf, &serializedSize);

                if (packet.rtcp_common.pt == Transport::RTCPPacket::RTCP_APP &&
                    packet.rtcp_common.length == 1)
                {
                    auto cmd = Proto::MEDIA::Command(Proto::MEDIA::MediaType::RTCP,
                        packet.rtcps[0].r.app.ssrc,
                        destAddr,
                        Common::toBase64(std::string((const char*)sendBuf, serializedSize))).Serialize();

                    if (webSocket.IsConnected())
                    {
                        webSocket.Send(cmd);
                    }
                    else
                    {
                        offlineQueue.push(cmd);
                    }

                    if (WSMSocket::WITH_TRACES)
                    {
                        sysLog->trace("WSMSocket[{0}] :: Send :: RTCP -> WSM (size: {1}, ssrc: {2})",
                            connectionId, serializedSize, packet.rtcps[0].r.app.ssrc);
                    }
                }
                else
                {
                    errLog->debug("WSMSocket[{0}] :: Send :: Unsupported RTCP", connectionId);
                }
            }
            break;
            default:
            break;
        }
    }

private:
    void OnWebSocket(WSMethod method, std::string_view message)
    {
		switch (method)
		{
			case Transport::WSMethod::Open:
				sysLog->info("WSMSocket :: Connection to server established");

				Logon();
			break;
			case Transport::WSMethod::Message:
			{
				auto commandType = Proto::GetCommandType(message);
				switch (commandType)
				{
				    case Proto::CommandType::ConnectResponse:
				    {
					    Proto::CONNECT_RESPONSE::Command cmd;
					    cmd.Parse(message);

                        connectionId = cmd.connection_id;

					    if (cmd.result == Proto::CONNECT_RESPONSE::Result::OK)
					    {
						    sysLog->trace("WSMSocket :: Logon success, connection id: {0}", cmd.connection_id);

                            while (!offlineQueue.empty())
                            {
                                webSocket.Send(offlineQueue.front());
                                offlineQueue.pop();
                            }
					    }
                        else
                        {
                            errLog->error("WSMSocket :: can't make the ws media session, [MEDIA FAIL]");
                        }
				    }
                    break;
                    case Proto::CommandType::Media:
                    {
                        Proto::MEDIA::Command cmd;
                        cmd.Parse(message);

                        auto data = Common::fromBase64(cmd.data);

                        switch (cmd.media_type)
                        {
                            case Proto::MEDIA::MediaType::RTP:
                            {
                                Transport::RTPPacket rtpPacket;
                                if (rtpReceiver != nullptr && rtpPacket.Parse((const uint8_t*)data.c_str(), data.size()))
                                {
                                    if (WSMSocket::WITH_TRACES)
                                    {
                                        sysLog->trace("WSMSocket[{0}] :: Receive :: WSM -> RTP (size: {1}, ssrc: {2})",
                                            connectionId, data.size(), cmd.ssrc);
                                    }

                                    rtpReceiver->Send(rtpPacket);
                                }
                            }
                            break;
                            case Proto::MEDIA::MediaType::RTCP:
                            {
                                Transport::RTCPPacket rtcpPacket;
                                if (rtcpReceiver != nullptr && rtcpPacket.Parse((const uint8_t*)data.c_str(), data.size()))
                                {
                                    if (WSMSocket::WITH_TRACES)
                                    {
                                        sysLog->trace("WSMSocket[{0}] :: Receive :: WSM -> RTCP (size: {1}, ssrc: {2})",
                                            connectionId, data.size(), cmd.ssrc);
                                    }

                                    rtcpReceiver->Send(rtcpPacket);
                                }
                            }
                            break;
                        }
                    }
                    break;
			        case Proto::CommandType::Ping:
				        webSocket.Send(Proto::PING::Command().Serialize());
				    break;
                }
			}
			break;
		    case Transport::WSMethod::Close:
			    sysLog->info("WSMSocket :: WebSocket closed (message: \"{0}\")", message);
			break;
		    case Transport::WSMethod::Error:
			    errLog->error("WSMSocket :: WebSocket error (message: \"{0}\")", message);
			break;
		}
    }
};

WSMSocket::WSMSocket()
    : impl(new WSMSocketImpl())
{
}

WSMSocket::~WSMSocket()
{
}

void WSMSocket::Start(std::string_view address_, std::string_view accessToken_, std::string_view destAddr_)
{
    impl->Start(address_, accessToken_, destAddr_);
}

void WSMSocket::Stop()
{
    impl->Stop();
}
    
void WSMSocket::SetReceiver(ISocket *rtpReceiver, ISocket *rtcpReceiver)
{
    impl->SetReceiver(rtpReceiver, rtcpReceiver);
}

void WSMSocket::Send(const IPacket &packet, const Address *address)
{
    impl->Send(packet, address);
}

}
