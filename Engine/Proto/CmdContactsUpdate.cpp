/**
 * CmdContactsUpdate.cpp - Contains protocol command CONTACTS_UPDATE impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022, 2023
 */

#include <Proto/CmdContactsUpdate.h>

#include <Common/JSONSymbolsScreener.h>
#include <Common/Quoter.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace CONTACTS_UPDATE
{

static const std::string ACTION = "action";
static const std::string CLIENT_ID = "client_id";

Command::Command()
	: action(Action::Undefined), client_id(-1)
{
}

Command::Command(Action action_,
    int64_t client_id_)
	: action(action_),
    client_id(client_id_)
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

		action = static_cast<Action>(obj.at(ACTION).get<uint32_t>());
		client_id = obj.at(CLIENT_ID).get<uint32_t>();

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
		quot(ACTION) + ":" + std::to_string(static_cast<uint32_t>(action)) + "," +
        quot(CLIENT_ID) + ":" + std::to_string(client_id) + "}}";
}

}
}
