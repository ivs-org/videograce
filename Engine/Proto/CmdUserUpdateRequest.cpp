/**
 * CmdUserUpdateRequest.cpp - Contains protocol command USER_UPDATE_REQUEST impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021, 2023
 */

#include <Proto/CmdUserUpdateRequest.h>

#include <Common/JSONSymbolsScreener.h>
#include <Common/Quoter.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace USER_UPDATE_REQUEST
{

static const std::string ACTION = "action";
static const std::string ID = "id";
static const std::string NAME_ = "name";
static const std::string AVATAR = "avatar";
static const std::string LOGIN = "login";
static const std::string PASSWORD = "password";


Command::Command()
	: action(Action::Undefined), id(0), name(), login(), password()
{
}

Command::Command(Action action_,
	int64_t id_,
	std::string_view name_,
	std::string_view avatar_,
	std::string_view login_,
	std::string_view password_)
	: action(action_),
	id(id_),
	name(name_),
	login(login_),
	password(password_)
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

		action = static_cast<Action>(obj.at(ACTION).get<uint32_t>());

		if (obj.count(ID) != 0) id = obj.at(ID).get<int64_t>();
		if (obj.count(NAME_) != 0) name = obj.at(NAME_).get<std::string>();
		if (obj.count(AVATAR) != 0) avatar = obj.at(AVATAR).get<std::string>();
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
		quot(ACTION) + ":" + std::to_string(static_cast<int32_t>(action)) + 
		(id != 0 ? "," + quot(ID) + ":" + std::to_string(id) : "") +
		(!name.empty() ? "," + quot(NAME_) + ":" + quot(Common::JSON::Screen(name)) : "") +
		(!avatar.empty() ? "," + quot(AVATAR) + ":" + quot(Common::JSON::Screen(avatar)) : "") +
		(!login.empty() ? "," + quot(LOGIN) + ":" + quot(Common::JSON::Screen(login)) : "") +
		(!password.empty() ? "," + quot(PASSWORD) + ":" + quot(Common::JSON::Screen(password)) : "") 
		+ "}}";
}

}
}
