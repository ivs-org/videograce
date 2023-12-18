/**
 * CmdSetMaxBitrate.cpp - Contains protocol command SET_MAX_BITRATE impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdSetMaxBitrate.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

namespace Proto
{
namespace SET_MAX_BITRATE
{

static const std::string BITRATE = "bitrate";
static const std::string STATE = "state";
static const std::string NAME_ = "name";
static const std::string NUMBER = "number";

Command::Command()
	: bitrate(0)
{
}

Command::Command(uint32_t bitrate_)
	: bitrate(bitrate_)
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

		bitrate = pt.get<uint32_t>(NAME + "." + BITRATE);
		
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
	return "{" + quot(NAME) + ":{" + quot(BITRATE) + ":" + std::to_string(bitrate) + "}}";
}

}
}
