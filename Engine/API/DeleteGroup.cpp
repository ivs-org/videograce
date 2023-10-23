/**
 * DeleteGroup.cpp - Contains API delete group json impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <API/DeleteGroup.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

namespace API
{
namespace DELETE_GROUP
{

static const std::string ID = "id";

Command::Command()
	: id(-1)
{
}

Command::Command(int64_t id_)
	: id(id_)
{
}

Command::~Command()
{
}

bool Command::Parse(const std::string &message_)
{
	using boost::property_tree::ptree;

	std::stringstream ss;
	ss << message_;

	try
	{
		ptree pt;
		read_json(ss, pt);

		auto id_opt = pt.get_optional<int64_t>(ID);
		if (id_opt) id = id_opt.get();
						
		return true;
	}
	catch (std::exception const& e)
	{
		DBGTRACE("Error parsing DeleteGroup %s\n", e.what());
	}
	return false;
}

std::string Command::Serialize()
{
	return "{" + 
		quot(ID) + ":" + std::to_string(id) + 
	"}";
}

}
}
