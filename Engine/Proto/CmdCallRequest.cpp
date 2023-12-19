/**
 * CmdCallRequest.cpp - Contains protocol command CALL_REQUEST impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdCallRequest.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <map>

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
	using boost::property_tree::ptree;

	std::stringstream ss;
	ss << message;

	try
	{
		ptree pt;
		read_json(ss, pt);
		
		name = pt.get<std::string>(NAME + "." + NAME_);
		id = pt.get<int64_t>(NAME + "." + ID);
		connection_id = pt.get<uint32_t>(NAME + "." + CONNECTION_ID);
		type = static_cast<Type>(pt.get<uint32_t>(NAME + "." + TYPE));
		time_limit = pt.get<uint64_t>(NAME + "." + TIME_LIMIT);

		return true;
	}
	catch (std::exception const& e)
	{
		DBGTRACE("Error parsing %s, %s\n", NAME.c_str(), e.what());
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
