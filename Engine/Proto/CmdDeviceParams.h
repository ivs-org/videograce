/**
 * CmdDeviceParams.h - Contains protocol command DEVICE_PARAMS
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <string>
#include <cstdint>

#include <Proto/DeviceType.h>

#include <Video/ColorSpace.h>

namespace Proto
{
namespace DEVICE_PARAMS
{
	static const std::string NAME = "device_params";

	struct Command
	{
		uint32_t id;
		uint32_t ssrc;

		DeviceType device_type;
		uint32_t ord;
		std::string name;
		std::string metadata;
		uint32_t resolution;
		Video::ColorSpace color_space;

		Command();
		Command(uint32_t id, uint32_t ssrc, DeviceType device_type, uint32_t ord, const std::string &name, const std::string &metadata, uint32_t resolution, Video::ColorSpace color_space);

		~Command();

		bool Parse(const std::string &message);
		std::string Serialize();
	};
}
}
