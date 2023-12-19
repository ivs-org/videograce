/**
 * CmdScheduleConnect.h - Contains protocol command SCHEDULE_CONNECT
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#pragma once

#include <string>
#include <cstdint>

namespace Proto
{
namespace SCHEDULE_CONNECT
{
	static const std::string NAME = "schedule_connect";

	struct Command
	{
		std::string tag;
		std::string name;
		uint64_t time_limit;

		Command();
		Command(std::string_view conference, std::string_view conference_name, uint64_t time_limit);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
