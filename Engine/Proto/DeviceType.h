/**
 * DeviceType.h - Contains the DeviceType enumeration
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

namespace Proto
{
	enum class DeviceType
	{
		Undefined = 0,

		Camera,
		Demonstration,
		Avatar,

		Microphone,

		VideoRenderer,
		AudioRenderer
	};
}
