/**
 * CmdChangeContactState.cpp - Contains protocol command CHANGE_CONTACT_STATE impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#include <Proto/CmdChangeContactState.h>

#include <Common/Quoter.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace CHANGE_CONTACT_STATE
{

static const std::string ID = "id";
static const std::string STATE = "state";

Command::Command()
	: id(0),
	state(MemberState::Undefined)
{
}

Command::Command(int64_t id_, MemberState state_)
	: id(id_), state(state_)
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

		id = obj.at(ID).get<int64_t>();
		state = static_cast<MemberState>(obj.at(STATE).get<uint32_t>());

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
	return "{" + quot(NAME) + ":{" + quot(ID) + ":" + std::to_string(id) + "," +
		quot(STATE) + ":" + std::to_string(static_cast<int32_t>(state)) + "}}";
}

}
}
