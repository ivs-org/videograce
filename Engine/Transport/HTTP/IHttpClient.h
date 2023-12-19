/**
 * IHttpClient.h - Contains http client's interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#pragma once

#include <string>

namespace Transport
{

class IHTTPClient
{
public:
	virtual void Connect(std::string_view url) = 0;
	virtual std::string Request(std::string_view target, std::string_view method, std::string_view body = "") = 0;
	virtual void Disconnect() = 0;

	virtual bool IsConnected() = 0;

protected:
	virtual ~IHTTPClient() {}
};

}
