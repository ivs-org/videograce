/**
 * CmdChangeMemberState.h - Contains protocol command CHANGE_MEMBER_STATE
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2017
 */

#pragma once

#include <string>
#include <cstdint>
#include <vector>

#include <Proto/Member.h>

namespace Proto
{
namespace CHANGE_MEMBER_STATE
{
	static const std::string NAME = "change_member_state";

	struct Command
	{
		std::vector<Member> members;

		Command();
		Command(const std::vector<Member> &members);
		Command(const Member &member);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
