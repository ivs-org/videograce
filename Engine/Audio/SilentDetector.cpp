/**
 * SilentDetector.cpp - Contains the silent detector impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#include <Audio/SilentDetector.h>

#include <Transport/RTP/RTPPacket.h>

#include <limits>

namespace Audio
{

SilentDetector::SilentDetector(SilentDetectorCallback &callback_)
	: callback(callback_),
	currentPower(0),
	currentMeasureTime(0),
	currentMode(SilentMode::Silent)
{
}

SilentDetector::~SilentDetector()
{
}
	
void SilentDetector::Send(const Transport::IPacket &packet_, const Transport::Address *)
{
	const auto &packet = *static_cast<const Transport::RTPPacket*>(&packet_);

	for (uint16_t i = 0; i != packet.payloadSize; i += 2)
	{
		auto sample = *reinterpret_cast<const int16_t*>(packet.payload + i);
		if (currentPower < std::numeric_limits<uint64_t>::max() - std::numeric_limits<uint16_t>::max() &&
			sample > 1000)
		{
			currentPower += sample;
		}
	}

	currentMeasureTime += 20;

	if (currentMeasureTime == MESAURE_TIME_LIMIT)
	{
		if ((currentPower >= SPEAK_POWER_VALUE && currentMode != SilentMode::Speak) ||
			(currentPower < SPEAK_POWER_VALUE && currentMode != SilentMode::Silent))
		{
			currentMode = currentPower >= SPEAK_POWER_VALUE ? SilentMode::Speak : SilentMode::Silent;
			callback.SilentChanged(currentMode);
		}

		currentPower = 0;
		currentMeasureTime = 0;
	}
}

}
