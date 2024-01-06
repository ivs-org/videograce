/**
 * RegisterUser.cpp - Contains API register user json impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2024
 */

#include <API/RegisterUser.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace API
{
namespace REGISTER_USER
{

static const std::string NAME = "name";
static const std::string LOGIN = "login";
static const std::string PASSWORD = "password";
static const std::string CAPTCHA = "captcha";

Command::Command()
	: name(), login(), password(), captcha()
{
}

Command::Command(std::string_view name_, std::string_view login_, std::string_view password_, std::string_view captcha_)
	: name(name_), login(login_), password(password_), captcha(captcha_)
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

		if (j.count(NAME) != 0) name = j.at(NAME).get<std::string>();
		if (j.count(LOGIN) != 0) login = j.at(LOGIN).get<std::string>();
		if (j.count(PASSWORD) != 0) password = j.at(PASSWORD).get<std::string>();
		if (j.count(CAPTCHA) != 0) captcha = j.at(CAPTCHA).get<std::string>();

		return true;
	}
	catch (nlohmann::json::parse_error& ex)
	{
		spdlog::get("Error")->critical("api::register_user :: error parse json (byte: {0}, what: {1})", ex.byte, ex.what());
	}
	return false;
}

std::string Command::Serialize()
{
	return "{" +
		quot(NAME) + ":" + quot(Common::JSON::Screen(name)) + "," +
		quot(LOGIN) + ":" + quot(Common::JSON::Screen(login)) + "," +
		quot(PASSWORD) + ":" + quot(Common::JSON::Screen(password)) + "," +
		quot(CAPTCHA) + ":" + quot(Common::JSON::Screen(captcha)) +
	"}";
}

}
}
