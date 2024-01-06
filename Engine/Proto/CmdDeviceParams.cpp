/**
 * CmdDeviceParams.cpp - Contains protocol command DEVICE_PARAMS impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018, 2023
 */

#include <Proto/CmdDeviceParams.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

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

Command::Command(uint32_t id_, uint32_t ssrc_, DeviceType device_type_, uint32_t ord_, std::string_view name_, std::string_view metadata_, uint32_t resolution_, Video::ColorSpace color_space_)
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
	try
	{
		auto j = nlohmann::json::parse(message);
		auto obj = j.get<nlohmann::json::object_t>().at(NAME);
		
		id = obj.at(ID).get<uint32_t>();
		ssrc = obj.at(SSRC).get<uint32_t>();
		device_type = static_cast<DeviceType>(obj.at(DEVICE_TYPE).get<uint32_t>());
		ord = obj.at(ORD).get<uint32_t>();
		name = obj.at(NAME_).get<std::string>();
		metadata = obj.at(METADATA).get<std::string>();
		resolution = obj.at(RESOLUTION).get<uint32_t>();
		color_space = static_cast<Video::ColorSpace>(obj.at(COLOR_SPACE).get<uint32_t>());
		
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
