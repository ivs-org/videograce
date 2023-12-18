/**
 * CmdConnectRequest.h - Contains protocol command CONNECT_REQUEST
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <string>
#include <cstdint>

namespace Proto
{
namespace CONNECT_REQUEST
{
	static const std::string NAME = "connect_request";

	struct Command
	{
		uint32_t client_version;
		std::string system;
		std::string login;
		std::string password;
		
		Command();
		Command(uint32_t client_version, const std::string &system, const std::string &login, const std::string &password);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
