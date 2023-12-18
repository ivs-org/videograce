/**
 * CmdResolutionChange.h - Contains protocol command RESOLUTION_CHANGE
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#pragma once

#include <string>
#include <cstdint>

namespace Proto
{
namespace RESOLUTION_CHANGE
{
	static const std::string NAME = "resolution_change";

	struct Command
	{
		uint32_t id;
		uint32_t resolution;

		Command();
		Command(uint32_t id, uint32_t resolution);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
