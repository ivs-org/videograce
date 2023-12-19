/**
 * CmdUserUpdateResponse.cpp - Contains protocol command USER_UPDATE_RESPONSE impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdUserUpdateResponse.h>

#include <Common/Common.h>
#include <Common/JSONSymbolsScreener.h>
#include <Common/Quoter.h>

namespace Proto
{
namespace USER_UPDATE_RESPONSE
{

static const std::string ACTION = "action";
static const std::string RESULT = "result";
static const std::string USER_ID = "user_id";
static const std::string MESSAGE = "message";

Command::Command()
	: action(Proto::USER_UPDATE_REQUEST::Action::Undefined), result(Result::Undefined), user_id(-1), message()
{
}

Command::Command(Proto::USER_UPDATE_REQUEST::Action action_, Result result_, int64_t user_id_, std::string_view message_)
	: action(action_), result(result_), user_id(user_id_), message(message_)
{
}

Command::~Command()
{
}

bool Command::Parse(std::string_view msg)
{
	using boost::property_tree::ptree;

	std::stringstream ss;
	ss << msg;

	try
	{
		ptree pt;
		read_json(ss, pt);

		action = static_cast<Proto::USER_UPDATE_REQUEST::Action>(pt.get<uint32_t>(NAME + "." + ACTION));
		result = static_cast<Result>(pt.get<uint32_t>(NAME + "." + RESULT));

		auto user_id_opt = pt.get_optional<int64_t>(NAME + "." + USER_ID);
		if (user_id_opt) user_id = user_id_opt.get();

		auto message_opt = pt.get_optional<std::string>(NAME + "." + MESSAGE);
		if (message_opt) message = message_opt.get();
		
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
		quot(ACTION) + ":" + std::to_string(static_cast<int32_t>(action)) + "," +
		quot(RESULT) + ":" + std::to_string(static_cast<int32_t>(result)) +
		(user_id != -1 ? "," + quot(USER_ID) + ":" + std::to_string(user_id) : "") +
		(!message.empty() ? "," + quot(MESSAGE) + ":" + quot(Common::JSON::Screen(message)) : "") + "}}";
}

}
}
