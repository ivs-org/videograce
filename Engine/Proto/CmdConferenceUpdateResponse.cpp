/**
 * CmdConferenceUpdateResponse.cpp - Contains protocol command CONFERENCE_UPDATE_RESPONSE impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <Proto/CmdConferenceUpdateResponse.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace CONFERENCE_UPDATE_RESPONSE
{

static const std::string ID = "id";
static const std::string RESULT = "result";

Command::Command()
	: id(-1), result(Result::Undefined)
{
}

Command::Command(Result result_)
    : id(-1), result(result_)
{
}

Command::Command(int64_t id_, Result result_)
	: id(id_), result(result_)
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

		if (obj.count(ID) != 0) id = obj.at(ID).get<int64_t>();

		result = static_cast<Result>(obj.at(RESULT).get<uint32_t>());
		
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
        (id != -1 ? quot(ID) + ":" + std::to_string(id) + "," : "") +
        quot(RESULT) + ":" + std::to_string(static_cast<int32_t>(result)) + "}}";
}

}
}
