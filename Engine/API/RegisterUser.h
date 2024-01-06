/**
 * RegisterUser.h - Contains API register user json header
 *
 * Author: Anton Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2024
 */

#pragma once

#include <string>
#include <cstdint>

namespace API
{
namespace REGISTER_USER
{
	struct Command
	{
		std::string name;
		std::string login;
		std::string password;
		std::string captcha;
		
		Command();
		Command(std::string_view name, std::string_view login, std::string_view password, std::string_view captcha);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
