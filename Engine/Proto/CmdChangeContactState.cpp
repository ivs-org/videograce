/**
 * CmdChangeContactState.cpp - Contains protocol command CHANGE_CONTACT_STATE impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdChangeContactState.h>

#include <Common/Common.h>
#include <Common/Quoter.h>

namespace Proto
{
namespace CHANGE_CONTACT_STATE
{

static const std::string ID = "id";
static const std::string STATE = "state";

Command::Command()
	: id(0),
	state(MemberState::Undefined)
{
}

Command::Command(int64_t id_, MemberState state_)
	: id(id_), state(state_)
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

		id = pt.get<int64_t>(NAME + "." + ID);
		state = static_cast<MemberState>(pt.get<uint32_t>(NAME + "." + STATE));

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
		quot(STATE) + ":" + std::to_string(static_cast<int32_t>(state)) + "}}";
}

}
}
