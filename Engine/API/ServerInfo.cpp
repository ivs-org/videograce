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

static const std::string INSTALLATION_ID = "installation_id";
static const std::string LICENCE_KEY = "license_key";
static const std::string KEY_EXPIRED = "key_expired";
static const std::string CHANNELS_COUNT = "channels_count";

static const std::string ADDRESS = "address";
static const std::string ADMIN_EMAIL = "admin_email";

static const std::string ENABLED_CRYPT = "enabled_crypt";

static const std::string COMMAND_PORT = "command_port";
static const std::string FIRST_AV_PORT = "first_av_port";
static const std::string TCP_PORT = "tcp_port";
static const std::string TRANSLATORS_COUNT = "translators_count";

static const std::string SERVER_VERSION = "server_version";
static const std::string SYSTEM = "system";

Command::Command()
	: company(),
	server_id(),
	installation_id(),
	license_key(),
	key_expired(-1),
	channels_count(-1),
	address(),
	admin_email(),
	enabled_crypt(-1),
	command_port(0),
	first_av_port(0),
	tcp_port(0),
	translators_count(0),
	server_version(),
	system()
{
}

Command::Command(std::string_view company_,
	std::string_view server_id_,

	std::string_view installation_id_,
	std::string_view license_key_,
	int64_t key_expired_,
	int32_t channels_count_,

	std::string_view address_,
	std::string_view admin_email_,

	bool enabled_crypt_,

	uint16_t command_port_,
	uint16_t first_av_port_,
	uint16_t tcp_port_,
	uint16_t translators_count_,

	std::string_view server_version_,
	std::string_view system_)
	: company(company_),
	server_id(server_id_),
	installation_id(installation_id_),
	license_key(license_key_),
	key_expired(key_expired_),
	channels_count(channels_count_),
	address(address_),
	admin_email(admin_email_),
	enabled_crypt(enabled_crypt_ ? 1 : 0),
	command_port(command_port_),
	first_av_port(first_av_port_),
	tcp_port(tcp_port_),
	translators_count(translators_count_),
	server_version(server_version_),
	system(system_)
{
}

Command::Command(std::string_view company_,
	std::string_view server_id_,

	std::string_view address_,
	std::string_view admin_email_,

	std::string_view server_version_,
	std::string_view system_)
	: company(company_),
	server_id(server_id_),
	installation_id(),
	license_key(),
	key_expired(-1),
	channels_count(-1),
	address(address_),
	admin_email(admin_email_),
	enabled_crypt(-1),
	command_port(0),
	first_av_port(0),
	tcp_port(0),
	translators_count(0),
	server_version(server_version_),
	system(system_)
{
}

Command::~Command()
{
}

bool Command::Parse(const std::string &message_)
{
	/// No need to parse informational API messages

	return false;
}

std::string Command::Serialize()
{
	return "{" + 
		(quot(COMPANY) + ":" + quot(Common::JSON::Screen(company)) + ",") +
		(!server_id.empty() ? quot(SERVER_ID) + ":" + quot(Common::JSON::Screen(server_id)) + "," : "") + 

		(!installation_id.empty() ? quot(INSTALLATION_ID) + ":" + quot(Common::JSON::Screen(installation_id)) + "," : "") +
		(!license_key.empty() ? quot(LICENCE_KEY) + ":" + quot(Common::JSON::Screen(license_key)) + "," : "") +
		(key_expired != -1 ? quot(KEY_EXPIRED) + ":" + std::to_string(key_expired) + "," : "") +
		(channels_count != -1 ? quot(CHANNELS_COUNT) + ":" + std::to_string(channels_count) + "," : "") +
		
		(!address.empty() ? quot(ADDRESS) + ":" + quot(Common::JSON::Screen(address)) + "," : "") +
		(!admin_email.empty() ? quot(ADMIN_EMAIL) + ":" + quot(Common::JSON::Screen(admin_email)) + "," : "") +

		(enabled_crypt != -1 ? quot(ENABLED_CRYPT) + (enabled_crypt ? ":1," : ":0,") : "") +

		(command_port != 0 ? quot(COMMAND_PORT) + ":" + std::to_string(command_port) + "," : "") +
		(first_av_port != 0 ? quot(FIRST_AV_PORT) + ":" + std::to_string(first_av_port) + "," : "") +
		(tcp_port != 0 ? quot(TCP_PORT) + ":" + std::to_string(tcp_port) + "," : "") +
		(translators_count != 0 ? quot(TRANSLATORS_COUNT) + ":" + std::to_string(translators_count) + "," : "") +

		(quot(SERVER_VERSION) + ":" + quot(Common::JSON::Screen(server_version))) + "," +
		(quot(SYSTEM) + ":" + quot(Common::JSON::Screen(system))) + "}";
}

}
}
