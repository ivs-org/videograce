/**
 * CmdMediaAddressesList.h - Contains protocol command MEDIA_ADDRESSES_LIST
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <string>
#include <cstdint>
#include <vector>

namespace Proto
{
namespace MEDIA_ADDRESSES_LIST
{
	static const std::string NAME = "media_addresses_list";

	struct Command
	{
		std::vector<std::string> addresses;

		Command();
		Command(const std::vector<std::string> &addresses);
		
		~Command();

		bool Parse(const std::string &message);
		std::string Serialize();
	};
}
}
