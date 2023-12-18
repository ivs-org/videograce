/**
 * CmdContactList.h - Contains protocol command CONTACT_LIST
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <string>
#include <cstdint>
#include <vector>

#include <Proto/Member.h>

namespace Proto
{
namespace CONTACT_LIST
{
	static const std::string NAME = "contact_list";

    enum class SortType
    {
        Undefined = 0,
        Name,
        Number
    };

	struct Command
	{
        SortType sort_type;
        bool show_numbers;

		std::vector<Member> members;
		
		Command();
		Command(const std::vector<Member> &members);
        Command(SortType sort_type, bool show_numbers, const std::vector<Member> &members);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
