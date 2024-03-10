/**
 * RuntimeMeter.cpp - Contains the runtime meters source
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2024
 */

#include <Common/RuntimeMeter.h>

//#define SUBTLE_TRACE 1
#include <Common/Common.h>

#include <chrono>

namespace Common
{

RuntimeMeter::RuntimeMeter(int64_t triggerMS_,
	std::function<void(int64_t)> callback_,
	Transport::ISocket &receiver_)
	: triggerMS(triggerMS_), callback(callback_), receiver(receiver_), statMeter(50)
{
}

void RuntimeMeter::Send(const Transport::IPacket& packet_, const Transport::Address*)
{
	auto start = std::chrono::high_resolution_clock::now();

	receiver.Send(packet_);

	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();

	if (duration == 0)
	{
		return;
	}

	statMeter.PushVal(duration);

	//subtle_trace("statmeter duration: ", duration);

	if (statMeter.GetFill() == 40 && statMeter.GetAvg() > triggerMS)
	{
		//subtle_trace("statmeter triggered, avg: ", statMeter.GetAvg());

		statMeter.Clear();
		
		callback(duration);
	}
}

}
