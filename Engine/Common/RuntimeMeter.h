/**
 * RuntimeMeter.h - Contains the runtime meters header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2024
 */

#pragma once

#include <Transport/ISocket.h>
#include <Common/StatMeter.h>
#include <functional>

namespace Common
{

class RuntimeMeter : public Transport::ISocket
{
public:
	RuntimeMeter(int64_t triggerMS,
		std::function<void(int64_t)> callback,
		Transport::ISocket &receiver);

	/// Receive from source and send to receiver
	virtual void Send(const Transport::IPacket &packet_, const Transport::Address *address = nullptr) final;
private:
	int64_t triggerMS;
	std::function<void(int64_t)> callback;
	Transport::ISocket& receiver;

	StatMeter statMeter;
};

}
