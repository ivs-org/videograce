/**
 * CmdMicrophoneActive.cpp - Contains protocol command MICROPHONE_ACTIVE impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdMicrophoneActive.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

namespace Proto
{
namespace MICROPHONE_ACTIVE
{

static const std::string ACTIVE_TYPE = "active_type";
static const std::string DEVICE_ID = "device_id";
static const std::string CLIENT_ID = "client_id";

Command::Command()
	: active_type(ActiveType::Undefined),
	device_id(0),
	client_id(0)
{
}

Command::Command(ActiveType active_type_, uint32_t device_id_, int64_t client_id_)
	: active_type(active_type_), device_id(device_id_), client_id(client_id_)
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

		active_type = static_cast<ActiveType>(pt.get<uint32_t>(NAME + "." + ACTIVE_TYPE));
		device_id = pt.get<uint32_t>(NAME + "." + DEVICE_ID);
		client_id = pt.get<int64_t>(NAME + "." + CLIENT_ID);
		
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
	return "{" + quot(NAME) + ":{" + quot(ACTIVE_TYPE) + ":" + std::to_string(static_cast<int32_t>(active_type)) + "," +
		quot(DEVICE_ID) + ":" + std::to_string(device_id) + "," +
		quot(CLIENT_ID) + ":" + std::to_string(client_id) + "}}";
}

}
}
