/**
 * CmdCredentialsResponse.cpp - Contains protocol command CREDENTIALS_RESPONSE impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2023
 */

#include <Proto/CmdCredentialsResponse.h>

#include <Common/JSONSymbolsScreener.h>
#include <Common/Quoter.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace CREDENTIALS_RESPONSE
{

static const std::string RESULT = "result";
static const std::string LOGIN = "login";
static const std::string PASSWORD = "password";

Command::Command()
	: result(Result::Undefined), login(), password()
{
}

Command::Command(Result result_, std::string_view login_, std::string_view password_)
	: result(result_), login(login_), password(password_)
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

		result = static_cast<Result>(obj.at(RESULT).get<uint32_t>());

		if (obj.count(LOGIN) != 0) login = obj.at(LOGIN).get<std::string>();
		if (obj.count(PASSWORD) != 0) password = obj.at(PASSWORD).get<std::string>();
		
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
	return "{" + quot(NAME) + ":{" + 
		quot(RESULT) + ":" + std::to_string(static_cast<int32_t>(result)) +
		(!login.empty() ? "," + quot(LOGIN) + ":" + quot(Common::JSON::Screen(login)) : "") +
		(!password.empty() ? "," + quot(PASSWORD) + ":" + quot(Common::JSON::Screen(password)) : "" ) + "}}";
}

}
}
