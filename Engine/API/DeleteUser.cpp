/**
 * DeleteUser.cpp - Contains API delete user json impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020, 2023
 */

#include <API/DeleteUser.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace API
{
namespace DELETE_USER
{

static const std::string LOGIN = "login";

Command::Command()
	: login()
{
}

Command::Command(std::string_view login_)
	: login(login_)
{
}

Command::~Command()
{
}

bool Command::Parse(std::string_view message_)
{
	try
	{
		spdlog::get("System")->trace("api::delete_user :: perform parsing");

		auto j = nlohmann::json::parse(message_);

		if (j.count(LOGIN) != 0) login = j.at(LOGIN).get<std::string>();

		return true;
	}
	catch (nlohmann::json::parse_error& ex)
	{
		spdlog::get("Error")->critical("api::delete_user :: error parse json (byte: {0}, what: {1})", ex.byte, ex.what());
	}

	return false;
}

std::string Command::Serialize()
{
	return "{" + 
		quot(LOGIN) + ":" + quot(Common::JSON::Screen(login)) + "}";
}

}
}
