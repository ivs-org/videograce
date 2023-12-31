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

	enum class Type
	{
		CommandLoop,
		WSMedia,
		BlobChannel
	};

	struct Command
	{
		Type type;

		uint32_t client_version;
		std::string system;
		
		std::string login;
		std::string password;

		std::string access_token;
		
		Command();
		Command(Type type,
			uint32_t client_version,
			std::string_view system,
			std::string_view login,
			std::string_view password,
			std::string_view access_token);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
