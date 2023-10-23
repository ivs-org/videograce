/**
 * ServerInfo.cpp - Contains API server info json impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <API/ServerInfo.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

namespace API
{
namespace SERVER_INFO
{

static const std::string COMPANY = "company";
static const std::string SERVER_ID = "server_id";
static const std::string ADDRESS = "address";
static const std::string PORT = "port";
static const std::string ENABLED_CRYPT = "enabled_crypt";
static const std::string TRANSLATORS_COUNT = "translators_count";
static const std::string ADMIN_EMAIL = "admin_email";

Command::Command()
	: company(), server_id(), address(), port(0), enabled_crypt(false), translators_count(0), admin_email()
{
}

Command::Command(const std::string &company_, const std::string &server_id_, const std::string &address_, uint16_t port_, bool enabled_crypt_, uint16_t translators_count_, const std::string &admin_email_)
	: company(company_), server_id(server_id_), address(address_), port(port_), enabled_crypt(enabled_crypt_), translators_count(translators_count_), admin_email(admin_email_)
{
}

Command::~Command()
{
}

bool Command::Parse(const std::string &message_)
{
	using boost::property_tree::ptree;

	std::stringstream ss;
	ss << message_;

	try
	{
		ptree pt;
		read_json(ss, pt);

		auto company_opt = pt.get_optional<std::string>(COMPANY);
		if (company_opt) company = company_opt.get();

		auto server_id_opt = pt.get_optional<std::string>(SERVER_ID);
		if (server_id_opt) server_id = server_id_opt.get();

		auto address_opt = pt.get_optional<std::string>(ADDRESS);
		if (address_opt) address = address_opt.get();

		auto port_opt = pt.get_optional<uint16_t>(PORT);
		if (port_opt) port = port_opt.get();

		auto enabled_crypt_opt = pt.get_optional<uint8_t>(ENABLED_CRYPT);
		if (enabled_crypt_opt) enabled_crypt = enabled_crypt_opt.get() != 0;

		auto translators_count_opt = pt.get_optional<uint16_t>(TRANSLATORS_COUNT);
		if (translators_count_opt) translators_count = translators_count_opt.get();
		
		auto admin_email_opt = pt.get_optional<std::string>(ADMIN_EMAIL);
		if (admin_email_opt) admin_email = admin_email_opt.get();

		return true;
	}
	catch (std::exception const& e)
	{
		DBGTRACE("Error parsing ServerInfo %s\n", e.what());
	}
	return false;
}

std::string Command::Serialize()
{
	return "{" + 
		(quot(COMPANY) + ":" + quot(Common::JSON::Screen(company)) + ",") +
		(!server_id.empty() ? quot(SERVER_ID) + ":" + quot(Common::JSON::Screen(server_id)) + "," : "") + 
		(quot(ADDRESS) + ":" + quot(Common::JSON::Screen(address)) + ",") +
		(quot(PORT) + ":" + std::to_string(port) + ",") +
		(enabled_crypt ? quot(ENABLED_CRYPT) + ":1," : "") +
		(quot(TRANSLATORS_COUNT) + ":" + std::to_string(translators_count) + ",") +
		(!admin_email.empty() ? quot(ADMIN_EMAIL) + ":" + quot(Common::JSON::Screen(admin_email)) : "") + "}";
}

}
}
