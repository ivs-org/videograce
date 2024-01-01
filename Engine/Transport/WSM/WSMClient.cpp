/**
 * Client.cpp - Contains WebSocket Media client impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2023
 */

#include <Transport/WSM/Client.h>

#include <thread>

#include <map>

#include <Transport/WS/WebSocket.h>

#include <Transport/UDPSocket.h>

#include <spdlog/spdlog.h>

namespace Transport
{

class WSMClientImpl
{
    std::string address;
    uint16_t port;
    std::string access_token;

    WebSocket web_socket;

    std::thread thread_;
public:
    WSMClientImpl()
        : address(), port(0), access_token(),
        web_socket(std::bind(&WSMClientImpl::OnWebSocket, this, std::placeholders::_1, std::placeholders::_2)),
        thread_()
    {}
    
    ~WSMClientImpl()
    {
        EndSession();
    }

    void SetServer(std::string_view address_, uint16_t port_, std::string_view access_token_)
    {
        address = address_;
        port = port_;
        access_token = access_token_;

        spdlog::get("System")->trace("WSMClient Server address {0}:{1}, access token: {2}", address_, port, access_token_);
    }

    uint16_t CreatePipe()
    {
        /*if (!tcp_client_)
        {
            io_service = std::unique_ptr<boost::asio::io_service>(new boost::asio::io_service());

            tcp::resolver resolver(*io_service);
            tcp::resolver::query query(address, std::to_string(port));
            tcp::resolver::iterator iterator = resolver.resolve(query);

            tcp_client_ = std::unique_ptr<tcp_client>(new tcp_client(*io_service, iterator));
            thread_ = std::thread([this]() { io_service->run(); });
        }

        return tcp_client_->create_pipe(serverPort);*/
        return 0;
    }

    void EndSession()
    {
        /*if (tcp_client_)
        {
            tcp_client_->close();

            thread_.join();

            tcp_client_.reset(nullptr);
            io_service.reset(nullptr);
        }*/
    }

    void OnWebSocket(WSMethod method, std::string_view message)
    {

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

void WSMClient::SetServer(std::string_view address_, uint16_t port_, std::string_view account_token_)
{
    impl->SetServer(address_, port_, account_token_);
}

void WSMClient::EndSession()
{
    impl->EndSession();
}

uint16_t WSMClient::CreatePipe()
{
    return impl->CreatePipe();
}

}
