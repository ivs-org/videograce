/**
 * Client.cpp - Contains WebSocket Media client impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2023
 */

#include <Transport/WSM/Client.h>

#include <map>

#include <Transport/WS/WebSocket.h>

#include <Transport/UDPSocket.h>

#include <Proto/CommandType.h>
#include <Proto/CmdConnectRequest.h>
#include <Proto/CmdConnectResponse.h>
#include <Proto/CmdPing.h>
#include <Proto/CmdDisconnect.h>

#include <spdlog/spdlog.h>

namespace Transport
{

class WSMClientImpl
{
    std::string address;
    std::string accessToken;

    WebSocket webSocket;

    typedef std::shared_ptr<UDPSocket> udp_socket_ptr;
    std::map<uint16_t /* remote (server's) port */, udp_socket_ptr> udp_sockets_;
    std::map<uint16_t /* local socket port */, uint16_t /* remote socket port */> ports;

    std::shared_ptr<spdlog::logger> sysLog, errLog;
public:
    WSMClientImpl()
        : address(), accessToken(),
        webSocket(std::bind(&WSMClientImpl::OnWebSocket, this, std::placeholders::_1, std::placeholders::_2)),
        udp_sockets_(), ports(),
        sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
    {}
    
    ~WSMClientImpl()
    {
        EndSession();
    }

    void SetServer(std::string_view address_, std::string_view access_token_)
    {
        address = address_;
        accessToken = access_token_;

        spdlog::get("System")->trace("WSMClient :: Server address {0}, access token: {1}", address_, access_token_);
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

    uint16_t CreatePipe(uint16_t serverUDPPort)
    {
        if (!webSocket.IsConnected())
        {
            webSocket.Connect("http://" + address); /// We don't need https, because media payload is already encrypted
        }

        auto it = udp_sockets_.find(serverUDPPort);
        if (it != udp_sockets_.end())
        {
            sysLog->info("WSMClient return exist pipe (local: {0}, remote: {1})", it->second->GetBindedPort(), serverUDPPort);
            return it->second->GetBindedPort();
        }

        udp_socket_ptr s(new UDPSocket(std::bind(&WSMClientImpl::ReceiveUDP, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
        s->Start();

        udp_sockets_.insert(std::pair<uint16_t, udp_socket_ptr>(serverUDPPort, s));

        ports[s->GetBindedPort()] = serverUDPPort;

        sysLog->info("WSMClient created pipe (local: {0}, remote: {1})", s->GetBindedPort(), serverUDPPort);

        return s->GetBindedPort();
    }

    void EndSession()
    {
        if (webSocket.IsConnected())
        {
            webSocket.Send(Proto::DISCONNECT::Command().Serialize());
        }
        webSocket.Disconnect();
    }

    void OnWebSocket(WSMethod method, std::string_view message)
    {
		switch (method)
		{
			case Transport::WSMethod::Open:
				sysLog->info("WSMClient :: Connection to server established");

				std::this_thread::yield();

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

					    if (cmd.result == Proto::CONNECT_RESPONSE::Result::OK)
					    {
						    sysLog->trace("WSMClient :: Connected");
					    }
                        else
                        {
                            errLog->error("WSMClient :: can't make the ws media session, [MEDIA FAIL]");
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
			    sysLog->info("WSMClient :: WebSocket closed (message: \"{0}\")", message);
                udp_sockets_.clear();
			break;
		    case Transport::WSMethod::Error:
			    errLog->critical("WSMClient :: WebSocket error (message: \"{0}\")", message);
                udp_sockets_.clear();
			break;
		}
    }

    void ReceiveUDP(const uint8_t* data, uint16_t size, const Address& address, uint16_t socketPort)
    {
        /// Send from local UDP to WS media channel on json with base64
        /*message msg;
        msg.body_length(size);
        msg.dest_port(address.type == Address::Type::IPv4 ? ntohs(address.v4addr.sin_port) : ntohs(address.v6addr.sin6_port));
        msg.src_port(ports[socketPort]);
        msg.write(data, size);
        msg.encode_header();

        io_service_.post(boost::bind(&tcp_client::do_write, this, msg));*/
    }
};

WSMClient::WSMClient()
    : impl(new WSMClientImpl())
{
}
WSMClient::~WSMClient()
{
    impl.reset();
}

void WSMClient::SetServer(std::string_view address_, std::string_view account_token_)
{
    impl->SetServer(address_, account_token_);
}

void WSMClient::EndSession()
{
    impl->EndSession();
}

uint16_t WSMClient::CreatePipe(uint16_t serverUDPPort)
{
    return impl->CreatePipe(serverUDPPort);
}

}
