/**
 * CmdConnectToConferenceRequest.cpp - Contains protocol command CONNECT_TO_CONFERENCE_REQUEST impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018, 2023
 */

#include <Proto/CmdConnectToConferenceRequest.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace CONNECT_TO_CONFERENCE_REQUEST
{

static const std::string TAG = "tag";
static const std::string CONNECT_MEMBERS = "connect_members";
static const std::string HAS_CAMERA = "has_camera";
static const std::string HAS_MICROPHONE = "has_microphone";
static const std::string HAS_DEMONSTRATION = "has_demonstration";

Command::Command()
	: tag(), connect_members(false), has_camera(false), has_microphone(false), has_demonstration(false)
{
}

Command::Command(std::string_view tag_)
	: tag(tag_), connect_members(false), has_camera(false), has_microphone(false), has_demonstration(false)
{
}

Command::Command(std::string_view tag_, bool connect_members_, bool has_camera_, bool has_microphone_, bool has_demonstration_)
	: tag(tag_), connect_members(connect_members_), has_camera(has_camera_), has_microphone(has_microphone_), has_demonstration(has_demonstration_)
{
}

Command::~Command()
{
}

bool Command::Parse(std::string_view message)
{
	try
	{
		spdlog::get("System")->trace("proto::{0} :: perform parsing", NAME);

		auto j = nlohmann::json::parse(message);
		auto obj = j.get<nlohmann::json::object_t>().at(NAME);

		tag = obj.at(TAG).get<std::string>();

		if (obj.count(CONNECT_MEMBERS) != 0) connect_members = obj.at(CONNECT_MEMBERS).get<uint8_t>() != 0;
		if (obj.count(HAS_CAMERA) != 0) has_camera = obj.at(HAS_CAMERA).get<uint8_t>() != 0;
		if (obj.count(HAS_MICROPHONE) != 0) has_microphone = obj.at(HAS_MICROPHONE).get<uint8_t>() != 0;
		if (obj.count(HAS_DEMONSTRATION) != 0) has_demonstration = obj.at(HAS_DEMONSTRATION).get<uint8_t>() != 0;
		
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
		quot(TAG) + ":" + quot(Common::JSON::Screen(tag)) +
		(connect_members ? "," + quot(CONNECT_MEMBERS) + ":1" : "") +
		(has_camera ? "," + quot(HAS_CAMERA) + ":1" : "") +
		(has_microphone ? "," + quot(HAS_MICROPHONE) + ":1" : "") +
        (has_demonstration ? "," + quot(HAS_DEMONSTRATION) + ":1" : "") + "}}";
}

}
}
