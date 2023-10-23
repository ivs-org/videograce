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
		Command(const std::string &guid);

		~Command();

		bool Parse(const std::string &message);
		std::string Serialize();
	};
}
}
