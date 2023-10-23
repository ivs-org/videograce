/**
 * CmdDeviceDisconnect.cpp - Contains protocol command DEVICE_DISCONNECT impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdDeviceDisconnect.h>

#include <Common/Common.h>
#include <Common/Quoter.h>

namespace Proto
{
namespace DEVICE_DISCONNECT
{

static const std::string TYPE = "type";
static const std::string DEVICE_ID = "device_id";
static const std::string CLIENT_ID = "client_id";

Command::Command()
	: device_type(DeviceType::Undefined),
	device_id(0),
	client_id(0)
{
}

Command::Command(DeviceType device_type_, uint32_t device_id_, int64_t client_id_)
	: device_type(device_type_), device_id(device_id_), client_id(client_id_)
{
}

Command::~Command()
{
}

bool Command::Parse(const std::string &message)
{
	using boost::property_tree::ptree;

	std::stringstream ss;
	ss << message;

	try
	{
		ptree pt;
		read_json(ss, pt);

		device_type = static_cast<DeviceType>(pt.get<uint32_t>(NAME + "." + TYPE));
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
	return "{" + quot(NAME) + ":{" + quot(TYPE) + ":" + std::to_string(static_cast<int32_t>(device_type)) + "," +
		quot(DEVICE_ID) + ":" + std::to_string(device_id) + "," +
		quot(CLIENT_ID) + ":" + std::to_string(client_id) + "}}";
}

}
}
