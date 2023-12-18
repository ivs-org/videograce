/**
 * CmdChangeMemberState.cpp - Contains protocol command CHANGE_MEMBER_STATE impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2017
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdChangeMemberState.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

namespace Proto
{
namespace CHANGE_MEMBER_STATE
{

Command::Command()
	: members()
{
}

Command::Command(const std::vector<Member> &members_)
	: members(members_)
{
}

Command::Command(const Member &member)
	: members()
{
	members.emplace_back(member);
}

Command::~Command()
{
}

bool Command::Parse(std::string_view message)
{
	using boost::property_tree::ptree;

	std::stringstream ss;
	ss << message;

	try
	{
		ptree pt;
		read_json(ss, pt);

		auto &users = pt.get_child(NAME);
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
	std::string out = "{" + quot(NAME) + ":[";

	for (auto &m : members)
	{
		out += m.Serialize() + ",";
	}
	if (!members.empty())
	{
		out.pop_back(); // drop last ","
	}
	return out + "]}";
}

}
}
