/**
 * CmdConferenceUpdateRequest.h - Contains protocol command CONFERENCE_UPDATE_REQUEST
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <string>
#include <cstdint>

#include <Proto/Conference.h>

namespace Proto
{
namespace CONFERENCE_UPDATE_REQUEST
{
	static const std::string NAME = "conference_update_request";

	enum class Action
	{
		Undefined = 0,

		Create,
		Edit,
		Delete,

        AddMe,
        DeleteMe
	};

	struct Command
	{
		Action action;
		Conference conference;
				
		Command();
		Command(Action action, const Conference &conference);

		~Command();

		bool Parse(const std::string &message);
		std::string Serialize();
	};
}
}
