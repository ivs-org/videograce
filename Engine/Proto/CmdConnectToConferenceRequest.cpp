/**
 * CmdConnectToConferenceRequest.cpp - Contains protocol command CONNECT_TO_CONFERENCE_REQUEST impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdConnectToConferenceRequest.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

namespace Proto
{
namespace CONNECT_TO_CONFERENCE_REQUEST
{

static const std::string TAG = "tag";
static const std::string CONNECT_MEMBERS = "connect_members";
static const std::string HAS_CAMERA = "has_camera";
static const std::string HAS_MICROPHONE = "has_microphone";
static const std::string HAS_DEMONSTRATION = "has_demonstration";

Command::Command()
	: tag(), connect_members(false), has_camera(false), has_microphone(false), has_demonstration(false)
{
}

Command::Command(const std::string &tag_)
	: tag(tag_), connect_members(false), has_camera(false), has_microphone(false), has_demonstration(false)
{
}

Command::Command(const std::string &tag_, bool connect_members_, bool has_camera_, bool has_microphone_, bool has_demonstration_)
	: tag(tag_), connect_members(connect_members_), has_camera(has_camera_), has_microphone(has_microphone_), has_demonstration(has_demonstration_)
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

		tag = pt.get<std::string>(NAME + "." + TAG);

		auto connect_members_opt = pt.get_optional<uint8_t>(NAME + "." + CONNECT_MEMBERS);
		if (connect_members_opt) connect_members = connect_members_opt.get() != 0;

		auto has_camera_opt = pt.get_optional<uint8_t>(NAME + "." + HAS_CAMERA);
		if (has_camera_opt) has_camera = has_camera_opt.get() != 0;

		auto has_microphone_opt = pt.get_optional<uint8_t>(NAME + "." + HAS_MICROPHONE);
		if (has_microphone_opt) has_microphone = has_microphone_opt.get() != 0;

        auto has_demonstration_opt = pt.get_optional<uint8_t>(NAME + "." + HAS_DEMONSTRATION);
        if (has_demonstration_opt) has_demonstration = has_demonstration_opt.get() != 0;
				
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
	return "{" + quot(NAME) + ":{" + 
		quot(TAG) + ":" + quot(Common::JSON::Screen(tag)) +
		(connect_members ? "," + quot(CONNECT_MEMBERS) + ":1" : "") +
		(has_camera ? "," + quot(HAS_CAMERA) + ":1" : "") +
		(has_microphone ? "," + quot(HAS_MICROPHONE) + ":1" : "") +
        (has_demonstration ? "," + quot(HAS_DEMONSTRATION) + ":1" : "") + "}}";
}

}
}
