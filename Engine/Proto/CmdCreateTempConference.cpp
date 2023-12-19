/**
 * CmdCreateTempConference.cpp - Contains protocol command CREATE_TEMP_CONFERENCE impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdCreateTempConference.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

namespace Proto
{
namespace CREATE_TEMP_CONFERENCE
{

static const std::string TAG = "tag";

Command::Command()
	: tag()
{
}

Command::Command(std::string_view tag_)
	: tag(tag_)
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

		tag = pt.get<std::string>(NAME + "." + TAG);
				
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
	return "{" + quot(NAME) + ":{" + quot(TAG) + ":" + quot(Common::JSON::Screen(tag)) + "}}";
}

}
}
