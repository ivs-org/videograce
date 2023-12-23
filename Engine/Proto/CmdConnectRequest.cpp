/**
 * CmdConnectRequest.cpp - Contains protocol command CONNECT_REQUIEST impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018, 2023
 */

#include <Proto/CmdConnectRequest.h>

#include <Common/JSONSymbolsScreener.h>
#include <Common/Quoter.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace CONNECT_REQUEST
{

static const std::string CLIENT_VERSION = "client_version";
static const std::string SYSTEM = "system";
static const std::string LOGIN = "login";
static const std::string PASSWORD = "password";

Command::Command()
	: client_version(0), system(), login(), password()
{
}

Command::Command(uint32_t client_version_, std::string_view system_, std::string_view login_, std::string_view password_)
	: client_version(client_version_), system(system_), login(login_), password(password_)
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

		client_version = obj.at(CLIENT_VERSION).get<uint32_t>();
		system = obj.at(SYSTEM).get<std::string>();
		login = obj.at(LOGIN).get<std::string>();
		password = obj.at(PASSWORD).get<std::string>();

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
		quot(CLIENT_VERSION) + ":" + std::to_string(client_version) + "," +
		quot(SYSTEM) + ":" + quot(system) + "," +
		quot(LOGIN) + ":" + quot(Common::JSON::Screen(login)) + "," +
		quot(PASSWORD) + ":" + quot(Common::JSON::Screen(password)) + "}}";
}

}
}
