/**
 * CmdRendererDisconnect.cpp - Contains protocol command RENDERER_DISCONNECT impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdRendererDisconnect.h>

#include <Common/Common.h>
#include <Common/Quoter.h>

namespace Proto
{
namespace RENDERER_DISCONNECT
{

static const std::string DEVICE_ID = "device_id";
static const std::string SSRC = "ssrc";

Command::Command()
	: device_id(0),
	ssrc(0)
{
}

Command::Command(uint32_t device_id_, uint32_t ssrc_)
	: device_id(device_id_), ssrc(ssrc_)
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

		device_id = pt.get<uint32_t>(NAME + "." + DEVICE_ID);
		ssrc = pt.get<uint32_t>(NAME + "." + SSRC);

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
	return "{" + quot(NAME) + ":{" + quot(DEVICE_ID) + ":" + std::to_string(device_id) + "," +
		quot(SSRC) + ":" + std::to_string(ssrc) + "}}";
}

}
}
