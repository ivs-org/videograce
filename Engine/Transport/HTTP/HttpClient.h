/**
 * HttpClient.h - Contains http client's header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#pragma once

#include <memory>
#include <functional>

#include "IHttpClient.h"

namespace Transport
{

class IImpl;

class HTTPClient : public IHTTPClient
{
public:
    HTTPClient(std::function<void(int32_t, const char*)> errorCallback = [](int32_t, const char*) {});
	virtual ~HTTPClient();

	virtual void Connect(std::string_view url);
	virtual std::string Request(std::string_view target, std::string_view method, std::string_view body = "");
	virtual void Disconnect();

	virtual bool IsConnected();
private:
	std::unique_ptr<IImpl> impl;
    std::function<void(int32_t, const char*)> errorCallback;
};

}
