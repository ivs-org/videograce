/**
 * CmdChangeServer.h - Contains protocol command CHANGE_SERVER
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#pragma once

#include <string>
#include <cstdint>

namespace Proto
{
namespace CHANGE_SERVER
{
	static const std::string NAME = "change_server";

	struct Command
	{
		std::string url;

		Command();
		Command(const std::string &url);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
