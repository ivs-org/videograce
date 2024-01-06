/**
 * CmdLoadMessages.cpp - Contains protocol command LOAD_MESSAGES impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2019, 2023
 */

#include <Proto/CmdLoadMessages.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace LOAD_MESSAGES
{

static const std::string FROM = "from_dt";

Command::Command()
	: from()
{
}

Command::Command(uint64_t from_)
	: from(from_)
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

		if (obj.count(FROM) != 0) from = obj.at(FROM).get<uint64_t>();

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
	return "{" + quot(NAME) + ":{" + (from != 0 ? quot(FROM) + ":" + std::to_string(from) : "") + "}}";
}

}
}
