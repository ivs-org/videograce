/**
 * RuntimeMeter.cpp - Contains the runtime meters source
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2024
 */

#pragma once

#include <RuntimeMeter/RuntimeMeter.h>

#include <chrono>

namespace Common
{

RuntimeMeter::RuntimeMeter(int64_t triggerMS_,
	std::function<void(int64_t)> callback_,
	Transport::ISocket &receiver_)
	: triggerMS(triggerMS_), callback(callback_), receiver(receiver_)
{
}

RuntimeMeter::~RuntimeMeter()
{
}

void RuntimeMeter::Send(const Transport::IPacket& packet_, const Transport::Address*)
{
	auto start = std::chrono::high_resolution_clock::now();

	receiver.Send(packet_);

	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();

	if (duration > triggerMS)
	{
		callback(duration);
	}
}

}
