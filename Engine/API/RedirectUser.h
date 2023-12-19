/**
 * RedirectUser.h - Contains API redirect user json header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#pragma once

#include <string>
#include <cstdint>

namespace API
{
namespace REDIRECT_USER
{
	struct Command
	{
		std::string login;
		std::string url;
		
		Command();
		Command(std::string_view login, std::string_view url);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
