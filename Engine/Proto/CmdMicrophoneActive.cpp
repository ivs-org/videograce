/**
 * CmdMicrophoneActive.cpp - Contains protocol command MICROPHONE_ACTIVE impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021, 2023
 */

#include <Proto/CmdMicrophoneActive.h>

#include <Common/Quoter.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace MICROPHONE_ACTIVE
{

static const std::string ACTIVE_TYPE = "active_type";
static const std::string DEVICE_ID = "device_id";
static const std::string CLIENT_ID = "client_id";

Command::Command()
	: active_type(ActiveType::Undefined),
	device_id(0),
	client_id(0)
{
}

Command::Command(ActiveType active_type_, uint32_t device_id_, int64_t client_id_)
	: active_type(active_type_), device_id(device_id_), client_id(client_id_)
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

		active_type = static_cast<ActiveType>(obj.at(ACTIVE_TYPE).get<uint32_t>());
		device_id = obj.at(DEVICE_ID).get<uint32_t>();
		client_id = obj.at(CLIENT_ID).get<int64_t>();
		
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
	return "{" + quot(NAME) + ":{" + quot(ACTIVE_TYPE) + ":" + std::to_string(static_cast<int32_t>(active_type)) + "," +
		quot(DEVICE_ID) + ":" + std::to_string(device_id) + "," +
		quot(CLIENT_ID) + ":" + std::to_string(client_id) + "}}";
}

}
}
