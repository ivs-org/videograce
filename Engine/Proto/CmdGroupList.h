/**
 * CmdGroupList.h - Contains protocol command GROUP_LIST
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#pragma once

#include <string>
#include <cstdint>
#include <vector>

#include <Proto/Group.h>

namespace Proto
{
namespace GROUP_LIST
{
	static const std::string NAME = "group_list";

	struct Command
	{
		std::vector<Group> groups;
		
		Command();
		Command(const std::vector<Group> &groups);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
