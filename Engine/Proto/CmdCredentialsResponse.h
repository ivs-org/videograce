/**
 * CmdCredentialsResponse.h - Contains protocol command CREDENTIALS_RESPONSE
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#pragma once

#include <string>
#include <cstdint>

namespace Proto
{
namespace CREDENTIALS_RESPONSE
{
	enum class Result
	{
		Undefined = 0,

		NotFound,
		OK
	};

	static const std::string NAME = "credentials_response";

	struct Command
	{
		Result result;

		std::string login;
		std::string password;
		
		Command();
		Command(Result result, const std::string &login, const std::string &password);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
