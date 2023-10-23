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
	virtual void Connect(const std::string &url) = 0;
	virtual std::string Request(const std::string &target, const std::string &method, const std::string &body = "") = 0;
	virtual void Disconnect() = 0;

	virtual bool IsConnected() = 0;

protected:
	virtual ~IHTTPClient() {}
};

}
