/**
 * HttpClient.h - Contains http client's header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#pragma once

#include <memory>
#include <functional>

namespace Transport
{

class IImpl;

class HTTPClient
{
public:
    HTTPClient(std::function<void(int32_t, std::string_view)> errorCallback = [](int32_t, std::string_view) {});
	~HTTPClient();

	void Connect(std::string_view url);
	std::string Request(std::string_view target, std::string_view method, std::string_view body = "");
	void Disconnect();

	bool IsConnected();
private:
	std::unique_ptr<IImpl> impl;
    std::function<void(int32_t, std::string_view)> errorCallback;
};

}
