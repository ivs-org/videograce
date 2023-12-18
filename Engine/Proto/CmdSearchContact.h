/**
 * CmdGetContactList.h - Contains protocol command SEARCH_CONTACT
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <string>
#include <cstdint>

namespace Proto
{
namespace SEARCH_CONTACT
{
	static const std::string NAME = "search_contact";
	
	struct Command
	{
		std::string query;
		
		Command();
		Command(std::string_view query);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
