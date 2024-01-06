/**
 * CmdContactList.cpp - Contains protocol command CONTACT_LIST impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018, 2023
 */

#include <Proto/CmdContactList.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace CONTACT_LIST
{

static const std::string SORT_TYPE = "sort_type";
static const std::string SHOW_NUMBERS = "show_numbers";
static const std::string MEMBERS = "members";

Command::Command()
	: sort_type(SortType::Undefined), show_numbers(false), members()
{
}

Command::Command(const std::vector<Member> &members_)
	: sort_type(SortType::Undefined), show_numbers(false), members(members_)
{
}

Command::Command(SortType sort_type_, bool show_numbers_, const std::vector<Member> &members_)
    : sort_type(sort_type_), show_numbers(show_numbers_), members(members_)
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

		if (obj.count(SORT_TYPE) != 0) sort_type = static_cast<SortType>(obj.at(SORT_TYPE).get<int32_t>());
		if (obj.count(SHOW_NUMBERS) != 0) show_numbers = obj.at(SHOW_NUMBERS).get<int32_t>();

		auto users = obj.at(MEMBERS);
		for (auto &u : users)
		{
			Member member;
			if (member.Parse(u))
			{
				members.emplace_back(member);
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
	std::string out = "{" + quot(NAME) + ":{";

    out += (sort_type != SortType::Undefined ? quot(SORT_TYPE) + ":" + std::to_string(static_cast<int32_t>(sort_type)) + "," : "") +
        (show_numbers ? quot(SHOW_NUMBERS) + ":1," : "");

    out += quot(MEMBERS) + ":[";
	for (auto &m : members)
	{
		out += m.Serialize() + ",";
	}
	if (!members.empty())
	{
		out.pop_back(); // drop last ","
	}
	return out + "]}}";
}

}
}
