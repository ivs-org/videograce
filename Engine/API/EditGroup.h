/**
 * EditGroup.h - Contains API edit group json header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#pragma once

#include <string>
#include <cstdint>

namespace API
{
namespace EDIT_GROUP
{
	struct Command
	{
		int64_t id;
		int64_t parent_id;
		std::string name;
		std::string tag;
		std::string password;
		bool limited;
		std::string guid;
		int64_t owner_id;
		
		Command();
		Command(int64_t id, int64_t parent_id, const std::string &name, const std::string &tag, const std::string &password, bool limited, const std::string &guid, int64_t owner_id);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
