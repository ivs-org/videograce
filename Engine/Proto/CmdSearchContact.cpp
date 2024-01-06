/**
 * CmdSearchContact.cpp - Contains protocol command SEARCH_CONTACT impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018, 2023
 */

#include <Proto/CmdSearchContact.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace SEARCH_CONTACT
{

static const std::string QUERY = "query";

Command::Command()
	: query()
{
}

Command::Command(std::string_view query_)
	: query(query_)
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

		query = obj.at(QUERY).get<std::string>();
		
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
	return "{" + quot(NAME) + ":{" + quot(QUERY) + ":" + quot(Common::JSON::Screen(query)) + "}}";
}

}
}
