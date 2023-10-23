/**
 * DeviceNotifies.h - Contains interface of device notifies receiver
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014, 2018, 2022
 */

#pragma once

#include <cstdint>
#include <string>

#include <Proto/DeviceType.h>

#include <functional>

namespace Client
{

enum class DeviceNotifyType
{
	Undefined = 0,

	CameraError,
	MicrophoneError,

	DeviceEnded,

	MemoryError,
	OtherError,

	OvertimeCoding,
	OvertimeRendering,

	ResolutionChanged,

	MicrophoneSpeak,
	MicrophoneSilent
};

typedef std::function<void(const std::string &name, DeviceNotifyType notifyType, Proto::DeviceType deviceType, uint32_t deviceId, int32_t iData)> DeviceNotifyCallback;

}
