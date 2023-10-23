/**
 * CmdWantSpeak.h - Contains protocol command WANT_SPEAK
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <string>
#include <cstdint>

namespace Proto
{
namespace WANT_SPEAK
{
	static const std::string NAME = "want_speak";
	
	struct Command
	{
		int64_t user_id;
		std::string user_name;
		bool is_speak;

		Command();
		Command(int64_t user_id, const std::string &user_name, bool is_speak);

		~Command();

		bool Parse(const std::string &message);
		std::string Serialize();
	};
}
}
