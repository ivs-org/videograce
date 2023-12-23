/**
 * EditUser.cpp - Contains API edit user json impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#include <API/EditUser.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace API
{
namespace EDIT_USER
{

static const std::string ID = "id";
static const std::string NAME = "name";
static const std::string LOGIN = "login";
static const std::string PASSWORD = "password";
static const std::string NUMBER = "number";
static const std::string GROUPS = "groups";
static const std::string TIME_LIMIT = "time_limit";
static const std::string ALLOW_CREATE_CONFERENCE = "allow_create_conference";
static const std::string USE_ONLY_TCP = "use_only_tcp";
static const std::string GUID = "guid";

Command::Command()
	: id(-1), name(), login(), password(), number(-1), groups(), time_limit(), allow_create_conference(false), use_only_tcp(false), guid()
{
}

Command::Command(int64_t id_, std::string_view name_, std::string_view login_, std::string_view password_, uint32_t number_, const std::vector<Proto::Group> &groups_, uint64_t time_limit_, bool allow_create_conference_, bool use_only_tcp_, std::string_view guid_)
	: id(id_), name(name_), login(login_), password(password_), number(number_), groups(groups_), time_limit(time_limit_), allow_create_conference(allow_create_conference_), use_only_tcp(use_only_tcp_), guid(guid_)
{
}

Command::~Command()
{
}

bool Command::Parse(std::string_view message_)
{
	try
	{
		spdlog::get("System")->trace("api::edit_user :: perform parsing");

		auto j = nlohmann::json::parse(message_);
		auto obj = j.get<nlohmann::json::object_t>();

		if (obj.count(ID) != 0) id = obj.at(ID).get<int64_t>();
		if (obj.count(NAME) != 0) name = obj.at(NAME).get<std::string>();
		if (obj.count(LOGIN) != 0) login = obj.at(LOGIN).get<std::string>();
		if (obj.count(PASSWORD) != 0) password = obj.at(PASSWORD).get<std::string>();
		if (obj.count(NUMBER) != 0) number = obj.at(NUMBER).get<uint32_t>();
		
		if (obj.count(GROUPS) != 0)
		{
			auto groups_ = j.get<nlohmann::json::object_t>().at(GROUPS);
			for (auto &g : groups_)
			{
				Proto::Group group;
				if (group.Parse(g))
				{
					groups.emplace_back(group);
				}
			}
		}

		if (obj.count(TIME_LIMIT) != 0) time_limit = obj.at(TIME_LIMIT).get<uint64_t>();
		if (obj.count(ALLOW_CREATE_CONFERENCE) != 0) allow_create_conference = obj.at(ALLOW_CREATE_CONFERENCE).get<uint8_t>();
		if (obj.count(USE_ONLY_TCP) != 0) use_only_tcp = obj.at(USE_ONLY_TCP).get<uint8_t>();
		if (obj.count(GUID) != 0) guid = obj.at(GUID).get<std::string>();

		return true;
	}
	catch (nlohmann::json::parse_error& ex)
	{
		spdlog::get("Error")->critical("api::edit_user :: error parse json (byte: {0}, what: {1})", ex.byte, ex.what());
	}
	return false;
}

std::string Command::Serialize()
{
	std::string out = "{" +
		quot(ID) + ":" + std::to_string(id) + "," +
		quot(NAME) + ":" + quot(Common::JSON::Screen(name)) + "," +
		quot(LOGIN) + ":" + quot(Common::JSON::Screen(login)) + "," +
		quot(PASSWORD) + ":" + quot(Common::JSON::Screen(password)) + "," +
		quot(NUMBER) + ":" + std::to_string(number) + ",";

	if (!groups.empty())
	{
		out += quot(GROUPS) + ":[";
		for (auto &g : groups)
		{
			out += g.Serialize() + ",";
		}
		out.pop_back(); // drop last ","
		out += "],";
	}

	out += quot(TIME_LIMIT) + ":" + std::to_string(time_limit) + "," +
		quot(ALLOW_CREATE_CONFERENCE) + (allow_create_conference ? ":1," : ":0,") +
		quot(USE_ONLY_TCP) + (use_only_tcp ? ":1" : ":0") + "," +
		quot(GUID) + ":" + quot(Common::JSON::Screen(guid)) +
	"}";

	return out;
}

}
}
