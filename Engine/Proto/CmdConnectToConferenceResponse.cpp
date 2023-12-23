/**
 * CmdConnectToConferenceResponse.cpp - Contains protocol command CONNECT_TO_CONFERENCE_RESPONSE impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <Proto/CmdConnectToConferenceResponse.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace CONNECT_TO_CONFERENCE_RESPONSE
{

static const std::string RESULT = "result";
static const std::string ID = "id";
static const std::string GRANTS = "grants";
static const std::string FOUNDER_ID = "founder_id";
static const std::string TAG = "tag";
static const std::string NAME_ = "name";
static const std::string TEMP = "temp";

Command::Command()
	: result(Result::Undefined),
	id(0),
	grants(0),
	founder_id(0),
	tag(), name(),
	temp(false)
{
}

Command::Command(Result result_, int64_t id_, uint32_t grants_, int64_t founder_id_, std::string_view tag_, std::string_view name_, bool temp_)
	: result(result_), id(id_), grants(grants_), founder_id(founder_id_), tag(tag_), name(name_), temp(temp_)
{
}

Command::Command(Result result_)
	: result(result_), id(0), grants(0), founder_id(0), tag(), name(), temp(false)
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

		result = static_cast<Result>(obj.at(RESULT).get<uint32_t>());

		if (obj.count(GRANTS) != 0) grants = obj.at(GRANTS).get<uint32_t>();
		if (obj.count(ID) != 0) id = obj.at(ID).get<int64_t>();
		if (obj.count(FOUNDER_ID) != 0) founder_id = obj.at(FOUNDER_ID).get<int64_t>();
		if (obj.count(TAG) != 0) tag = obj.at(TAG).get<std::string>();
		if (obj.count(NAME_) != 0) name = obj.at(NAME_).get<std::string>();
		if (obj.count(TEMP) != 0) temp = obj.at(TEMP).get<int8_t>() != 0;

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
		quot(RESULT) + ":" + std::to_string(static_cast<int32_t>(result)) + 
		(grants != 0 ? "," + quot(GRANTS) + ":" + std::to_string(grants) : "") +
		(id != 0 ? "," + quot(ID) + ":" + std::to_string(id) : "") +
		(founder_id != 0 ? "," + quot(FOUNDER_ID) + ":" + std::to_string(founder_id) : "") +
		(!tag.empty() ? "," + quot(TAG) + ":" + quot(Common::JSON::Screen(tag)) : "") +
		(!name.empty() ? "," + quot(NAME_) + ":" + quot(Common::JSON::Screen(name)) : "") + 
		(temp ? "," + quot(TEMP) + ":1" : "") +	"}}";
}

}
}
