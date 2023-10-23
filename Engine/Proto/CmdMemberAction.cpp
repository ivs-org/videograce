/**
 * CmdMemberAction.cpp - Contains protocol command MEMBER_ACTION impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdMemberAction.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

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

Command::Command(Action action_, Result result_, int64_t actor_id_, const std::string &actor_name_)
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

bool Command::Parse(const std::string &message)
{
	using boost::property_tree::ptree;

	std::stringstream ss;
	ss << message;

	try
	{
		ptree pt;
		read_json(ss, pt);
		
		auto &is = pt.get_child(NAME + "." + IDS);
		for (auto &i : is)
		{
			ids.emplace_back(i.second.get_value<uint32_t>());
		}
		
		auto action_opt = pt.get_optional<uint32_t>(NAME + "." + ACTION);
		if (action_opt) action = static_cast<Action>(action_opt.get());

		auto result_opt = pt.get_optional<uint32_t>(NAME + "." + RESULT);
		if (result_opt) result = static_cast<Result>(result_opt.get());

		auto actor_id_opt = pt.get_optional<int64_t>(NAME + "." + ACTOR_ID);
		if (actor_id_opt) actor_id = actor_id_opt.get();

		auto actor_name_opt = pt.get_optional<std::string>(NAME + "." + ACTOR_NAME);
		if (actor_name_opt) actor_name = actor_name_opt.get();

        auto grants_opt = pt.get_optional<uint32_t>(NAME + "." + GRANTS);
        if (grants_opt) grants = grants_opt.get();

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
