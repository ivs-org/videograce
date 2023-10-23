/**
 * CmdPing.h - Contains protocol command PING
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2017
 */

#pragma once

#include <string>

namespace Proto
{
namespace PING
{
	static const std::string NAME = "ping";

	struct Command
	{
		Command();
		
		~Command();

		bool Parse(const std::string &message);
		std::string Serialize();
	};
}
}
