/**
 * CmdCredentialsRequest.h - Contains protocol command CREDENTIALS_REQUEST
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#pragma once

#include <string>
#include <cstdint>

namespace Proto
{
namespace CREDENTIALS_REQUEST
{
	static const std::string NAME = "credentials_request";

	struct Command
	{
		std::string guid;

		Command();
		Command(std::string_view guid);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
