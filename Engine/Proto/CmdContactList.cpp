/**
 * CmdContactList.cpp - Contains protocol command CONTACT_LIST impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdContactList.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

namespace Proto
{
namespace CONTACT_LIST
{

static const std::string SORT_TYPE = "sort_type";
static const std::string SHOW_NUMBERS = "show_numbers";
static const std::string MEMBERS = "members";

Command::Command()
	: sort_type(SortType::Undefined), show_numbers(false), members()
{
}

Command::Command(const std::vector<Member> &members_)
	: sort_type(SortType::Undefined), show_numbers(false), members(members_)
{
}

Command::Command(SortType sort_type_, bool show_numbers_, const std::vector<Member> &members_)
    : sort_type(sort_type_), show_numbers(show_numbers_), members(members_)
{
}

Command::~Command()
{
}

bool Command::Parse(const std::string &message)
{
	using boost::property_tree::ptree;

	std::stringstream ss;
	ss << message;

	try
	{
		ptree pt;
		read_json(ss, pt);

        auto sort_type_opt = pt.get_optional<int32_t>(NAME + "." + SORT_TYPE);
        if (sort_type_opt) sort_type = static_cast<SortType>(sort_type_opt.get());

        auto show_numbers_opt = pt.get_optional<int32_t>(NAME + "." + SHOW_NUMBERS);
        if (show_numbers_opt) show_numbers = show_numbers_opt.get() != 0;

		auto &users = pt.get_child(NAME + "." + MEMBERS);
		for (auto &u : users)
		{
			Member member;
			if (member.Parse(u.second))
			{
				members.emplace_back(member);
			}
		}
				
		return true;
	}
	catch (std::exception const& e)
	{
		DBGTRACE("Error parsing %s, %s\n", NAME.c_str(), e.what());
	}
	return false;
}

std::string Command::Serialize()
{
	std::string out = "{" + quot(NAME) + ":{";

    out += (sort_type != SortType::Undefined ? quot(SORT_TYPE) + ":" + std::to_string(static_cast<int32_t>(sort_type)) + "," : "") +
        (show_numbers ? quot(SHOW_NUMBERS) + ":1," : "");

    out += quot(MEMBERS) + ":[";
	for (auto &m : members)
	{
		out += m.Serialize() + ",";
	}
	if (!members.empty())
	{
		out.pop_back(); // drop last ","
	}
	return out + "]}}";
}

}
}
