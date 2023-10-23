/**
 * CmdConferenceUpdateRequest.cpp - Contains protocol command CONFERENCE_UPDATE_REQUEST impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdConferenceUpdateRequest.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

namespace Proto
{
namespace CONFERENCE_UPDATE_REQUEST
{

static const std::string ACTION = "action";
static const std::string CONFERENCE = "conference";

Command::Command()
	: action(Action::Undefined),
	conference()
{
}

Command::Command(Action action_, const Conference &conference_)
	: action(action_),
	conference(conference_)
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

		action = static_cast<Action>(pt.get<uint32_t>(NAME + "." + ACTION));

		return conference.Parse(pt.get_child(NAME + "." + CONFERENCE));
	}
	catch (std::exception const& e)
	{
		DBGTRACE("Error parsing %s, %s\n", NAME.c_str(), e.what());
	}
	return false;
}

std::string Command::Serialize()
{
	return "{" + quot(NAME) + ":{" + quot(ACTION) + ":" + std::to_string(static_cast<int32_t>(action)) + "," +
		quot(CONFERENCE) + ":" + conference.Serialize() +
		"}}";
}

}
}
