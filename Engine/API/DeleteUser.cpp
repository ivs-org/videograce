/**
 * DeleteUser.cpp - Contains API delete user json impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <API/DeleteUser.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

namespace API
{
namespace DELETE_USER
{

static const std::string LOGIN = "login";

Command::Command()
	: login()
{
}

Command::Command(const std::string &login_)
	: login(login_)
{
}

Command::~Command()
{
}

bool Command::Parse(std::string_view message_)
{
	using boost::property_tree::ptree;

	std::stringstream ss;
	ss << message_;

	try
	{
		ptree pt;
		read_json(ss, pt);

		auto login_opt = pt.get_optional<std::string>(LOGIN);
		if (login_opt) login = login_opt.get();
						
		return true;
	}
	catch (std::exception const& e)
	{
		DBGTRACE("Error parsing DeleteUser %s\n", e.what());
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
