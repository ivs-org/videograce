/**
 * CmdResolutionChange.cpp - Contains protocol command RESOLUTION_CHANGE impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdResolutionChange.h>

#include <Common/Common.h>
#include <Common/Quoter.h>

namespace Proto
{
namespace RESOLUTION_CHANGE
{

static const std::string ID = "id";
static const std::string RESOLUTION = "resolution";

Command::Command()
	: id(0),
	resolution(0)
{
}

Command::Command(uint32_t id_, uint32_t resolution_)
	: id(id_), resolution(resolution_)
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

		id = pt.get<uint32_t>(NAME + "." + ID);
		resolution = pt.get<uint32_t>(NAME + "." + RESOLUTION);

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
	return "{" + quot(NAME) + ":{" + quot(ID) + ":" + std::to_string(id) + "," + quot(RESOLUTION) + ":" + std::to_string(resolution) + "}}";
}

}
}
