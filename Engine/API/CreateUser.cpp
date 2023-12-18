/**
 * CreateUser.cpp - Contains API create user json impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <API/CreateUser.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

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

Command::Command(int64_t group_id_, const std::string &name_, const std::string &login_, const std::string &password_, uint32_t number_, uint64_t time_limit_, bool allow_create_conference_, bool use_only_tcp_, const std::string &guid_)
	: group_id(group_id_), name(name_), login(login_), password(password_), number(number_), time_limit(time_limit_), allow_create_conference(allow_create_conference_), use_only_tcp(use_only_tcp_), guid(guid_)
{
}

Command::~Command()
{
}

bool Command::Parse(std::string_view message_)
{
	using boost::property_tree::ptree;

	std::stringstream ss;
	ss << message_;

	try
	{
		ptree pt;
		read_json(ss, pt);

		auto group_id_opt = pt.get_optional<int64_t>(GROUP_ID);
		if (group_id_opt) group_id = group_id_opt.get();

		auto name_opt = pt.get_optional<std::string>(NAME);
		if (name_opt) name = name_opt.get();
		
		auto login_opt = pt.get_optional<std::string>(LOGIN);
		if (login_opt) login = login_opt.get();

		auto password_opt = pt.get_optional<std::string>(PASSWORD);
		if (password_opt) password = password_opt.get();

		auto number_opt = pt.get_optional<uint32_t>(NUMBER);
		if (number_opt) number = number_opt.get();

		auto time_limit_opt = pt.get_optional<uint64_t>(TIME_LIMIT);
		if (time_limit_opt) time_limit = time_limit_opt.get();

		auto allow_create_conference_opt = pt.get_optional<uint8_t>(ALLOW_CREATE_CONFERENCE);
		if (allow_create_conference_opt) allow_create_conference = allow_create_conference_opt.get() != 0;

        auto use_only_tcp_opt = pt.get_optional<uint8_t>(USE_ONLY_TCP);
        if (use_only_tcp_opt) use_only_tcp = use_only_tcp_opt.get() != 0;

		auto guid_opt = pt.get_optional<std::string>(GUID);
		if (guid_opt) guid = guid_opt.get();

		return true;
	}
	catch (std::exception const& e)
	{
		DBGTRACE("Error parsing CreateUser %s\n", e.what());
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
