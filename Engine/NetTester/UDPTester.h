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
#include <functional>
#include <thread>

#include <wui/locale/i_locale.hpp>

#include <Transport/ISocket.h>

#include <Transport/Address.h>

namespace NetTester
{

class UDPTester : public Transport::ISocket
{
public:
    UDPTester(std::shared_ptr<wui::i_locale> locale, std::function<void()> readyCallback = []() {});
	~UDPTester();

	void AddAddress(const char* address, uint16_t port);
	void ClearAddresses();

	void DoTheTest();

	bool TestPassed() const;

	std::string GetErrorMessage() const;

private:
	struct Connection
	{
		Transport::Address address;

		bool available;

		Connection()
			: address(), available(false) {}
		Connection(Transport::Address address_)
			: address(address_), available(false) {}

		inline bool operator==(const Connection& lv)
		{
			return address == lv.address;
		}
	};

    std::shared_ptr<wui::i_locale> locale;

    std::function<void()> readyCallback;

	std::atomic<bool> runned;

	std::thread thread;

	std::vector<Connection> connections;
	
	virtual void Send(const Transport::IPacket &packet, const Transport::Address *address = nullptr);
};

}
