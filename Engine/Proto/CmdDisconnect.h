/**
 * CmdDisconnect.h - Contains protocol command DISCONNECT
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#pragma once

#include <string>

namespace Proto
{
namespace DISCONNECT
{
	static const std::string NAME = "disconnect";

	struct Command
	{
		Command();
		~Command();

		bool Parse(const std::string &message);
		std::string Serialize();
	};
}
}
