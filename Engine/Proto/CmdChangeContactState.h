/**
 * CmdChangeContactState.h - Contains protocol command CHANGE_CONTACT_STATE
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#pragma once

#include <string>
#include <cstdint>

#include <Proto/Member.h>

namespace Proto
{
namespace CHANGE_CONTACT_STATE
{
	static const std::string NAME = "change_contact_state";

	struct Command
	{
		int64_t id;
		MemberState state;
		
		Command();
		Command(int64_t id, MemberState state);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
