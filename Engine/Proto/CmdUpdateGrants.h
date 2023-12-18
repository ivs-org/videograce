/**
 * CmdUpdateGrants.h - Contains protocol command UPDATE_GRANTS
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <string>

namespace Proto
{
namespace UPDATE_GRANTS
{
	static const std::string NAME = "update_grants";

	struct Command
	{
		uint32_t grants;

		Command();
		Command(uint32_t grants);
		
		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
