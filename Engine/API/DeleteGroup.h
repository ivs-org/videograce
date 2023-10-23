/**
 * DeleteGroup.h - Contains API delete group json header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#pragma once

#include <string>
#include <cstdint>

namespace API
{
namespace DELETE_GROUP
{
	struct Command
	{
		int64_t id;
		
		Command();
		Command(int64_t id);

		~Command();

		bool Parse(const std::string &message);
		std::string Serialize();
	};
}
}
