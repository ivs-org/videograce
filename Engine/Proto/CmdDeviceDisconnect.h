/**
 * CmdDeviceDisconnect.h - Contains protocol command DEVICE_DISCONNECT
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#pragma once

#include <string>
#include <cstdint>

#include <Proto/DeviceType.h>

namespace Proto
{
namespace DEVICE_DISCONNECT
{
	static const std::string NAME = "device_disconnect";

	struct Command
	{
		DeviceType device_type;
		uint32_t device_id;
		int64_t client_id;

		Command();
		Command(DeviceType device_type, uint32_t device_id, int64_t client_id);

		~Command();

		bool Parse(const std::string &message);
		std::string Serialize();
	};
}
}
