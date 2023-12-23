/**
 * CmdMemberAction.cpp - Contains protocol command WANT_SPEAK impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018, 2023
 */

#include <Proto/CmdWantSpeak.h>

#include <Common/Quoter.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

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

Command::Command(int64_t user_id_, std::string_view user_name_, bool is_speak_)
	: user_id(user_id_), user_name(user_name_), is_speak(is_speak_)
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
		auto obj = j.get<nlohmann::json::object_t>().at(NAME);

		user_id = obj.at(USER_ID).get<int64_t>();
		user_name = obj.at(USER_NAME).get<std::string>();
		is_speak = obj.at(IS_SPEAK).get<int8_t>() != 0;

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
	return "{" + quot(NAME) + ":{" + quot(USER_ID) + ":" + std::to_string(user_id) + "," +
		quot(USER_NAME) + ":" + quot(user_name) + "," +
		quot(IS_SPEAK) + ":" + std::to_string(is_speak ? 1 : 0) + "}}";
}

}
}
