/**
 * CmdChangeServer.cpp - Contains protocol command CHANGE_SERVER impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020, 2023
 */

#include <Proto/CmdChangeServer.h>

#include <Common/JSONSymbolsScreener.h>
#include <Common/Quoter.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace CHANGE_SERVER
{

static const std::string URL = "url";

Command::Command()
	: url()
{
}

Command::Command(std::string_view url_)
	: url(url_)
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

		url = obj.at(URL).get<std::string>();
		
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
		quot(URL) + ":" + quot(Common::JSON::Screen(url)) + "}}";
}

}
}
