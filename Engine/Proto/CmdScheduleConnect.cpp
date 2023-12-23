/**
 * CmdScheduleConnect.cpp - Contains protocol command SCHEDULE_CONNECT impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016, 2023
 */

#include <Proto/CmdScheduleConnect.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace SCHEDULE_CONNECT
{

static const std::string TAG = "tag";
static const std::string NAME_ = "name";
static const std::string TIME_LIMIT = "time_limit";

Command::Command()
	: tag(),
	name(),
	time_limit(0)
{
}

Command::Command(std::string_view tag_, std::string_view name_, uint64_t time_limit_)
	: tag(tag_), name(name_), time_limit(time_limit_)
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
		name = obj.at(NAME_).get<std::string>();
		time_limit = obj.at(TIME_LIMIT).get<uint64_t>();

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
	return "{" + quot(NAME) + ":{" + quot(TAG) + ":" + quot(Common::JSON::Screen(tag)) + "," +
		quot(NAME_) + ":" + quot(Common::JSON::Screen(name)) + "," +
		quot(TIME_LIMIT) + ":" + std::to_string(time_limit) + "}}";
}

}
}
