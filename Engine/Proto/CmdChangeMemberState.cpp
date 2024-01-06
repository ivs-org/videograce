/**
 * CmdChangeMemberState.cpp - Contains protocol command CHANGE_MEMBER_STATE impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2017, 2023
 */

#include <Proto/CmdChangeMemberState.h>

#include <Common/Quoter.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace CHANGE_MEMBER_STATE
{

Command::Command()
	: members()
{
}

Command::Command(const std::vector<Member> &members_)
	: members(members_)
{
}

Command::Command(const Member &member)
	: members()
{
	members.emplace_back(member);
}

Command::~Command()
{
}

bool Command::Parse(std::string_view message)
{
	try
	{
		auto j = nlohmann::json::parse(message);
		auto users = j.get<nlohmann::json::object_t>().at(NAME);
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
	std::string out = "{" + quot(NAME) + ":[";

	for (auto &m : members)
	{
		out += m.Serialize() + ",";
	}
	if (!members.empty())
	{
		out.pop_back(); // drop last ","
	}
	return out + "]}";
}

}
}
