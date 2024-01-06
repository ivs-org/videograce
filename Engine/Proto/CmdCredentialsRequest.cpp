/**
 * CmdCredentialsRequest.cpp - Contains protocol command CREDENTIALS_REQUEST impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021, 2023
 */

#include <Proto/CmdCredentialsRequest.h>

#include <Common/JSONSymbolsScreener.h>
#include <Common/Quoter.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace CREDENTIALS_REQUEST
{

static const std::string GUID = "guid";

Command::Command()
	: guid()
{
}

Command::Command(std::string_view guid_)
	: guid(guid_)
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

		guid = obj.at(GUID).get<std::string>();

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
		quot(GUID) + ":" + quot(Common::JSON::Screen(guid)) + "}}";
}

}
}
