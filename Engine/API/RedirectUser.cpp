/**
 * RedirectUser.cpp - Contains API redirect user json impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <API/RedirectUser.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

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

Command::Command(const std::string &login_, const std::string &url_)
	: login(login_), url(url_)
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

		auto url_opt = pt.get_optional<std::string>(URL);
		if (url_opt) url = url_opt.get();
						
		return true;
	}
	catch (std::exception const& e)
	{
		DBGTRACE("Error parsing RedirectUser %s\n", e.what());
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
