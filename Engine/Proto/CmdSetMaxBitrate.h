/**
 * CmdSetMaxBitrate.h - Contains protocol command SET_MAX_BITRATE
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <string>
#include <cstdint>

namespace Proto
{
namespace SET_MAX_BITRATE
{
	static const std::string NAME = "set_max_bitrate";

	struct Command
	{
		uint32_t bitrate;
		
		Command();
		Command(uint32_t bitrate);

		~Command();

		bool Parse(const std::string &message);
		std::string Serialize();
	};
}
}
