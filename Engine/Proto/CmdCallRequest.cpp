/**
 * CmdCallRequest.cpp - Contains protocol command CALL_REQUEST impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#include <Proto/CmdCallRequest.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace CALL_REQUEST
{

static const std::string NAME_ = "name";
static const std::string ID = "id";
static const std::string CONNECTION_ID = "connection_id";
static const std::string TYPE = "type";
static const std::string TIME_LIMIT = "time_limit";

Command::Command()
    : name(),
    id(0),
    connection_id(0),
    type(Type::Undefined),
    time_limit()
{
}

Command::Command(std::string_view name_, int64_t id_, uint32_t connection_id_, Type type_, uint64_t time_limit_)
    : name(name_), id(id_), connection_id(connection_id_), type(type_), time_limit(time_limit_)
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

        name = obj.at(NAME_).get<std::string>();
        id = obj.at(ID).get<int64_t>();
        connection_id = obj.at(CONNECTION_ID).get<uint32_t>();
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
	return "{" + quot(NAME) + ":{" + quot(NAME_) + ":" + quot(Common::JSON::Screen(name)) + "," +
		quot(ID) + ":" + std::to_string(id) + "," +
		quot(CONNECTION_ID) + ":" + std::to_string(connection_id) + "," +
		quot(TYPE) + ":" + std::to_string(static_cast<int32_t>(type)) + "," +
		quot(TIME_LIMIT) + ":" + std::to_string(time_limit) + "}}";
}

std::string str(Type t)
{
	static const std::map<Type, std::string> names = { 
        { Type::Undefined, "Undefined" },
        { Type::Invocation, "Invocation" },
        { Type::Cancel, "Cancel" }
    };

    auto n = names.find(t);
    if (n != names.end())
    {
        return n->second;
    }

    return "";
}

}
}
