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

static const std::string TYPE = "type";
static const std::string CLIENT_VERSION = "client_version";
static const std::string SYSTEM = "system";
static const std::string LOGIN = "login";
static const std::string PASSWORD = "password";
static const std::string ACCESS_TOKEN = "access_token";

Command::Command()
	: type(Type::CommandLoop), client_version(0), system(), login(), password(), access_token()
{
}

Command::Command(Type type_, uint32_t client_version_, std::string_view system_, std::string_view login_, std::string_view password_, std::string_view access_token_)
	: type(type_), client_version(client_version_), system(system_), login(login_), password(password_), access_token(access_token_)
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

		if (obj.count(TYPE) != 0) type = static_cast<Type>(obj.at(TYPE).get<uint32_t>());
		
		if (obj.count(CLIENT_VERSION) != 0) client_version = obj.at(CLIENT_VERSION).get<uint32_t>();
		if (obj.count(SYSTEM) != 0) system = obj.at(SYSTEM).get<std::string>();
		
		if (obj.count(LOGIN) != 0) login = obj.at(LOGIN).get<std::string>();
		if (obj.count(PASSWORD) != 0) password = obj.at(PASSWORD).get<std::string>();

		if (obj.count(ACCESS_TOKEN) != 0) access_token = obj.at(ACCESS_TOKEN).get<std::string>();

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
		quot(TYPE) + ":" + std::to_string(static_cast<int32_t>(type)) +
		(client_version != 0 ? "," + quot(CLIENT_VERSION) + ":" + std::to_string(client_version) : "") +
		(!system.empty() ? "," + quot(SYSTEM) + ":" + quot(Common::JSON::Screen(system)) : "") +
		(!login.empty() ? "," + quot(LOGIN) + ":" + quot(Common::JSON::Screen(login)) : "") +
		(!password.empty() ? "," + quot(PASSWORD) + ":" + quot(Common::JSON::Screen(password)) : "") +
		(!access_token.empty() ? "," + quot(ACCESS_TOKEN) + ":" + quot(Common::JSON::Screen(access_token)) : "") + "}}";
}

}
}
