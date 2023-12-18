/**
 * Result.h - Contains API result json header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#pragma once

#include <string>
#include <cstdint>

namespace API
{
namespace RESULT
{
	struct Command
	{
		uint32_t code;
		std::string message;
		std::string opt;
		
		Command();
		Command(uint32_t code, const std::string &message, const std::string &opt = "");

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
