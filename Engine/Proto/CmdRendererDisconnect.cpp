/**
 * CmdRendererDisconnect.cpp - Contains protocol command RENDERER_DISCONNECT impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018, 2023
 */

#include <Proto/CmdRendererDisconnect.h>

#include <Common/Quoter.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace RENDERER_DISCONNECT
{

static const std::string DEVICE_ID = "device_id";
static const std::string SSRC = "ssrc";

Command::Command()
	: device_id(0),
	ssrc(0)
{
}

Command::Command(uint32_t device_id_, uint32_t ssrc_)
	: device_id(device_id_), ssrc(ssrc_)
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

		device_id = obj.at(DEVICE_ID).get<uint32_t>();
		ssrc = obj.at(SSRC).get<uint32_t>();

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
	return "{" + quot(NAME) + ":{" + quot(DEVICE_ID) + ":" + std::to_string(device_id) + "," +
		quot(SSRC) + ":" + std::to_string(ssrc) + "}}";
}

}
}
