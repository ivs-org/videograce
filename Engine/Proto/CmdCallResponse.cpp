/**
 * CmdCallResponse.cpp - Contains protocol command CALL_RESPONSE impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#include <Proto/CmdCallResponse.h>

#include <Common/Quoter.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace CALL_RESPONSE
{

static const std::string ID = "id";
static const std::string CONNECTION_ID = "connection_id";
static const std::string NAME_ = "name";
static const std::string TYPE = "type";
static const std::string TIME_LIMIT = "time_limit";

Command::Command()
	: id(0),
	connection_id(0),
	name(),
	type(Type::Undefined),
	time_limit()
{
}

Command::Command(int64_t id_, uint32_t connection_id_, std::string_view name_, Type type_, uint64_t time_limit_)
	: id(id_), connection_id(connection_id_), name(name_), type(type_), time_limit(time_limit_)
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

		id = obj.at(ID).get<int64_t>();
		connection_id = obj.at(CONNECTION_ID).get<uint32_t>();
		name = obj.at(NAME_).get<std::string>();
		type = static_cast<Type>(obj.at(TYPE).get<uint32_t>());
		time_limit = obj.at(TIME_LIMIT).get<int64_t>();

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
	return "{" + quot(NAME) + ":{" + quot(ID) + ":" + std::to_string(id) + "," +
		quot(CONNECTION_ID) + ":" + std::to_string(connection_id) + "," +
		quot(NAME_) + ":" + quot(name) + "," +
		quot(TYPE) + ":" + std::to_string(static_cast<int32_t>(type)) + "," +
		quot(TIME_LIMIT) + ":" + std::to_string(time_limit) + "}}";
}

}
}
