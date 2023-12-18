/**
 * CmdUpdateGrants.cpp - Contains protocol command UPDATE_GRANTS impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdUpdateGrants.h>

#include <Common/Common.h>
#include <Common/Quoter.h> 

namespace Proto
{
namespace UPDATE_GRANTS
{

static const std::string GRANTS = "grants";

Command::Command()
	: grants(0)
{
}

Command::Command(uint32_t grants_)
	: grants(grants_)
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

		grants = pt.get<uint32_t>(NAME + "." + GRANTS);

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
	return "{" + quot(NAME) + ":{" + quot(GRANTS) + ":" + std::to_string(grants) + "}}";
}

}
}
