/**
 * RedirectUser.cpp - Contains API redirect user json impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020, 2023
 */

#include <API/RedirectUser.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace API
{
namespace REDIRECT_USER
{

static const std::string LOGIN = "login";
static const std::string URL = "url";

Command::Command()
	: login(), url()
{
}

Command::Command(std::string_view login_, std::string_view url_)
	: login(login_), url(url_)
{
}

Command::~Command()
{
}

bool Command::Parse(std::string_view message_)
{
	try
	{
		spdlog::get("System")->trace("api::redirect_user :: perform parsing");

		auto j = nlohmann::json::parse(message_);

		if (j.count(LOGIN) != 0) login = j.at(LOGIN).get<std::string>();
		if (j.count(URL) != 0) url = j.at(URL).get<std::string>();

		return true;
	}
	catch (nlohmann::json::parse_error& ex)
	{
		spdlog::get("Error")->critical("api::redirect_user :: error parse json (byte: {0}, what: {1})", ex.byte, ex.what());
	}
	return false;
}

std::string Command::Serialize()
{
	return "{" + 
		quot(LOGIN) + ":" + quot(Common::JSON::Screen(login)) + "," +
		quot(URL) + ":" + quot(Common::JSON::Screen(url)) + "}";
}

}
}
