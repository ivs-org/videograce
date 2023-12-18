/**
 * CmdCallResponse.cpp - Contains protocol command CALL_RESPONSE impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdCallResponse.h>

#include <Common/Common.h>
#include <Common/Quoter.h>

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

Command::Command(int64_t id_, uint32_t connection_id_, const std::string &name_, Type type_, uint64_t time_limit_)
	: id(id_), connection_id(connection_id_), name(name_), type(type_), time_limit(time_limit_)
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

		id = pt.get<int64_t>(NAME + "." + ID);
		connection_id = pt.get<uint32_t>(NAME + "." + CONNECTION_ID);
		name = pt.get<std::string>(NAME + "." + NAME_);
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
	return "{" + quot(NAME) + ":{" + quot(ID) + ":" + std::to_string(id) + "," +
		quot(CONNECTION_ID) + ":" + std::to_string(connection_id) + "," +
		quot(NAME_) + ":" + quot(name) + "," +
		quot(TYPE) + ":" + std::to_string(static_cast<int32_t>(type)) + "," +
		quot(TIME_LIMIT) + ":" + std::to_string(time_limit) + "}}";
}

}
}
