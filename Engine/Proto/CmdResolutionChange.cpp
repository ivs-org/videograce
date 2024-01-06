/**
 * CmdResolutionChange.cpp - Contains protocol command RESOLUTION_CHANGE impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016, 2023
 */

#include <Proto/CmdResolutionChange.h>

#include <Common/Quoter.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace RESOLUTION_CHANGE
{

static const std::string ID = "id";
static const std::string RESOLUTION = "resolution";

Command::Command()
	: id(0),
	resolution(0)
{
}

Command::Command(uint32_t id_, uint32_t resolution_)
	: id(id_), resolution(resolution_)
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

		id = obj.at(ID).get<uint32_t>();
		resolution = obj.at(RESOLUTION).get<uint32_t>();

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
	return "{" + quot(NAME) + ":{" + quot(ID) + ":" + std::to_string(id) + "," + quot(RESOLUTION) + ":" + std::to_string(resolution) + "}}";
}

}
}
