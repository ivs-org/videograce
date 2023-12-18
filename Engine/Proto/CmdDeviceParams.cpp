/**
 * CmdDeviceParams.cpp - Contains protocol command DEVICE_PARAMS impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdDeviceParams.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

namespace Proto
{
namespace DEVICE_PARAMS
{

static const std::string ID = "id";
static const std::string SSRC = "ssrc";
static const std::string DEVICE_TYPE = "device_type";
static const std::string ORD = "ord";
static const std::string NAME_ = "name";
static const std::string METADATA = "metadata";
static const std::string RESOLUTION = "resolution";
static const std::string COLOR_SPACE = "color_space";

Command::Command()
	: id(0),
	ssrc(0),
	device_type(DeviceType::Undefined),
	ord(0),
	name(),
	metadata(),
	resolution(0),
	color_space(Video::ColorSpace::I420)
{
}

Command::Command(uint32_t id_, uint32_t ssrc_, DeviceType device_type_, uint32_t ord_, const std::string &name_, const std::string &metadata_, uint32_t resolution_, Video::ColorSpace color_space_)
	: id(id_),
	ssrc(ssrc_),
	device_type(device_type_),
	ord(ord_),
	name(name_),
	metadata(metadata_),
	resolution(resolution_),
	color_space(color_space_)
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
		
		id = pt.get<uint32_t>(NAME + "." + ID);
		ssrc = pt.get<uint32_t>(NAME + "." + SSRC);
		device_type = static_cast<DeviceType>(pt.get<uint32_t>(NAME + "." + DEVICE_TYPE));
		ord = pt.get<uint32_t>(NAME + "." + ORD);
		name = pt.get<std::string>(NAME + "." + NAME_);
		metadata = pt.get<std::string>(NAME + "." + METADATA);
		resolution = pt.get<uint32_t>(NAME + "." + RESOLUTION);
		color_space = static_cast<Video::ColorSpace>(pt.get<uint32_t>(NAME + "." + COLOR_SPACE));
		
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
		quot(SSRC) + ":" + std::to_string(ssrc) + "," +
		quot(DEVICE_TYPE) + ":" + std::to_string(static_cast<int32_t>(device_type)) + "," +
		quot(ORD) + ":" + std::to_string(ord) + "," +
		quot(NAME_) + ":" + quot(Common::JSON::Screen(name)) + "," +
		quot(METADATA) + ":" + quot(Common::JSON::Screen(metadata)) + "," +
		quot(RESOLUTION) + ":" + std::to_string(resolution) + "," +
		quot(COLOR_SPACE) + ":" + std::to_string(static_cast<int32_t>(color_space)) + "}}";
}

}
}
