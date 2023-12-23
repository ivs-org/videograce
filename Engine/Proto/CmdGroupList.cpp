/**
 * CmdGroupList.cpp - Contains protocol command GROUP_LIST impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021, 2023
 */

#include <Proto/CmdGroupList.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace GROUP_LIST
{

Command::Command()
	: groups()
{
}

Command::Command(const std::vector<Group> &groups_)
	: groups(groups_)
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
		auto groups_ = j.get<nlohmann::json::object_t>().at(NAME);
		for (auto &g : groups_)
		{
			Group group;
			if (group.Parse(g))
			{
				groups.emplace_back(group);
			}
		}
				
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
	std::string out = "{" + quot(NAME) + ":[";

	for (auto &g : groups)
	{
		out += g.Serialize() + ",";
	}
	if (!groups.empty())
	{
		out.pop_back(); // drop last ","
	}
	return out + "]}";
}

}
}
