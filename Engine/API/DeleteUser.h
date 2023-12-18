/**
 * DeleteUser.h - Contains API delete user json header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#pragma once

#include <string>
#include <cstdint>

namespace API
{
namespace DELETE_USER
{
	struct Command
	{
		std::string login;
		
		Command();
		Command(const std::string &login);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
