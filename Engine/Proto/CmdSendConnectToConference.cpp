/**
 * CmdSendConnectToConference.cpp - Contains protocol command SEND_CONNECT_TO_CONFERENCE impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdSendConnectToConference.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

namespace Proto
{
namespace SEND_CONNECT_TO_CONFERENCE
{

static const std::string TAG = "tag";
static const std::string CONNECTER_ID = "connecter_id";
static const std::string CONNECTER_CONNECTION_ID = "connecter_connection_id";
static const std::string FLAGS = "flags";

Command::Command()
	: tag(),
	connecter_id(0),
    connecter_connection_id(0),
    flags(0)
{
}

Command::Command(std::string_view tag_, int64_t connecter_id_, uint32_t connecter_connection_id_, uint32_t flags_)
	: tag(tag_), connecter_id(connecter_id_), connecter_connection_id(connecter_connection_id_), flags(flags_)
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
		connecter_id = pt.get<int64_t>(NAME + "." + CONNECTER_ID);
		connecter_connection_id = pt.get<uint32_t>(NAME + "." + CONNECTER_CONNECTION_ID);

        auto flags_opt = pt.get_optional<uint32_t>(NAME + "." + FLAGS);
        if (flags_opt) flags = flags_opt.get();
		
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
		quot(TAG) + ":" + quot(Common::JSON::Screen(tag)) + "," +
		quot(CONNECTER_ID) + ":" + std::to_string(connecter_id) + "," + 
		quot(CONNECTER_CONNECTION_ID) + ":" + std::to_string(connecter_connection_id) +
        (flags != 0 ? "," + quot(FLAGS) + ":" + std::to_string(flags) : "") + "}}";
}

}
}
