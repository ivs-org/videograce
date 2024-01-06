/**
 * CreateUser.cpp - Contains API create user json impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020, 2023
 */

#include <API/CreateUser.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace API
{
namespace CREATE_USER
{

static const std::string GROUP_ID = "group_id";
static const std::string NAME = "name";
static const std::string LOGIN = "login";
static const std::string PASSWORD = "password";
static const std::string NUMBER = "number";
static const std::string TIME_LIMIT = "time_limit";
static const std::string ALLOW_CREATE_CONFERENCE = "allow_create_conference";
static const std::string USE_ONLY_TCP = "use_only_tcp";
static const std::string GUID = "guid";

Command::Command()
	: group_id(0), name(), login(), password(), number(), time_limit(), allow_create_conference(false), use_only_tcp(false), guid()
{
}

Command::Command(int64_t group_id_, std::string_view name_, std::string_view login_, std::string_view password_, uint32_t number_, uint64_t time_limit_, bool allow_create_conference_, bool use_only_tcp_, std::string_view guid_)
	: group_id(group_id_), name(name_), login(login_), password(password_), number(number_), time_limit(time_limit_), allow_create_conference(allow_create_conference_), use_only_tcp(use_only_tcp_), guid(guid_)
{
}

Command::~Command()
{
}

bool Command::Parse(std::string_view message_)
{
	try
	{
		auto j = nlohmann::json::parse(message_);

		if (j.count(GROUP_ID) != 0) group_id = j.at(GROUP_ID).get<int64_t>();
		if (j.count(NAME) != 0) name = j.at(NAME).get<std::string>();
		if (j.count(LOGIN) != 0) login = j.at(LOGIN).get<std::string>();
		if (j.count(PASSWORD) != 0) password = j.at(PASSWORD).get<std::string>();
		if (j.count(NUMBER) != 0) number = j.at(NUMBER).get<uint32_t>();
		if (j.count(TIME_LIMIT) != 0) time_limit = j.at(TIME_LIMIT).get<uint64_t>();
		if (j.count(ALLOW_CREATE_CONFERENCE) != 0) allow_create_conference = j.at(ALLOW_CREATE_CONFERENCE).get<uint8_t>() != 0;
		if (j.count(USE_ONLY_TCP) != 0) use_only_tcp = j.at(USE_ONLY_TCP).get<uint8_t>() != 0;
		if (j.count(GUID) != 0) guid = j.at(GUID).get<std::string>();

		return true;
	}
	catch (nlohmann::json::parse_error& ex)
	{
		spdlog::get("Error")->critical("api::create_user :: error parse json (byte: {0}, what: {1})", ex.byte, ex.what());
	}
	return false;
}

std::string Command::Serialize()
{
	return "{" +
		quot(GROUP_ID) + ":" + std::to_string(group_id) + "," +
		quot(NAME) + ":" + quot(Common::JSON::Screen(name)) + "," +
		quot(LOGIN) + ":" + quot(Common::JSON::Screen(login)) + "," +
		quot(PASSWORD) + ":" + quot(Common::JSON::Screen(password)) + "," +
		quot(NUMBER) + ":" + std::to_string(number) + "," +
		quot(TIME_LIMIT) + ":" + std::to_string(time_limit) + "," +
		quot(ALLOW_CREATE_CONFERENCE) + (allow_create_conference ? ":1," : ":0,") +
		quot(USE_ONLY_TCP) + (use_only_tcp ? ":1" : ":0") + "," +
		quot(GUID) + ":" + quot(Common::JSON::Screen(guid)) +
	"}";
}

}
}
