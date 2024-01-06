/**
 * CmdConferenceUpdateRequest.cpp - Contains protocol command CONFERENCE_UPDATE_REQUEST impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018, 2023
 */

#include <Proto/CmdConferenceUpdateRequest.h>

#include <Common/Quoter.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace CONFERENCE_UPDATE_REQUEST
{

static const std::string ACTION = "action";
static const std::string CONFERENCE = "conference";

Command::Command()
	: action(Action::Undefined),
	conference()
{
}

Command::Command(Action action_, const Conference &conference_)
	: action(action_),
	conference(conference_)
{
}

Command::~Command()
{
}

bool Command::Parse(std::string_view message)
{
	try
	{
		auto j = nlohmann::json::parse(message);
		auto obj = j.get<nlohmann::json::object_t>().at(NAME);

		action = static_cast<Action>(obj.at(ACTION).get<int32_t>());

		return conference.Parse(obj.at(CONFERENCE));
	}
	catch (nlohmann::json::parse_error& ex)
	{
		spdlog::get("Error")->critical("proto::{0} :: error parse json (byte: {1}, what: {2})", NAME, ex.byte, ex.what());
	}
	return false;
}

std::string Command::Serialize()
{
	return "{" + quot(NAME) + ":{" + quot(ACTION) + ":" + std::to_string(static_cast<int32_t>(action)) + "," +
		quot(CONFERENCE) + ":" + conference.Serialize() +
		"}}";
}

}
}
