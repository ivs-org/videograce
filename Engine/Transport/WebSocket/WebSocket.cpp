/**
 * WebSocket.cpp - Contains web socket's impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include "WebSocket.h"

#include <Transport/URI/URI.h>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>

#include <mutex>
#include <queue>
#include <cstdlib>
#include <thread>
#include <atomic>

using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;               // from <boost/asio/ssl.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>

namespace Transport
{

class session : public std::enable_shared_from_this<session>
{
	tcp::resolver resolver_;

	boost::asio::io_context& ioc_;

	websocket::stream<tcp::socket> plain_ws_;
	ssl::context ctx;
	websocket::stream<ssl::stream<tcp::socket>> ssl_ws_;
	
	boost::beast::multi_buffer buffer_;
	std::string host_;

	std::mutex write_mutex_;
	std::queue<std::string> write_queue_;
	std::atomic<bool> writing_;

	IWebSocketCallback &callback_;

	bool secure;

public:
	// Resolver and socket require an io_context
	explicit session(boost::asio::io_context& ioc, IWebSocketCallback &callback, bool secure_)
		: resolver_(ioc),
		ioc_(ioc),
		plain_ws_(ioc),
		ctx(ssl::context::sslv23_client),
		ssl_ws_(ioc, ctx),
		buffer_(),
		host_(),
		write_mutex_(),
		write_queue_(),
		writing_(false),
		callback_(callback),
		secure(secure_)
	{
	}

	// Start the asynchronous operation
	void run(char const* host, char const* port)
	{
		// Save these for later
		host_ = host;
		
		// Look up the domain name
		resolver_.async_resolve(
			host,
			port,
			std::bind(
				&session::on_resolve,
				shared_from_this(),
				std::placeholders::_1,
				std::placeholders::_2));
	}
	
	void on_resolve(boost::system::error_code ec, tcp::resolver::results_type results)
	{
		if (ec && !ioc_.stopped())
		{
			callback_.OnWebSocket(WSMethod::Error, "on_resolve " + ec.message());
			return;
		}
		
		// Make the connection on the IP address we get from a lookup
		boost::asio::async_connect(
			secure ? ssl_ws_.next_layer().next_layer() : plain_ws_.next_layer(),
			results.begin(),
			results.end(),
			std::bind(
				&session::on_connect,
				shared_from_this(),
				std::placeholders::_1));
	}
	
	void on_connect(boost::system::error_code ec)
	{
		if (ec && !ioc_.stopped())
		{
			callback_.OnWebSocket(WSMethod::Error, "on_connect " + ec.message());
			return;
		}
		
		if (secure)
		{
			// Perform the SSL handshake
			ssl_ws_.next_layer().async_handshake(
				ssl::stream_base::client,
				std::bind(
					&session::on_ssl_handshake,
					shared_from_this(),
					std::placeholders::_1));
		}
		else
		{
			// Perform the websocket handshake
			plain_ws_.async_handshake(host_, "/",
				std::bind(
					&session::on_handshake,
					shared_from_this(),
					std::placeholders::_1));
		}
	}
	
	void on_ssl_handshake(boost::system::error_code ec)
	{
		if (ec && !ioc_.stopped())
		{
			callback_.OnWebSocket(WSMethod::Error, "on_ssl_handshake " + ec.message());
			return;
		}

		// Perform the websocket handshake
		ssl_ws_.async_handshake(host_, "/",
			std::bind(
				&session::on_handshake,
				shared_from_this(),
				std::placeholders::_1));
	}

	void on_handshake(boost::system::error_code ec)
	{
		if (ec && !ioc_.stopped())
		{
			callback_.OnWebSocket(WSMethod::Error, "on_handshake " + ec.message());
			return;
		}

		callback_.OnWebSocket(WSMethod::Open, "");

		do_read();
	}

	void do_read()
	{
		if (secure)
		{
			ssl_ws_.async_read(
				buffer_,
				std::bind(
					&session::on_read,
					shared_from_this(),
					std::placeholders::_1,
					std::placeholders::_2));
		}
		else
		{
			plain_ws_.async_read(
				buffer_,
				std::bind(
					&session::on_read,
					shared_from_this(),
					std::placeholders::_1,
					std::placeholders::_2));
		}
	}

    void on_read(boost::system::error_code ec, std::size_t bytes_transferred)
	{
		boost::ignore_unused(bytes_transferred);
		if (ec && !ioc_.stopped())
		{
			callback_.OnWebSocket(WSMethod::Error, "on_read " + ec.message());
			return;
		}
		
		const std::string &readed = buffers_to_string(buffer_.data());
		buffer_.consume(buffer_.size());
		callback_.OnWebSocket(WSMethod::Message, readed);

		do_read();
	}

	void write(const std::string &message)
	{
		std::lock_guard<std::mutex> lock(write_mutex_);
		write_queue_.push(message);
		if (!writing_)
		{
			writing_ = true;
			do_write();
		}
	}

	void do_write()
	{
		std::string message = write_queue_.front();
		write_queue_.pop();

		if (secure)
		{
			ssl_ws_.text(true);
			ssl_ws_.async_write(
				boost::asio::buffer(message.data(), message.size()),
				std::bind(
					&session::on_write,
					shared_from_this(),
					std::placeholders::_1,
					std::placeholders::_2));
		}
		else
		{
			plain_ws_.text(true);
			plain_ws_.async_write(
				boost::asio::buffer(message.data(), message.size()),
				std::bind(
					&session::on_write,
					shared_from_this(),
					std::placeholders::_1,
					std::placeholders::_2));
		}
	}

	void on_write(boost::system::error_code ec,
		std::size_t bytes_transferred)
	{
		boost::ignore_unused(bytes_transferred);

		if (ec && !ioc_.stopped())
		{
			callback_.OnWebSocket(WSMethod::Error, "on_write " + ec.message());
			return;
		}

		std::lock_guard<std::mutex> lock(write_mutex_);
		if (!write_queue_.empty())
			do_write();
		else
			writing_ = false;
	}

	void close()
	{
		while (writing_) /// wait ending of writing
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		if (secure)
		{
			ssl_ws_.async_close(websocket::close_code::normal,
				std::bind(
					&session::on_close,
					shared_from_this(),
					std::placeholders::_1));
		}
		else
		{
			plain_ws_.async_close(websocket::close_code::normal,
				std::bind(
					&session::on_close,
					shared_from_this(),
					std::placeholders::_1));
		}
	}

	void on_close(boost::system::error_code ec)
	{
		ioc_.stop();

		callback_.OnWebSocket(WSMethod::Close, ec ? "error: " + ec.message() : "");
	}

	bool is_connected()
	{
		if (secure)
		{
			return ssl_ws_.is_open();
		}
		else
		{
			return plain_ws_.is_open();
		}
	}
};

class WebSocketImpl
{
	boost::asio::io_context ioc;
	
	IWebSocketCallback &callback;

	bool secure;
	
	std::string address, port;

	std::shared_ptr<session> session_;

	std::thread thread;
public:
	WebSocketImpl(const std::string &url, IWebSocketCallback &callback_)
		: ioc(),
		callback(callback_),
		secure(false),
		address(), port(),
		session_(),
		thread()
	{
		std::string proto;
		ParseURI(url, proto, address, port);

		if (!address.empty())
		{
			tcp::resolver resolver(ioc);

			if (port.empty())
			{
				port = (proto == "http" ? "80" : "443");
			}

			if (proto == "https")
			{
				secure = true;
			}

			thread = std::thread([this] {
				session_ = std::make_shared<session>(ioc, callback, secure);
				session_->run(address.c_str(), port.c_str());

				ioc.run();
			});
		}
	}

	void Send(const std::string &message)
	{
		if (session_)
		{
			session_->write(message);
		}
	}

	~WebSocketImpl()
	{
		if (session_ && session_->is_connected())
		{
            session_->close();
		}
		else
		{
			callback.OnWebSocket(WSMethod::Close, "session not established");
		}

		if (thread.joinable()) thread.join();
	}
	
	bool IsConnected()
	{
		if (session_)
		{
			return session_->is_connected();
		}
		return false;
	}
};

WebSocket::WebSocket(IWebSocketCallback &callback_)
	: implMutex(),
	  impl(),
	  callback(callback_)
{
}

WebSocket::~WebSocket()
{
	Disconnect();
}

void WebSocket::Connect(const std::string &url)
{
	std::lock_guard<std::recursive_mutex> lock(implMutex);
	if (impl)
	{
		impl.reset(nullptr);
	}
	impl = std::unique_ptr<WebSocketImpl>(new WebSocketImpl(url, callback));
}

void WebSocket::Send(const std::string &message)
{
	std::lock_guard<std::recursive_mutex> lock(implMutex);
	if (impl)
	{
		impl->Send(message);
	}
}

void WebSocket::Disconnect()
{
	std::lock_guard<std::recursive_mutex> lock(implMutex);
	impl.reset(nullptr);
}

bool WebSocket::IsConnected()
{
	std::lock_guard<std::recursive_mutex> lock(implMutex);
	if (impl)
	{
		return impl->IsConnected();
	}
	return false;
}

}
