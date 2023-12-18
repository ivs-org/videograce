/**
 * CmdScheduleConnect.cpp - Contains protocol command SCHEDULE_CONNECT impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdScheduleConnect.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

namespace Proto
{
namespace SCHEDULE_CONNECT
{

static const std::string TAG = "tag";
static const std::string NAME_ = "name";
static const std::string TIME_LIMIT = "time_limit";

Command::Command()
	: tag(),
	name(),
	time_limit(0)
{
}

Command::Command(const std::string &tag_, const std::string &name_, uint64_t time_limit_)
	: tag(tag_), name(name_), time_limit(time_limit_)
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
		name = pt.get<std::string>(NAME + "." + NAME_);
		time_limit = pt.get<uint64_t>(NAME + "." + TIME_LIMIT);

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
	return "{" + quot(NAME) + ":{" + quot(TAG) + ":" + quot(Common::JSON::Screen(tag)) + "," +
		quot(NAME_) + ":" + quot(Common::JSON::Screen(name)) + "," +
		quot(TIME_LIMIT) + ":" + std::to_string(time_limit) + "}}";
}

}
}
