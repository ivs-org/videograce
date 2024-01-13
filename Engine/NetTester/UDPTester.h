/**
 * UDPTester.h - Contains udp availability tester header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <atomic>
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <functional>

#include <wui/locale/i_locale.hpp>

#include <Transport/RTPSocket.h>

#include <Transport/Address.h>

#include <mt/timer.h>

#include <spdlog/spdlog.h>

namespace NetTester
{

class UDPTester : public Transport::ISocket
{
public:
    UDPTester(std::shared_ptr<wui::i_locale> locale, std::function<void()> readyCallback = []() {});
	~UDPTester();

	void AddAddress(std::string_view address, uint16_t port);
	void ClearAddresses();

	void DoTheTest();
	void Stop();

	bool TestPassed() const;

	std::string GetErrorMessage() const;

private:
    std::shared_ptr<wui::i_locale> locale;

    std::function<void()> readyCallback;

	std::mutex mutex;

	uint16_t iteration; /// Recheck after (65565 / 2) seconds

	mt::timer timer;
	
	std::vector<Transport::Address> inputAddresses, availAddresses;
	std::vector<std::shared_ptr<Transport::RTPSocket>> rtpSockets;

	std::shared_ptr<spdlog::logger> sysLog, errLog;
	
	virtual void Send(const Transport::IPacket &packet, const Transport::Address *address = nullptr);

	void onTimer();
};

}
