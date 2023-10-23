/**
 * Server.cpp - Contains TCP server impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2023
 */

#include <Transport/TCP/Server.h>

#include <Transport/TCP/Message.h>
#include <Transport/UDPSocket.h>

#include <boost/bind/bind.hpp>

#include <deque>
#include <map>

#include <spdlog/spdlog.h>

namespace Transport
{

using boost::asio::ip::tcp;

typedef std::deque<message> message_queue;

class tcp_session : public std::enable_shared_from_this<tcp_session>
{
    bool alive;

    tcp::socket socket_;

    message read_msg_;

    std::mutex msgs_mutex_;
    message_queue write_msgs_;

    typedef std::shared_ptr<UDPSocket> socket_ptr;
    std::map<uint16_t /* remote socket port */, socket_ptr> sockets;
    std::map<uint16_t /* local socket port */, uint16_t /* remote socket port */> ports;

    std::shared_ptr<spdlog::logger> sysLog, errLog;

public:
    tcp_session(boost::asio::io_service& io_service)
        : alive(true), socket_(io_service),
        sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
    {
    }

    ~tcp_session()
    {
        alive = false;
    }

    tcp::socket& socket()
    {
        return socket_;
    }

    void start()
    {
        boost::asio::async_read(socket_,
            boost::asio::buffer(read_msg_.data(), message::header_length),
            boost::bind(
                &tcp_session::handle_read_header, shared_from_this(),
                boost::asio::placeholders::error));
    }

    socket_ptr find_socket(uint16_t sender_port)
    {
        auto sock = sockets.find(sender_port);
        if (sock != sockets.end())
        {
            return sock->second;
        }
        else
        {
            socket_ptr s(new UDPSocket(std::bind(&tcp_session::send, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
            s->Start();

            sockets.insert(std::pair<uint16_t, socket_ptr>(sender_port, s));

            ports[s->GetBindedPort()] = sender_port;

            return s;
        }

        return nullptr;
    }

    uint16_t find_port(uint16_t local_port)
    {
        auto port = ports.find(local_port);
        if (port != ports.end())
        {
            return port->second;
        }
        return 0;
    }

    virtual void send(const uint8_t *data, uint16_t size, const Address &address, uint16_t socketPort)
    {
        if (alive)
        {
            message msg;
            msg.body_length(size);
            msg.dest_port(find_port(socketPort));
            msg.src_port(address.type == Address::Type::IPv4 ? ntohs(address.v4addr.sin_port) : ntohs(address.v6addr.sin6_port));
            msg.write(data, size);
            msg.encode_header();
            deliver(msg);

            if (TCPServer::WITH_TRACES)
            {
                sysLog->trace("TCPServer send msg, len: {0}, src port: {1}, dest port: {2}", msg.body_length(), msg.src_port(), msg.dest_port());
            }
        }
    }

    void deliver(const message& msg)
    {
        std::lock_guard<std::mutex> lock(msgs_mutex_);

        bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(msg);
        if (!write_in_progress)
        {
            boost::asio::async_write(socket_,
                boost::asio::buffer(write_msgs_.front().data(),
                    write_msgs_.front().length()),
                boost::bind(&tcp_session::handle_write, shared_from_this(),
                    boost::asio::placeholders::error));

            if (TCPServer::WITH_TRACES)
            {
                sysLog->trace("TCPServer delivered msg, len: {0}, src port: {1}, dest port: {2}", write_msgs_.front().body_length(), write_msgs_.front().src_port(), write_msgs_.front().dest_port());
            }
        }
        else
        {
            if (TCPServer::WITH_TRACES)
            {
                errLog->warn("TCPServer deliver write_in_progress");
            }
        }
    }

    void handle_read_header(const boost::system::error_code& error)
    {
        if (!error && read_msg_.decode_header())
        {
            boost::asio::async_read(socket_,
                boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                boost::bind(&tcp_session::handle_read_body, shared_from_this(),
                    boost::asio::placeholders::error));

            if (TCPServer::WITH_TRACES)
            {
                sysLog->trace("TCPServer readed header, len: {0}, src port: {1}, dest port: {2}", 
                    read_msg_.body_length(), read_msg_.src_port(), read_msg_.dest_port());
            }
        }
        else
        {
            errLog->critical("TCPServer handle_read_header error: {0}", error.message());
        }
    }

    void handle_read_body(const boost::system::error_code& error)
    {
        if (!error)
        {
            auto sock = find_socket(read_msg_.dest_port());
            if (sock)
            {
                if (TCPServer::WITH_TRACES)
                {
                    sysLog->trace("TCPServer readed body and send to UDP, len: {0}, src port: {1}, dest port: {2}", 
                        read_msg_.body_length(), read_msg_.src_port(), read_msg_.dest_port());
                }
                
                sock->Send((const uint8_t*)read_msg_.body(), read_msg_.body_length(), Address("127.0.0.1", read_msg_.src_port()), 0);
            }

            boost::asio::async_read(socket_,
                boost::asio::buffer(read_msg_.data(), message::header_length),
                boost::bind(&tcp_session::handle_read_header, shared_from_this(),
                    boost::asio::placeholders::error));
        }
    }

    void handle_write(const boost::system::error_code& error)
    {
        if (!error)
        {
            std::lock_guard<std::mutex> lock(msgs_mutex_);

            write_msgs_.pop_front();
            if (!write_msgs_.empty())
            {
                boost::asio::async_write(socket_,
                    boost::asio::buffer(write_msgs_.front().data(),
                        write_msgs_.front().length()),
                    boost::bind(&tcp_session::handle_write, shared_from_this(),
                        boost::asio::placeholders::error));

                if (TCPServer::WITH_TRACES)
                {
                    sysLog->trace("TCPServer async writed, len: {0}, src port: {1}, dest port: {2}", 
                        write_msgs_.front().body_length(), write_msgs_.front().src_port(), write_msgs_.front().dest_port());
                }
            }
        }
    }
};

typedef std::shared_ptr<tcp_session> tcp_session_ptr;

class tcp_server
{
public:
    tcp_server(boost::asio::io_service& io_service,
        const tcp::endpoint& endpoint)
        : io_service_(io_service),
        acceptor_(io_service, endpoint)
    {
        start_accept();
    }

    void start_accept()
    {
        tcp_session_ptr new_session(new tcp_session(io_service_));
        acceptor_.async_accept(new_session->socket(),
            boost::bind(&tcp_server::handle_accept, this, new_session,
                boost::asio::placeholders::error));
    }

    void handle_accept(tcp_session_ptr session,
        const boost::system::error_code& error)
    {
        if (!error)
        {
            boost::system::error_code error_;
            session->socket().set_option(tcp::no_delay(true), error_);

            session->start();

            spdlog::get("System")->info("TCPServer accepted and started new session");
        }

        start_accept();
    }

private:
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
};

TCPServer::TCPServer(uint16_t port_)
    : address(), port(port_),
    io_service(),
    tcp_server_(),
    thread_()
{
}

TCPServer::~TCPServer()
{
    Stop();
}
    
void TCPServer::Start(bool useIPv6)
{
    if (!tcp_server_ && port != 0)
    {
        try
        {
            io_service = std::unique_ptr<boost::asio::io_service>(new boost::asio::io_service());

            tcp::endpoint endpoint(useIPv6 ? tcp::v6() : tcp::v4(), port);
            tcp_server_ = std::unique_ptr<tcp_server>(new tcp_server(*io_service, endpoint));

            thread_ = std::thread([this]() { io_service->run(); });
        }
        catch (...)
        {
            spdlog::get("Error")->critical("TCPServer start error");
        }
    }
}

void TCPServer::Stop()
{
    if (tcp_server_)
    {
        io_service->stop();
        thread_.join();

        tcp_server_.reset(nullptr);
        io_service.reset(nullptr);
    }
}

}
