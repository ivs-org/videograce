/**
 * CmdConferenceUpdateResponse.h - Contains protocol command CONFERENCE_UPDATE_RESPONSE
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <string>
#include <cstdint>

namespace Proto
{
namespace CONFERENCE_UPDATE_RESPONSE
{
	static const std::string NAME = "conference_update_response";

	enum class Result
	{
		Undefined = 0,

		OK,
		NotFound,
		DuplicateTag,
		NotAllowed
	};

	struct Command
	{
        int64_t id;
		Result result;
				
		Command();
        Command(Result result);
		Command(int64_t id, Result result);

		~Command();

		bool Parse(const std::string &message);
		std::string Serialize();
	};
}
}
