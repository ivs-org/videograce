/**
 * CmdSendConnectToConference.cpp - Contains protocol command SEND_CONNECT_TO_CONFERENCE impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018, 2023
 */

#include <Proto/CmdSendConnectToConference.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

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
	try
	{
		spdlog::get("System")->trace("proto::{0} :: perform parsing", NAME);

		auto j = nlohmann::json::parse(message);
		auto obj = j.get<nlohmann::json::object_t>().at(NAME);

		tag = obj.at(TAG).get<std::string>();
		connecter_id = obj.at(CONNECTER_ID).get<int64_t>();
		connecter_connection_id = obj.at(CONNECTER_CONNECTION_ID).get<uint32_t>();
		
		if (obj.count(FLAGS) != 0) flags = obj.at(FLAGS).get<uint32_t>();

		return true;
	}
	catch (nlohmann::json::parse_error& ex)
	{
		spdlog::get("Error")->critical("proto::{0} :: error parse json (byte: {1}, what: {2})", NAME, ex.byte, ex.what());
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
