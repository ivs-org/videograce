/**
 * CmdDisconnectFromConference.h - Contains protocol command DISCONNECT_FROM_CONFERENCE
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#pragma once

#include <string>

namespace Proto
{
namespace DISCONNECT_FROM_CONFERENCE
{
	static const std::string NAME = "disconnect_from_conference";

	struct Command
	{
		Command();
		~Command();

		bool Parse(const std::string &message);
		std::string Serialize();
	};
}
}
