/**
 * CmdLoadMessages.cpp - Contains protocol command LOAD_MESSAGES impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2019
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdLoadMessages.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

namespace Proto
{
namespace LOAD_MESSAGES
{

static const std::string FROM = "from_dt";

Command::Command()
	: from()
{
}

Command::Command(uint64_t from_)
	: from(from_)
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

		auto from_opt = pt.get_optional<uint64_t>(NAME + "." + FROM);
		if (from_opt) from = from_opt.get();

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
	return "{" + quot(NAME) + ":{" + (from != 0 ? quot(FROM) + ":" + std::to_string(from) : "") + "}}";
}

}
}
