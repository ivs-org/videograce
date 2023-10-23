/**
 * CmdConferenceUpdateResponse.cpp - Contains protocol command CONFERENCE_UPDATE_RESPONSE impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdConferenceUpdateResponse.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

namespace Proto
{
namespace CONFERENCE_UPDATE_RESPONSE
{

static const std::string ID = "id";
static const std::string RESULT = "result";

Command::Command()
	: id(-1), result(Result::Undefined)
{
}

Command::Command(Result result_)
    : id(-1), result(result_)
{
}

Command::Command(int64_t id_, Result result_)
	: id(id_), result(result_)
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

        auto id_opt = pt.get_optional<int64_t>(NAME + "." + ID);
        if (id_opt) id = id_opt.get();

		result = static_cast<Result>(pt.get<uint32_t>(NAME + "." + RESULT));
		
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
        (id != -1 ? quot(ID) + ":" + std::to_string(id) + "," : "") +
        quot(RESULT) + ":" + std::to_string(static_cast<int32_t>(result)) + "}}";
}

}
}
