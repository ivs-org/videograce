/**
 * UDPSocket.h - Contains UDP socket interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018, 2023
 */

#pragma once

#include <Transport/Address.h>
#include <Transport/SockCommon.h>

#include <atomic>
#include <functional>

#include <spdlog/spdlog.h>

namespace Transport
{

class UDPSocket
{
public:	
	UDPSocket(std::function<void(const uint8_t *data, uint16_t size, const Address &address, uint16_t socketPort)> receiver);
	~UDPSocket();
	
	void Start(Address::Type type = Address::Type::IPv4, uint16_t bindPort = 0);
	void Stop();

	void Send(const uint8_t *data, uint16_t size, const Address &address, uint16_t socketPort);

	bool Runned() const;
	uint64_t GetSocketNumber() const;
	uint16_t GetBindedPort() const;

	static constexpr uint16_t MAX_DATAGRAM_SIZE = 32768;

	static constexpr bool WITH_TRACES = false;

private:
	std::function<void(const uint8_t *data, uint16_t size, const Address &address, uint16_t socketPort)> receiver;
	
	SOCKET netsocket;

	uint16_t bindedPort;
	
	std::thread thread;

	std::atomic<bool> runned;

	std::shared_ptr<spdlog::logger> sysLog, errLog;

	void run();
};

}
