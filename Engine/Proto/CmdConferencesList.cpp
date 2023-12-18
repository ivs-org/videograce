/**
 * CmdConferencesList.cpp - Contains protocol command CONFERENCES_LIST impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdConferencesList.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

namespace Proto
{
namespace CONFERENCES_LIST
{

Command::Command()
	: conferences()
{
}

Command::Command(const std::vector<Conference> &conferences_)
	: conferences(conferences_)
{
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

		for (auto &c : pt.get_child(NAME))
		{
			Conference conference;
			if (conference.Parse(c.second))
			{
				conferences.emplace_back(conference);
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

	for (auto &c : conferences)
	{
		out += c.Serialize() + ",";
	}
	if (!conferences.empty())
	{
		out.pop_back(); // drop last ","
	}

	return out + "]}";
}

}
}
