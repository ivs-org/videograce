/**
 * CmdUpdateGrants.cpp - Contains protocol command UPDATE_GRANTS impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018, 2023
 */

#include <Proto/CmdUpdateGrants.h>

#include <Common/Quoter.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace UPDATE_GRANTS
{

static const std::string GRANTS = "grants";

Command::Command()
	: grants(0)
{
}

Command::Command(uint32_t grants_)
	: grants(grants_)
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

		grants = obj.at(GRANTS).get<uint32_t>();

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
	return "{" + quot(NAME) + ":{" + quot(GRANTS) + ":" + std::to_string(grants) + "}}";
}

}
}
