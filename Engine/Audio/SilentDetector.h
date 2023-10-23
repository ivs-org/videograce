/**
 * SilentDetector.h - Contains the silent detector interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#pragma once

#include <Transport/ISocket.h>
#include <cstdint>

namespace Audio
{

enum class SilentMode
{
	Undefined = 0,

	Silent,
	Speak
};

class SilentDetectorCallback
{
public:
	virtual void SilentChanged(SilentMode silentMode) = 0;
};

class SilentDetector : public Transport::ISocket
{
public:
	SilentDetector(SilentDetectorCallback &callback);
	~SilentDetector();

	/// Input
	virtual void Send(const Transport::IPacket &packet, const Transport::Address *address = nullptr) final;

private:
	static const uint32_t MESAURE_TIME_LIMIT = 3000; // 3 second
	static const uint64_t SPEAK_POWER_VALUE = 650000 * 3;

	SilentDetectorCallback &callback;

	uint64_t currentPower;
	uint32_t currentMeasureTime;
	SilentMode currentMode;
};

}
