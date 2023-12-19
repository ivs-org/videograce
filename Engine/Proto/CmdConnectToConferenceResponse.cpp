/**
 * CmdConnectToConferenceResponse.cpp - Contains protocol command CONNECT_TO_CONFERENCE_RESPONSE impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdConnectToConferenceResponse.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

namespace Proto
{
namespace CONNECT_TO_CONFERENCE_RESPONSE
{

static const std::string RESULT = "result";
static const std::string ID = "id";
static const std::string GRANTS = "grants";
static const std::string FOUNDER_ID = "founder_id";
static const std::string TAG = "tag";
static const std::string NAME_ = "name";
static const std::string TEMP = "temp";

Command::Command()
	: result(Result::Undefined),
	id(0),
	grants(0),
	founder_id(0),
	tag(), name(),
	temp(false)
{
}

Command::Command(Result result_, int64_t id_, uint32_t grants_, int64_t founder_id_, std::string_view tag_, std::string_view name_, bool temp_)
	: result(result_), id(id_), grants(grants_), founder_id(founder_id_), tag(tag_), name(name_), temp(temp_)
{
}

Command::Command(Result result_)
	: result(result_), id(0), grants(0), founder_id(0), tag(), name(), temp(false)
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

		result = static_cast<Result>(pt.get<uint32_t>(NAME + "." + RESULT));

		auto grants_opt = pt.get_optional<uint32_t>(NAME + "." + GRANTS);
		if (grants_opt) grants = grants_opt.get();

		auto id_opt = pt.get_optional<int64_t>(NAME + "." + ID);
		if (id_opt) id = id_opt.get();

		auto founder_id_opt = pt.get_optional<int64_t>(NAME + "." + FOUNDER_ID);
		if (founder_id_opt) founder_id = founder_id_opt.get();

		auto tag_opt = pt.get_optional<std::string>(NAME + "." + TAG);
		if (tag_opt) tag = tag_opt.get();

		auto name_opt = pt.get_optional<std::string>(NAME + "." + NAME_);
		if (name_opt) name = name_opt.get();

		auto temp_opt = pt.get_optional<uint32_t>(NAME + "." + TEMP);
		if (temp_opt) temp = temp_opt.get() != 0;
		
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
		quot(RESULT) + ":" + std::to_string(static_cast<int32_t>(result)) + 
		(grants != 0 ? "," + quot(GRANTS) + ":" + std::to_string(grants) : "") +
		(id != 0 ? "," + quot(ID) + ":" + std::to_string(id) : "") +
		(founder_id != 0 ? "," + quot(FOUNDER_ID) + ":" + std::to_string(founder_id) : "") +
		(!tag.empty() ? "," + quot(TAG) + ":" + quot(Common::JSON::Screen(tag)) : "") +
		(!name.empty() ? "," + quot(NAME_) + ":" + quot(Common::JSON::Screen(name)) : "") + 
		(temp ? "," + quot(TEMP) + ":1" : "") +	"}}";
}

}
}
