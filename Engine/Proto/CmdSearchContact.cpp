/**
 * CmdSearchContact.cpp - Contains protocol command SEARCH_CONTACT impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdSearchContact.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

namespace Proto
{
namespace SEARCH_CONTACT
{

static const std::string QUERY = "query";

Command::Command()
	: query()
{
}

Command::Command(std::string_view query_)
	: query(query_)
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

		query = pt.get<std::string>(NAME + "." + QUERY);

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
	return "{" + quot(NAME) + ":{" + quot(QUERY) + ":" + quot(Common::JSON::Screen(query)) + "}}";
}

}
}
