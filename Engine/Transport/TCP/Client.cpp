/**
 * Client.cpp - Contains TCP client impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <deque>
#include <thread>

#include <map>

#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>

#include <Transport/TCP/Client.h>

#include <Transport/TCP/Message.h>
#include <Transport/UDPSocket.h>

#include <spdlog/spdlog.h>

namespace Transport
{

using boost::asio::ip::tcp;

typedef std::deque<message> message_queue;

class tcp_client
{
    boost::asio::io_service& io_service_;
    tcp::socket socket_;

    tcp::resolver::iterator endpoint_iterator_;

    message read_msg_;

    message_queue write_msgs_;

    typedef std::shared_ptr<UDPSocket> udp_socket_ptr;
    std::map<uint16_t /* remote (server's) port */, udp_socket_ptr> udp_sockets_;
    std::map<uint16_t /* local socket port */, uint16_t /* remote socket port */> ports;

    bool connected, first_do;

    std::shared_ptr<spdlog::logger> sysLog, errLog;

public:
    tcp_client(boost::asio::io_service& io_service, tcp::resolver::iterator endpoint_iterator)
        : io_service_(io_service),
        socket_(io_service),
        endpoint_iterator_(endpoint_iterator),
        read_msg_(),
        write_msgs_(),
        udp_sockets_(), ports(),
        connected(false), first_do(false),
        sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
    {
        do_connect();
    }

    void send(const uint8_t *data, uint16_t size, const Address &address, uint16_t socketPort)
    {
        message msg;
        msg.body_length(size);
        msg.dest_port(address.type == Address::Type::IPv4 ? ntohs(address.v4addr.sin_port) : ntohs(address.v6addr.sin6_port));
        msg.src_port(ports[socketPort]);
        msg.write(data, size);
        msg.encode_header();

        io_service_.post(boost::bind(&tcp_client::do_write, this, msg));
    }

    void close()
    {
        io_service_.post(boost::bind(&tcp_client::do_close, this));
    }

    uint16_t create_pipe(uint16_t server_port)
    {
        auto it = udp_sockets_.find(server_port);
        if (it != udp_sockets_.end())
        {
            sysLog->info("TCPClient return exist pipe UDP[{0}] -> TCP[{1}]", it->second->GetBindedPort(), server_port);
            return it->second->GetBindedPort();
        }

        udp_socket_ptr s(new UDPSocket(std::bind(&tcp_client::send, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
        s->Start();

        udp_sockets_.insert(std::pair<uint16_t, udp_socket_ptr>(server_port, s));

        ports[s->GetBindedPort()] = server_port;

        sysLog->info("TCPClient created pipe UDP[{0}] -> TCP[{1}]", s->GetBindedPort(), server_port);

        return s->GetBindedPort();
    }

 private:
    void handle_connect(const boost::system::error_code& error)
    {
        if (!error)
        {
            boost::system::error_code error_;
            socket_.set_option(tcp::no_delay(true), error_);

            boost::asio::async_read(socket_,
                 boost::asio::buffer(read_msg_.data(), message::header_length),
                 boost::bind(&tcp_client::handle_read_header, this,
                     boost::asio::placeholders::error));

            connected = true;

            sysLog->info("TCPClient connected");
        }
        else
        {
            errLog->critical("TCPClient connect error, reconnect, error: {0}", error.message());
            do_connect();
        }
    }
    
    void handle_read_header(const boost::system::error_code& error)
    {
        if (!error && read_msg_.decode_header())
        {
            boost::asio::async_read(socket_,
                boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                boost::bind(&tcp_client::handle_read_body, this,
                    boost::asio::placeholders::error));

            if (TCPClient::WITH_TRACES)
            {
                sysLog->trace("TCPClient readed header, len: {0}, src port: {1}, dest port: {2}", 
                    read_msg_.body_length(), read_msg_.src_port(), read_msg_.dest_port());
            }
        }
        else
        {
            errLog->critical("TCPClient handle_read_header error, sock closed, error: {0}", error.message());
            do_close();
        }
    }

    void handle_read_body(const boost::system::error_code& error)
    {
        if (!error)
        {
            auto it = udp_sockets_.find(read_msg_.src_port());
            if (it != udp_sockets_.end())
            {
                it->second->Send((const uint8_t*)read_msg_.body(), read_msg_.body_length(), Address("127.0.0.1", read_msg_.dest_port()), 0);

                if (TCPClient::WITH_TRACES)
                {
                    sysLog->trace("TCPClient receive and send to UDP, len: {0}, src port: {1}, dst port {2}", 
                       read_msg_.body_length(), read_msg_.src_port(), read_msg_.dest_port());
                }
            }

            boost::asio::async_read(socket_,
                boost::asio::buffer(read_msg_.data(), message::header_length),
                boost::bind(&tcp_client::handle_read_header, this,
                    boost::asio::placeholders::error));
        }
        else
        {
            errLog->critical("TCPClient handle_read_body error, sock closed, error: {0}", error.message());
            do_close();
        }
    }

    void do_write(message msg)
    {
        bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(msg);
        
        if (!first_do)
        {
            if (connected)
            {
                first_do = true;
                write_next_from_queue();
            }
        }
        else
        {
            if (!write_in_progress)
            {
                write_next_from_queue();
            }
        }
    }

    void write_next_from_queue()
    {
        if (!write_msgs_.empty())
        {         
            boost::asio::async_write(socket_,
                boost::asio::buffer(write_msgs_.front().data(),
                    write_msgs_.front().length()),
                boost::bind(&tcp_client::handle_write, this,
                    boost::asio::placeholders::error));
            
            if (TCPClient::WITH_TRACES)
            {
                sysLog->trace("TCPClient async writed msg, len: {0}, src port: {1}, dest port: {2}",
                    write_msgs_.front().body_length(), write_msgs_.front().src_port(), write_msgs_.front().dest_port());
            }
        }
    }

    void handle_write(const boost::system::error_code& error)
    {
        if (!error)
        {
            write_msgs_.pop_front();

            write_next_from_queue();
            
            if (TCPClient::WITH_TRACES)
            {
                sysLog->trace("TCPClient write from UDP, len: {0}, src port: {1}, dst port {2}", 
                    write_msgs_.front().body_length(), write_msgs_.front().src_port(), write_msgs_.front().dest_port());
            }
        }
        else
        {
            errLog->critical("TCPClient handle_write error, sock closed, error: {0}", error.message());
            do_close();
        }
    }

    void do_connect()
    {
        sysLog->info("TCPClient connecting");

        boost::asio::async_connect(socket_, endpoint_iterator_,
            boost::bind(&tcp_client::handle_connect, this,
                boost::asio::placeholders::error));
    }
    
    void do_close()
    {
        socket_.close();
        sysLog->info("TCPClient closed");
    }
};

class TCPClientImpl
{
    std::string address;
    uint16_t port;

    std::unique_ptr <boost::asio::io_service> io_service;

    std::unique_ptr<tcp_client> tcp_client_;

    std::thread thread_;
public:
    TCPClientImpl()
        : address(), port(0),
        io_service(),
        tcp_client_(),
        thread_()
    {}
    
    ~TCPClientImpl()
    {
        EndSession();
    }

    void SetServerAddress(const std::string &address_, uint16_t port_)
    {
        address = address_;
        port = port_;

        spdlog::get("System")->trace("TCPClient Server address {0}:{1}", address_, port);
    }

    uint16_t CreatePipe(uint16_t serverPort)
    {
        if (!tcp_client_)
        {
            io_service = std::unique_ptr<boost::asio::io_service>(new boost::asio::io_service());

            tcp::resolver resolver(*io_service);
            tcp::resolver::query query(address, std::to_string(port));
            tcp::resolver::iterator iterator = resolver.resolve(query);

            tcp_client_ = std::unique_ptr<tcp_client>(new tcp_client(*io_service, iterator));
            thread_ = std::thread([this]() { io_service->run(); });
        }

        return tcp_client_->create_pipe(serverPort);
    }

    void EndSession()
    {
        if (tcp_client_)
        {
            tcp_client_->close();

            thread_.join();

            tcp_client_.reset(nullptr);
            io_service.reset(nullptr);
        }
    }
};

TCPClient::TCPClient()
    : impl(new TCPClientImpl())
{
}
TCPClient::~TCPClient()
{
    impl.reset();
}

void TCPClient::SetServerAddress(const std::string &address_, uint16_t port_)
{
    impl->SetServerAddress(address_, port_);
}

void TCPClient::EndSession()
{
    impl->EndSession();
}

uint16_t TCPClient::CreatePipe(uint16_t serverPort)
{
    return impl->CreatePipe(serverPort);
}

}
