/**
 * CmdUserUpdateRequest.cpp - Contains protocol command USER_UPDATE_REQUEST impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdUserUpdateRequest.h>

#include <Common/Common.h>
#include <Common/JSONSymbolsScreener.h>
#include <Common/Quoter.h>

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
	using boost::property_tree::ptree;

	std::stringstream ss;
	ss << message;

	try
	{
		ptree pt;
		read_json(ss, pt);

		action = static_cast<Action>(pt.get<uint32_t>(NAME + "." + ACTION));

		auto id_opt = pt.get_optional<int64_t>(NAME + "." + ID);
		if (id_opt) id = id_opt.get();

		auto name_opt = pt.get_optional<std::string>(NAME + "." + NAME_);
		if (name_opt) name = name_opt.get();

		auto avatar_opt = pt.get_optional<std::string>(NAME + "." + AVATAR);
		if (avatar_opt) avatar = avatar_opt.get();

		auto login_opt = pt.get_optional<std::string>(NAME + "." + LOGIN);
		if (login_opt) login = login_opt.get();

		auto password_opt = pt.get_optional<std::string>(NAME + "." + PASSWORD);
		if (password_opt) password = password_opt.get();

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
