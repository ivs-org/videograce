/**
 * CmdCredentialsRequest.cpp - Contains protocol command CREDENTIALS_REQUEST impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdCredentialsRequest.h>

#include <Common/Common.h>
#include <Common/JSONSymbolsScreener.h>
#include <Common/Quoter.h>

namespace Proto
{
namespace CREDENTIALS_REQUEST
{

static const std::string GUID = "guid";

Command::Command()
	: guid()
{
}

Command::Command(std::string_view guid_)
	: guid(guid_)
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

		guid = pt.get<std::string>(NAME + "." + GUID);

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
	return "{" + quot(NAME) + ":{" + 
		quot(GUID) + ":" + quot(Common::JSON::Screen(guid)) + "}}";
}

}
}
