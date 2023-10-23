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

		bool Parse(const std::string &message);
		std::string Serialize();
	};
}
}
