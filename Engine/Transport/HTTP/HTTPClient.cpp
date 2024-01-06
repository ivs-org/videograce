/**
 * HttpClient.cpp - Contains http client's impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020, 2023
 */

#include <Version.h>

#include <Transport/URI/URI.h>
#include <Transport/HTTP/HttpClient.h>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>

#include <cstdlib>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>

#define VG_CLIENT_VERSION_STRING SYSTEM_NAME ".Client/" SYSTEM_VERSION

namespace Transport
{

class IImpl
{
public:
	virtual std::string Request(std::string_view target, std::string_view method, std::string_view body) = 0;
	virtual bool IsConnected() = 0;

	virtual ~IImpl() {}
};

class HttpImpl : public IImpl
{
public:
	std::string host, port;

	boost::asio::io_context ioc;
	tcp::socket socket;

    std::function<void(int32_t, std::string_view)> errorCallback;

	std::shared_ptr<spdlog::logger> sysLog, errLog;
	
	HttpImpl(std::string_view url, std::function<void(int32_t, std::string_view)> errorCallback_)
		: host(), port(),
		ioc(),
		socket(ioc),
		errorCallback(errorCallback_),
		sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
	{
		std::string proto;
		ParseURI(url, proto, host, port);

		if (!host.empty())
		{
			tcp::resolver resolver(ioc);
			
			port = port.empty() ? "80" : port;

			sysLog->trace("HttpImpl::HttpImpl :: Perform connect to: {0}", url);

			boost::system::error_code ec;
			auto const results = resolver.resolve(host, port, ec);
			if (ec)
			{
				errLog->error("HttpImpl::HttpImpl :: resolver error: (v: {0:d}, m:{1})", ec.value(), ec.message());
				if (errorCallback) errorCallback(ec.value(), ec.message().c_str());
				return;
			}

			boost::asio::connect(socket, results.begin(), results.end(), ec);
			if (ec && errorCallback)
			{
				errLog->error("HttpImpl::HttpImpl :: connect error: (v: {0:d}, m:{1})", ec.value(), ec.message());
                errorCallback(ec.value(), ec.message().c_str());
			}

			sysLog->trace("HttpImpl::HttpImpl :: Connected to: {0}", url);
		}
	}

	virtual ~HttpImpl()
	{
		boost::system::error_code ec;
		socket.shutdown(tcp::socket::shutdown_both, ec);
		if (ec.value() != 0 && ec != boost::asio::error::eof && errorCallback)
		{
			errLog->error("HttpImpl::~HttpImpl :: shutdown error: (v: {0:d}, m:{1})", ec.value(), ec.message());
            errorCallback(ec.value(), ec.message().c_str());
		}

		sysLog->trace("HttpImpl::~HttpImpl :: Ended");
	}

	virtual std::string Request(std::string_view target, std::string_view method, std::string_view body)
	{
		http::request<http::string_body> req{method == "POST" ? http::verb::post : http::verb::get, target, 11};
		req.set(http::field::host, host + ":" + port);
		req.set(http::field::user_agent, VG_CLIENT_VERSION_STRING);
		req.body() = body;
		req.content_length(body.size());
		req.prepare_payload();

		boost::system::error_code ec;

		http::write(socket, req, ec);
		if (ec)
		{
			errLog->error("HttpImpl::Request :: http::write error: (v: {0:d}, m:{1})", ec.value(), ec.message());
			if (errorCallback) errorCallback(ec.value(), ec.message().c_str());
			return "";
		}

		boost::beast::flat_buffer buffer;
		http::response_parser<http::string_body> res;
		res.body_limit(1024 * 1024 * 20); // 20 Mb
				
		http::read(socket, buffer, res, ec);
		if (ec)
		{
			errLog->error("HttpImpl::Request :: http::read error: (v: {0:d}, m:{1})", ec.value(), ec.message());
			if (errorCallback) errorCallback(ec.value(), ec.message().c_str());
			return "";
		}

		return res.get().body();
	}

	virtual bool IsConnected()
	{
		return socket.is_open();
	}
};

class HttpsImpl : public IImpl
{
public:
	std::string host, port;

	boost::asio::io_context ioc;
	ssl::context ctx;
	ssl::stream<tcp::socket> stream;

    std::function<void(int32_t, std::string_view)> errorCallback;

	std::shared_ptr<spdlog::logger> sysLog, errLog;

	HttpsImpl(std::string_view url, std::function<void(int32_t, std::string_view)> errorCallback_)
		: host(), port(),
		ioc(),
		ctx{ ssl::context::sslv23_client },
		stream{ ioc, ctx },
		errorCallback(errorCallback_),
		sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
	{
		std::vector<std::string> vals, addr;
		std::string query(url);
		boost::split(vals, query, boost::is_any_of("/"));
		if (vals.size() >= 2)
		{
			boost::split(addr, vals[2], boost::is_any_of(":"));
			if (!addr.empty())
			{
				tcp::resolver resolver(ioc);
				host = addr[0];
				port = addr.size() > 1 ? addr[1] : "443";

				sysLog->trace("HttpsImpl::HttpsImpl :: Perform connect to: {0}", url);
								
				// Set SNI Hostname (many hosts need this to handshake successfully)
				if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str()) && errorCallback)
				{
					boost::system::error_code ec{ static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category() };
					errLog->error("HttpsImpl::HttpsImpl :: SSL_set_tlsext_host_name: (v: {0:d}, m:{1})", ec.value(), ec.message());
                    errorCallback(ec.value(), ec.message().c_str());
				}

				boost::system::error_code ec;

				auto const results = resolver.resolve(host, port, ec);
				if (ec)
				{
					errLog->error("HttpsImpl::HttpsImpl :: resolver error: (v: {0:d}, m:{1})", ec.value(), ec.message());
					if (errorCallback) errorCallback(ec.value(), ec.message().c_str());
					return;
				}

				boost::asio::connect(stream.next_layer(), results.begin(), results.end(), ec);
				if (ec)
				{
					errLog->error("HttpsImpl::HttpsImpl :: connect error: (v: {0:d}, m:{1})", ec.value(), ec.message());
					if (errorCallback) errorCallback(ec.value(), ec.message().c_str());
					return;
				}

				stream.handshake(ssl::stream_base::client, ec);
				if (ec)
				{
					errLog->error("HttpsImpl::HttpsImpl :: handshake error: (v: {0:d}, m:{1})", ec.value(), ec.message());
					if (errorCallback) errorCallback(ec.value(), ec.message().c_str());
					return;
				}

				sysLog->trace("HttpsImpl::HttpsImpl :: Connected to: {0}", url);
			}
		}
	}

	virtual ~HttpsImpl()
	{
		boost::system::error_code ec;
		stream.shutdown(ec);
		if (ec.value() != 0 && ec != boost::asio::error::eof && errorCallback)
		{
			errLog->error("HttpsImpl::~HttpsImpl :: shutdown error: (v: {0:d}, m:{1})", ec.value(), ec.message());
            errorCallback(ec.value(), ec.message().c_str());
		}
		sysLog->trace("HttpsImpl::~HttpsImpl :: Ended");
	}

	virtual std::string Request(std::string_view target, std::string_view method, std::string_view body)
	{
		http::request<http::string_body> req{ method == "POST" ? http::verb::post : http::verb::get, target, 11 };
		req.set(http::field::host, host + ":" + port);
		req.set(http::field::user_agent, VG_CLIENT_VERSION_STRING);
		req.body() = body;
		req.content_length(body.size());
		req.prepare_payload();

		boost::system::error_code ec;

		http::write(stream, req, ec);
		if (ec)
		{
			errLog->error("HttpsImpl::Request :: http::write error: (v: {0:d}, m:{1})", ec.value(), ec.message());
			if (errorCallback) errorCallback(ec.value(), ec.message().c_str());
			return "";
		}

		boost::beast::flat_buffer buffer;
		http::response_parser<http::string_body> res;
		res.body_limit(1024 * 1024 * 100); // 100 Mb
		
		http::read(stream, buffer, res, ec);
		if (ec)
		{
			errLog->error("HttpsImpl::Request :: http::read error: (v: {0:d}, m:{1})", ec.value(), ec.message());
			if (errorCallback) errorCallback(ec.value(), ec.message().c_str());
			return "";
		}

		return res.get().body();
	}

	virtual bool IsConnected()
	{
		return stream.next_layer().is_open();
	}
};

HTTPClient::HTTPClient(std::function<void(int32_t, std::string_view)> errorCallback_)
	: impl(), errorCallback(errorCallback_)
{
}

HTTPClient::~HTTPClient()
{
	Disconnect();
}

void HTTPClient::Connect(std::string_view url)
{
	if (impl)
	{
		return;
	}

	if (url.find("https") != std::string::npos)
	{
		impl = std::unique_ptr<HttpsImpl>(new HttpsImpl(url, errorCallback));
	}
	else
	{
		impl = std::unique_ptr<HttpImpl>(new HttpImpl(url, errorCallback));
	}
}

std::string HTTPClient::Request(std::string_view path, std::string_view method, std::string_view body)
{
	if (impl)
	{
		return impl->Request(path, method, body);
	}

	return "";
}

void HTTPClient::Disconnect()
{
	impl.reset(nullptr);
}

bool HTTPClient::IsConnected()
{
	if (impl)
	{
		return impl->IsConnected();
	}
	return false;
}

}
