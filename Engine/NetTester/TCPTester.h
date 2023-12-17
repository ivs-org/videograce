/**
 * TCPTester.h - Contains tcp media availability tester header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <atomic>
#include <cstdint>
#include <string>
#include <memory>
#include <thread>
#include <functional>

#include <wui/locale/i_locale.hpp>

#include <spdlog/spdlog.h>

namespace NetTester
{

class tcp_client;

class TCPTester
{
public:
	TCPTester(std::shared_ptr<wui::i_locale> locale, std::function<void()> readyCallback);
	~TCPTester();

	void SetAddress(const std::string &address, uint16_t port);
	void ClearAddresses();

	void DoTheTest();
	void Stop();

	bool TestPassed() const;

	std::string GetErrorMessage() const;

private:
    std::shared_ptr<wui::i_locale> locale;

    std::function<void()> readyCallback;

	std::atomic<bool> runned;

	std::string address;
	uint16_t port;

	bool isOK;

	std::shared_ptr<spdlog::logger> sysLog, errLog;

	std::thread thread;
};

}
