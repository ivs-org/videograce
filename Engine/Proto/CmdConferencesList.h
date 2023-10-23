/**
 * CmdConferencesList.h - Contains protocol command CONFERENCES_LIST
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <string>
#include <cstdint>
#include <vector>

#include <Proto/Conference.h>

namespace Proto
{
namespace CONFERENCES_LIST
{
	static const std::string NAME = "conferences_list";

	struct Command
	{
		std::vector<Conference> conferences;
		
		Command();
		Command(const std::vector<Conference> &conferences);

		~Command();

		bool Parse(const std::string &message);
		std::string Serialize();
	};
}
}
