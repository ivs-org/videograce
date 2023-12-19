/**
 * CreateGroup.h - Contains API create group json header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#pragma once

#include <string>
#include <cstdint>

namespace API
{
namespace CREATE_GROUP
{
	struct Command
	{
		int64_t parent_id;
		std::string name;
		std::string tag;
		std::string password;
		bool limited;
		std::string guid;
		int64_t owner_id;
		
		Command();
		Command(int64_t parent_id, std::string_view name, std::string_view tag, std::string_view password, bool limited, std::string_view guid, int64_t owner_id);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
