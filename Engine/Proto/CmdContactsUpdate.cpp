/**
 * CmdContactsUpdate.cpp - Contains protocol command CONTACTS_UPDATE impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdContactsUpdate.h>

#include <Common/Common.h>
#include <Common/JSONSymbolsScreener.h>
#include <Common/Quoter.h>

namespace Proto
{
namespace CONTACTS_UPDATE
{

static const std::string ACTION = "action";
static const std::string CLIENT_ID = "client_id";

Command::Command()
	: action(Action::Undefined), client_id(-1)
{
}

Command::Command(Action action_,
    int64_t client_id_)
	: action(action_),
    client_id(client_id_)
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
	return "{" + quot(NAME) + ":{" + 
		quot(ACTION) + ":" + std::to_string(static_cast<uint32_t>(action)) + "," +
        quot(CLIENT_ID) + ":" + std::to_string(client_id) + "}}";
}

}
}
