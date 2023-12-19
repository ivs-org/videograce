/**
 * CmdConnectResponse.cpp - Contains protocol command CONNECT_RESPONSE impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdConnectResponse.h>

#include <Common/Common.h>
#include <Common/JSONSymbolsScreener.h>
#include <Common/Quoter.h>

namespace Proto
{
namespace CONNECT_RESPONSE
{

static const std::string RESULT = "result";
static const std::string SERVER_VERSION = "server_version";
static const std::string ID = "id";
static const std::string CONNECTION_ID = "connection_id";
static const std::string NAME_ = "name";
static const std::string REDIRECT_URL = "redirect_url";
static const std::string SECURE_KEY = "secure_key";
static const std::string SERVER_NAME = "server_name";
static const std::string OPTIONS = "options";
static const std::string GRANTS = "grants";
static const std::string MAX_OUTPUT_BITRATE = "max_output_bitrate";
static const std::string REDUCED_FRAME_RATE = "reduced_frame_rate";

Command::Command()
	: result(Result::Undefined), server_version(0), id(0), connection_id(0), name(), redirect_url(), secure_key(), options(0), grants(0), max_output_bitrate(0), reduced_frame_rate(0)
{
}

Command::Command(Result result_, uint32_t server_version_, int64_t id_, uint32_t connection_id_, std::string_view name_, std::string_view redirect_url_, std::string_view secure_key_, std::string_view server_name_, uint32_t options_, uint32_t grants_, uint32_t max_output_bitrate_, uint16_t reduced_frame_rate_)
	: result(result_), server_version(server_version_), id(id_), connection_id(connection_id_), name(name_), redirect_url(redirect_url_), secure_key(secure_key_), server_name(server_name_), options(options_), grants(grants_), max_output_bitrate(max_output_bitrate_), reduced_frame_rate(reduced_frame_rate_)
{
}

Command::Command(Result result_)
	: result(result_), server_version(0), id(0), connection_id(0), name(), redirect_url(), secure_key(), server_name(), options(0), grants(0), max_output_bitrate(0), reduced_frame_rate(0)
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
		server_version = pt.get<uint32_t>(NAME + "." + SERVER_VERSION);
		id = pt.get<int64_t>(NAME + "." + ID);
		connection_id = pt.get<uint32_t>(NAME + "." + CONNECTION_ID);
		name = pt.get<std::string>(NAME + "." + NAME_);

		auto redirect_url_opt = pt.get_optional<std::string>(NAME + "." + REDIRECT_URL);
		if (redirect_url_opt) redirect_url = redirect_url_opt.get();

		auto secure_key_opt = pt.get_optional<std::string>(NAME + "." + SECURE_KEY);
		if (secure_key_opt) secure_key = secure_key_opt.get();

		auto server_name_opt = pt.get_optional<std::string>(NAME + "." + SERVER_NAME);
		if (server_name_opt) server_name = server_name_opt.get();
				
		options = pt.get<uint32_t>(NAME + "." + OPTIONS);
		grants = pt.get<uint32_t>(NAME + "." + GRANTS);
		max_output_bitrate = pt.get<uint32_t>(NAME + "." + MAX_OUTPUT_BITRATE);
		
		auto reduced_frame_rate_opt = pt.get_optional<uint16_t>(NAME + "." + REDUCED_FRAME_RATE);
		if (reduced_frame_rate_opt) reduced_frame_rate = reduced_frame_rate_opt.get();

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
		quot(RESULT) + ":" + std::to_string(static_cast<int32_t>(result)) + "," +
		quot(SERVER_VERSION) + ":" + std::to_string(server_version) + "," +
		quot(ID) + ":" + std::to_string(id) + "," +
		quot(CONNECTION_ID) + ":" + std::to_string(connection_id) + "," +
		quot(NAME_) + ":" + quot(Common::JSON::Screen(name)) + "," +
		(!redirect_url.empty() ? quot(REDIRECT_URL) + ":" + quot(Common::JSON::Screen(redirect_url)) + "," : "") +
		(!secure_key.empty() ? quot(SECURE_KEY) + ":" + quot(Common::JSON::Screen(secure_key)) + "," : "") +
		(!server_name.empty() ? quot(SERVER_NAME) + ":" + quot(Common::JSON::Screen(server_name)) + "," : "") +
		quot(OPTIONS) + ":" + std::to_string(options) + "," + 
		quot(GRANTS) + ":" + std::to_string(grants) + "," +
		quot(MAX_OUTPUT_BITRATE) + ":" + std::to_string(max_output_bitrate) + "," +
		quot(REDUCED_FRAME_RATE) + ":" + std::to_string(reduced_frame_rate) + "}}";
}

}
}
