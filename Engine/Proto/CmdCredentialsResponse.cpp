/**
 * CmdCredentialsResponse.cpp - Contains protocol command CREDENTIALS_RESPONSE impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdCredentialsResponse.h>

#include <Common/Common.h>
#include <Common/JSONSymbolsScreener.h>
#include <Common/Quoter.h>

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
	using boost::property_tree::ptree;

	std::stringstream ss;
	ss << message;

	try
	{
		ptree pt;
		read_json(ss, pt);

		result = static_cast<Result>(pt.get<uint32_t>(NAME + "." + RESULT));
		
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
		quot(RESULT) + ":" + std::to_string(static_cast<int32_t>(result)) +
		(!login.empty() ? "," + quot(LOGIN) + ":" + quot(Common::JSON::Screen(login)) : "") +
		(!password.empty() ? "," + quot(PASSWORD) + ":" + quot(Common::JSON::Screen(password)) : "" ) + "}}";
}

}
}
