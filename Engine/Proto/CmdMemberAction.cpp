/**
 * CmdMemberAction.cpp - Contains protocol command MEMBER_ACTION impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016, 2023
 */

#include <Proto/CmdMemberAction.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace MEMBER_ACTION
{

static const std::string IDS = "id";
static const std::string ACTION = "action";
static const std::string RESULT = "result";
static const std::string ACTOR_ID = "actor_id";
static const std::string ACTOR_NAME = "actor_name";
static const std::string GRANTS = "grants";

Command::Command()
	: ids(), action(Action::Undefined), result(Result::Undefined), actor_id(0), actor_name(), grants(0)
{
}

Command::Command(const std::vector<int64_t> &ids_, Action action_, uint32_t grants_)
	: ids(ids_), action(action_), result(Result::Undefined), actor_id(0), actor_name(), grants(grants_)
{
}

Command::Command(Action action_, Result result_, int64_t actor_id_, std::string_view actor_name_)
	: ids(), action(action_), result(result_), actor_id(actor_id_), actor_name(actor_name_), grants(0)
{
}

Command::Command(Result result_)
	: ids(), action(Action::Undefined), result(result_), actor_id(0), actor_name(), grants(0)
{
}

Command::Command(int64_t actor_id_, Result result_)
	: ids(std::vector<int64_t>{ actor_id_ }), action(Action::Undefined), result(result_), actor_id(0), actor_name(), grants(0)
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
		
		auto is = obj.get<nlohmann::json::object_t>().at(IDS);
		for (auto &i : is)
		{
			ids.emplace_back(i.get<uint32_t>());
		}
		
		if (obj.count(ACTION) != 0) action = static_cast<Action>(obj.at(ACTION).get<uint32_t>());
		if (obj.count(RESULT) != 0) result = static_cast<Result>(obj.at(RESULT).get<uint32_t>());
		if (obj.count(ACTOR_ID) != 0) actor_id = obj.at(ACTOR_ID).get<int64_t>();
		if (obj.count(ACTOR_NAME) != 0) actor_name = obj.at(ACTOR_NAME).get<std::string>();
		if (obj.count(GRANTS) != 0) grants = obj.at(GRANTS).get<uint32_t>();

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
	std::string out = "{" + quot(NAME) + ":{" + quot(IDS) + ":[";
	
	for (auto &i : ids)
	{
		out += std::to_string(i) + ",";
	}
	if (!ids.empty())
	{
		out.pop_back(); // drop last ","
	}
	out += "],";
	
    out += (action != Action::Undefined ? quot(ACTION) + ":" + std::to_string(static_cast<int32_t>(action)) + "," : "") +
        (result != Result::Undefined ? quot(RESULT) + ":" + std::to_string(static_cast<int32_t>(result)) + "," : "") +
        (actor_id != 0 ? quot(ACTOR_ID) + ":" + std::to_string(actor_id) + "," : "") +
        (!actor_name.empty() ? quot(ACTOR_NAME) + ":" + quot(Common::JSON::Screen(actor_name)) + "," : "") +
        (grants != 0 ? quot(GRANTS) + ":" + std::to_string(grants) + "," : "");

	out.pop_back(); // drop last ","
	return out + "}}";
}

}
}
