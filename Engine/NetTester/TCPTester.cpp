/**
 * TCPTester.cpp - Contains tcp media availability tester impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <NetTester/TCPTester.h>

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

#include <Transport/RTP/RTCPPacket.h>

#include <Common/Common.h>

namespace NetTester
{

using boost::asio::ip::tcp;

class tcp_client
{
	boost::asio::io_service& io_service_;
	tcp::socket socket_;

	bool is_ok_;
public:
	tcp_client(boost::asio::io_service& io_service,
		tcp::resolver::iterator endpoint_iterator)
		: io_service_(io_service),
		socket_(io_service),
		is_ok_(false)
	{
		boost::asio::async_connect(socket_, endpoint_iterator,
			boost::bind(&tcp_client::handle_connect, this,
				boost::asio::placeholders::error));
	}

	void close()
	{
		io_service_.post(boost::bind(&tcp_client::do_close, this));
	}

	bool is_ok() const
	{
		return is_ok_;
	}

private:
	void handle_connect(const boost::system::error_code& error)
	{
		is_ok_ = !error;
	}

	void do_close()
	{
		socket_.close();
	}
};

TCPTester::TCPTester(std::shared_ptr<wui::i_locale> locale_, std::function<void()> readyCallback_)
	: locale(locale_),
    readyCallback(readyCallback_),
	runned(false),
	address(), port(0),
	isOK(false),
	thread()
{
}

TCPTester::~TCPTester()
{
	runned = false;
	if (thread.joinable()) thread.join();
}

void TCPTester::SetAddress(const std::string &address_, uint16_t port_)
{
	address = address_;
	port = port_;
}

void TCPTester::ClearAddresses()
{
	address.clear();
	port = 0;
}

void TCPTester::DoTheTest()
{
	if (!runned && !address.empty())
	{
		if (thread.joinable()) thread.join();

		thread = std::thread([this]() {
			runned = true;

			boost::asio::io_service io_service;

			tcp::resolver resolver(io_service);
			tcp::resolver::query query(address, std::to_string(port));
			tcp::resolver::iterator iterator = resolver.resolve(query);

			tcp_client c(io_service, iterator);

			std::thread t([&io_service]() { try { io_service.run(); } catch (...) {} });
			t.detach();

			/// Wait the 5 seconds for responses
			auto cnt = 0;
			while (runned && cnt < 5000 / 200)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
				++cnt;
			}
			
			isOK = c.is_ok();

			io_service.stop();

			if (runned)
			{
                readyCallback();
			}

			runned = false;
		});
	}
}

bool TCPTester::TestPassed() const
{
    return isOK;
}

std::string TCPTester::GetErrorMessage() const
{
	std::string errorMessage = locale->get("net_test", "tcp_socket_unavailable") + " " +
		address + ":" + std::to_string(port) +
		"\n";
	return errorMessage;
}

}
