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
		uint32_t connection_id;
		std::string redirect_url;
		std::string name;
		std::string secure_key;
		std::string server_name;
		uint32_t options;
		uint32_t grants;
		uint32_t max_output_bitrate;
		uint16_t reduced_frame_rate;

		Command();
		Command(Result result, uint32_t server_version, int64_t id, uint32_t connection_id, const std::string &name, const std::string &redirect_url, const std::string &secure_key, const std::string &server_name, uint32_t options, uint32_t grants, uint32_t max_output_bitrate, uint16_t reduced_frame_rate);
		Command(Result result);

		~Command();

		bool Parse(const std::string &message);
		std::string Serialize();
	};
}
}
