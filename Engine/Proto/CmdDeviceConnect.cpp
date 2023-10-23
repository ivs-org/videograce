/**
 * CmdDeviceConnect.cpp - Contains protocol command DEVICE_CONNECT impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdDeviceConnect.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

namespace Proto
{
namespace DEVICE_CONNECT
{

static const std::string CONNECT_TYPE = "connect_type";
static const std::string DEVICE_TYPE = "device_type";
static const std::string DEVICE_ID = "device_id";
static const std::string CLIENT_ID = "client_id";
static const std::string METADATA = "metadata";
static const std::string RECEIVER_SSRC = "receiver_ssrc";
static const std::string AUTHOR_SSRC = "author_ssrc";
static const std::string ADDRESS = "address";
static const std::string PORT = "port";
static const std::string NAME_ = "name";
static const std::string RESOLUTION = "resolution";
static const std::string COLOR_SPACE = "color_space";
static const std::string MY = "my";
static const std::string SECURE_KEY = "secure_key";

Command::Command()
	: connect_type(ConnectType::Undefined),
	device_type(DeviceType::Undefined),
	device_id(0),
	client_id(0),
	metadata(),
	receiver_ssrc(0),
	author_ssrc(0),
	address(),
	port(0),
	name(),
	resolution(0),
	color_space(Video::ColorSpace::I420),
	my(false),
	secure_key()
{
}

Command::Command(ConnectType connect_type_, DeviceType device_type_, uint32_t device_id_, int64_t client_id_, const std::string &metadata_, uint32_t receiver_ssrc_, uint32_t author_ssrc_, const std::string &address_, uint32_t port_, const std::string &name_, uint32_t resolution_, Video::ColorSpace color_space_, bool my_, const std::string &secure_key_)
	: connect_type(connect_type_), device_type(device_type_), device_id(device_id_), client_id(client_id_), metadata(metadata_), receiver_ssrc(receiver_ssrc_), author_ssrc(author_ssrc_), address(address_), port(port_), name(name_), resolution(resolution_), color_space(color_space_), my(my_), secure_key(secure_key_)
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

		connect_type = static_cast<ConnectType>(pt.get<uint32_t>(NAME + "." + CONNECT_TYPE));
		device_type = static_cast<DeviceType>(pt.get<uint32_t>(NAME + "." + DEVICE_TYPE));
		device_id = pt.get<uint32_t>(NAME + "." + DEVICE_ID);
		client_id = pt.get<int64_t>(NAME + "." + CLIENT_ID);
		metadata = pt.get<std::string>(NAME + "." + METADATA);
		receiver_ssrc = pt.get<uint32_t>(NAME + "." + RECEIVER_SSRC);
		author_ssrc = pt.get<uint32_t>(NAME + "." + AUTHOR_SSRC);
		address = pt.get<std::string>(NAME + "." + ADDRESS);
		port = pt.get<uint16_t>(NAME + "." + PORT);
		name = pt.get<std::string>(NAME + "." + NAME_);
		resolution = pt.get<uint32_t>(NAME + "." + RESOLUTION);
		color_space = static_cast<Video::ColorSpace>(pt.get<int32_t>(NAME + "." + COLOR_SPACE));
		my = pt.get<uint32_t>(NAME + "." + MY) != 0;
		secure_key = pt.get<std::string>(NAME + "." + SECURE_KEY);
		
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
	return "{" + quot(NAME) + ":{" + quot(CONNECT_TYPE) + ":" + std::to_string(static_cast<int32_t>(connect_type)) + "," +
		quot(DEVICE_TYPE) + ":" + std::to_string(static_cast<int32_t>(device_type)) + "," +
		quot(DEVICE_ID) + ":" + std::to_string(device_id) + "," +
		quot(CLIENT_ID) + ":" + std::to_string(client_id) + "," +
		quot(METADATA) + ":" + quot(Common::JSON::Screen(metadata)) + "," +
		quot(RECEIVER_SSRC) + ":" + std::to_string(receiver_ssrc) + "," +
		quot(AUTHOR_SSRC) + ":" + std::to_string(author_ssrc) + "," +
		quot(ADDRESS) + ":" + quot(Common::JSON::Screen(address)) + "," +
		quot(PORT) + ":" + std::to_string(port) + "," +
		quot(NAME_) + ":" + quot(Common::JSON::Screen(name)) + "," +
		quot(RESOLUTION) + ":" + std::to_string(resolution) + "," +
		quot(COLOR_SPACE) + ":" + std::to_string(static_cast<int32_t>(color_space)) + "," +
		quot(MY) + ":" + (my ? "1" : "0") + "," +
		quot(SECURE_KEY) + ":" + quot(Common::JSON::Screen(secure_key)) + "}}";
}

}
}
