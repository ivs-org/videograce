/**
 * CmdUserUpdateResponse.h - Contains protocol command USER_UPDATE_RESPONSE
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#pragma once

#include <string>
#include <cstdint>

#include <Proto/CmdUserUpdateRequest.h>

namespace Proto
{
namespace USER_UPDATE_RESPONSE
{
	static const std::string NAME = "user_update_response";

	enum class Result
	{
		Undefined = 0,

		OK,
		
		DuplicateName,
		DuplicateLogin,
		RegistrationDenied
	};

	struct Command
	{
		Proto::USER_UPDATE_REQUEST::Action action;

		Result result;

		int64_t user_id;
		std::string message;
		
		Command();
		Command(Proto::USER_UPDATE_REQUEST::Action action, Result result, int64_t user_id = -1, const std::string &message = "");

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
