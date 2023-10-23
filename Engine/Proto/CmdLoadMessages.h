/**
 * CmdLoadMessages.h - Contains protocol command LOAD_MESSAGES
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2019
 */

#pragma once

#include <string>
#include <cstdint>

namespace Proto
{
namespace LOAD_MESSAGES
{
	static const std::string NAME = "load_messages";

	struct Command
	{
		uint64_t from;

		Command();
		Command(uint64_t from);

		~Command();

		bool Parse(const std::string &message);
		std::string Serialize();
	};
}
}
