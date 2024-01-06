/**
 * CmdConnectResponse.cpp - Contains protocol command CONNECT_RESPONSE impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018, 2023
 */

#include <Proto/CmdConnectResponse.h>

#include <Common/JSONSymbolsScreener.h>
#include <Common/Quoter.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace CONNECT_RESPONSE
{

static const std::string RESULT = "result";
static const std::string SERVER_VERSION = "server_version";
static const std::string ID = "id";
static const std::string CONNECTION_ID = "connection_id";
static const std::string NAME_ = "name";
static const std::string ACCESS_TOKEN = "access_token";
static const std::string REDIRECT_URL = "redirect_url";
static const std::string SECURE_KEY = "secure_key";
static const std::string SERVER_NAME = "server_name";
static const std::string OPTIONS = "options";
static const std::string GRANTS = "grants";
static const std::string MAX_OUTPUT_BITRATE = "max_output_bitrate";

Command::Command()
	: result(Result::Undefined), server_version(0), id(0), connection_id(0), access_token(), name(), redirect_url(), secure_key(), options(0), grants(0), max_output_bitrate(0)
{
}

Command::Command(Result result_, uint32_t server_version_, int64_t id_, int64_t connection_id_, std::string_view access_token_, std::string_view name_, std::string_view redirect_url_, std::string_view secure_key_, std::string_view server_name_, uint32_t options_, uint32_t grants_, uint32_t max_output_bitrate_)
	: result(result_), server_version(server_version_), id(id_), connection_id(connection_id_), access_token(access_token_), name(name_), redirect_url(redirect_url_), secure_key(secure_key_), server_name(server_name_), options(options_), grants(grants_), max_output_bitrate(max_output_bitrate_)
{
}

Command::Command(Result result_)
	: result(result_), server_version(0), id(0), connection_id(0), access_token(), name(), redirect_url(), secure_key(), server_name(), options(0), grants(0), max_output_bitrate(0)
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

		result = static_cast<Result>(obj.at(RESULT).get<uint32_t>());
		
		if (obj.count(SERVER_VERSION) != 0) server_version = obj.at(SERVER_VERSION).get<uint32_t>();
		if (obj.count(ID) != 0) id = obj.at(ID).get<uint64_t>();
		if (obj.count(CONNECTION_ID) != 0) connection_id = obj.at(CONNECTION_ID).get<int64_t>();
		if (obj.count(ACCESS_TOKEN) != 0) access_token = obj.at(ACCESS_TOKEN).get<std::string>();

		if (obj.count(NAME_) != 0) name = obj.at(NAME_).get<std::string>();
		if (obj.count(REDIRECT_URL) != 0) redirect_url = obj.at(REDIRECT_URL).get<std::string>();
		if (obj.count(SECURE_KEY) != 0) secure_key = obj.at(SECURE_KEY).get<std::string>();
		if (obj.count(SERVER_NAME) != 0) server_name = obj.at(SERVER_NAME).get<std::string>();
		
		if (obj.count(OPTIONS) != 0) options = obj.at(OPTIONS).get<uint32_t>();
		if (obj.count(GRANTS) != 0) grants = obj.at(GRANTS).get<uint32_t>();
		if (obj.count(MAX_OUTPUT_BITRATE) != 0) max_output_bitrate = obj.at(MAX_OUTPUT_BITRATE).get<uint32_t>();

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
		quot(RESULT) + ":" + std::to_string(static_cast<int32_t>(result)) + 
		(server_version != 0 ? "," + quot(SERVER_VERSION) + ":" + std::to_string(server_version) : "") +
		(id != 0 ? "," + quot(ID) + ":" + std::to_string(id) : "") +
		(connection_id != 0 ? "," + quot(CONNECTION_ID) + ":" + std::to_string(connection_id) : "") +
		(!access_token.empty() ? "," + quot(ACCESS_TOKEN) + ":" + quot(Common::JSON::Screen(access_token)) : "") +
		(!name.empty() ? "," + quot(NAME_) + ":" + quot(Common::JSON::Screen(name)) : "") +
		(!redirect_url.empty() ? "," + quot(REDIRECT_URL) + ":" + quot(Common::JSON::Screen(redirect_url)) : "") +
		(!secure_key.empty() ? "," + quot(SECURE_KEY) + ":" + quot(Common::JSON::Screen(secure_key)) : "") +
		(!server_name.empty() ? "," + quot(SERVER_NAME) + ":" + quot(Common::JSON::Screen(server_name)) : "") +
		(options != 0 ? "," + quot(OPTIONS) + ":" + std::to_string(options) : "") +
		(grants != 0 ? "," + quot(GRANTS) + ":" + std::to_string(grants) : "") + "}}";
}

}
}
