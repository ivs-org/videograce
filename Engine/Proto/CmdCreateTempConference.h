/**
 * CmdCreateConference.h - Contains protocol command CREATE_TEMP_CONFERENCE
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <string>
#include <cstdint>

namespace Proto
{
namespace CREATE_TEMP_CONFERENCE
{
	static const std::string NAME = "create_temp_conference";

	struct Command
	{
		std::string tag;
		
		Command();
		Command(std::string_view tag);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
