/**
 * CmdMemberAction.cpp - Contains protocol command WANT_SPEAK impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdWantSpeak.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

namespace Proto
{
namespace WANT_SPEAK
{

static const std::string USER_ID = "user_id";
static const std::string USER_NAME = "user_name";
static const std::string IS_SPEAK = "is_speak";

Command::Command()
	: user_id(0),
	user_name(),
	is_speak(false)
{
}

Command::Command(int64_t user_id_, const std::string &user_name_, bool is_speak_)
	: user_id(user_id_), user_name(user_name_), is_speak(is_speak_)
{
}

Command::~Command()
{
}

bool Command::Parse(std::string_view message)
{
	using boost::property_tree::ptree;

	std::stringstream ss;
	ss << message;

	try
	{
		ptree pt;
		read_json(ss, pt);

		user_id = pt.get<int64_t>(NAME + "." + USER_ID);
		user_name = pt.get<std::string>(NAME + "." + USER_NAME);
		is_speak = pt.get<uint8_t>(NAME + "." + IS_SPEAK) != 0;

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
	return "{" + quot(NAME) + ":{" + quot(USER_ID) + ":" + std::to_string(user_id) + "," +
		quot(USER_NAME) + ":" + quot(user_name) + "," +
		quot(IS_SPEAK) + ":" + std::to_string(is_speak ? 1 : 0) + "}}";
}

}
}
