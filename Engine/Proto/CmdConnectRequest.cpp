/**
 * CmdConnectRequest.cpp - Contains protocol command CONNECT_REQUIEST impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdConnectRequest.h>

#include <Common/Common.h>
#include <Common/JSONSymbolsScreener.h>
#include <Common/Quoter.h>

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

Command::Command(uint32_t client_version_, const std::string &system_, const std::string &login_, const std::string &password_)
	: client_version(client_version_), system(system_), login(login_), password(password_)
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

		client_version = pt.get<uint32_t>(NAME + "." + CLIENT_VERSION);
		system = pt.get<std::string>(NAME + "." + SYSTEM);
		login = pt.get<std::string>(NAME + "." + LOGIN);
		password = pt.get<std::string>(NAME + "." + PASSWORD);

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
		quot(CLIENT_VERSION) + ":" + std::to_string(client_version) + "," +
		quot(SYSTEM) + ":" + quot(system) + "," +
		quot(LOGIN) + ":" + quot(Common::JSON::Screen(login)) + "," +
		quot(PASSWORD) + ":" + quot(Common::JSON::Screen(password)) + "}}";
}

}
}
