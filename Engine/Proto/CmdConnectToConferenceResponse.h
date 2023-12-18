/**
 * CmdConnectToConferenceResponse.h - Contains protocol command CONNECT_TO_CONFERENCE_RESPONSE
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <string>
#include <cstdint>

namespace Proto
{
namespace CONNECT_TO_CONFERENCE_RESPONSE
{
	static const std::string NAME = "connect_to_conference_response";

	enum class Result
	{
		Undefined = 0,

		OK,
		NotExists,
		NotAllowed,

		LicenseFull
	};

	struct Command
	{
		Result result;
		int64_t id;
		uint32_t grants;
		int64_t founder_id;
		std::string tag;
		std::string name;
		bool temp;
		
		Command();
		Command(Result result, int64_t id, uint32_t grants, int64_t founder_id, const std::string &tag, const std::string &name, bool temp);
		Command(Result result);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
