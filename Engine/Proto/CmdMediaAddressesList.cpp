/**
 * CmdMediaAddressesList.cpp - Contains protocol command RTP_ADDRESSES_LIST impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdMediaAddressesList.h>

#include <Common/Common.h>
#include <Common/JSONSymbolsScreener.h>
#include <Common/Quoter.h>

namespace Proto
{
namespace MEDIA_ADDRESSES_LIST
{

Command::Command()
	: addresses()
{
}

Command::Command(const std::vector<std::string> &addresses_)
	: addresses(addresses_)
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
		
		auto &addresses_ = pt.get_child(NAME);
		for (auto &a : addresses_)
		{
			addresses.emplace_back(a.second.data());
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

	for (auto &a : addresses)
	{
		out += "\"" + a + "\",";
	}
	if (!addresses.empty())
	{
		out.pop_back(); // drop last ","
	}
	return out + "]}";
}

}
}
