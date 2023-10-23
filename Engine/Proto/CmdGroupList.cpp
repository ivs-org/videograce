/**
 * CmdGroupList.cpp - Contains protocol command GROUP_LIST impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdGroupList.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

namespace Proto
{
namespace GROUP_LIST
{

Command::Command()
	: groups()
{
}

Command::Command(const std::vector<Group> &groups_)
	: groups(groups_)
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

		auto &groups_ = pt.get_child(NAME);
		for (auto &g : groups_)
		{
			Group group;
			if (group.Parse(g.second))
			{
				groups.emplace_back(group);
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

	for (auto &g : groups)
	{
		out += g.Serialize() + ",";
	}
	if (!groups.empty())
	{
		out.pop_back(); // drop last ","
	}
	return out + "]}";
}

}
}
