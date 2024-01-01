/**
 * CmdConnectResponse.h - Contains protocol command CONNECT_RESPONSE
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018 - 2021
 */

#pragma once

#include <string>
#include <cstdint>

namespace Proto
{
namespace CONNECT_RESPONSE
{
	static const std::string NAME = "connect_response";

	enum class Result
	{
		Undefined = 0,

		OK,
		InvalidCredentials,
		UpdateRequired,
		Redirect,
		ServerFull,
		InternalServerError
	};

	struct Command
	{
		Result result;

		uint32_t server_version;
		int64_t id;
		int64_t connection_id;	
		std::string access_token;

		std::string redirect_url;
		std::string name;
		std::string secure_key;
		std::string server_name;
		
		uint32_t options;
		uint32_t grants;
		uint32_t max_output_bitrate;

		Command();
		Command(Result result,

			uint32_t server_version,
			int64_t id,
			int64_t connection_id,
			std::string_view access_token,

			std::string_view name,
			std::string_view redirect_url,
			std::string_view secure_key,
			std::string_view server_name,
			
			uint32_t options,
			uint32_t grants,
			uint32_t max_output_bitrate);
		Command(Result result);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
