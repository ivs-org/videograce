/**
 * CmdUserUpdateResponse.cpp - Contains protocol command USER_UPDATE_RESPONSE impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021, 2023
 */

#include <Proto/CmdUserUpdateResponse.h>

#include <Common/JSONSymbolsScreener.h>
#include <Common/Quoter.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

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
	try
	{
		spdlog::get("System")->trace("proto::{0} :: perform parsing", NAME);

		auto j = nlohmann::json::parse(message);
		auto obj = j.get<nlohmann::json::object_t>().at(NAME);

		action = static_cast<Proto::USER_UPDATE_REQUEST::Action>(obj.at(ACTION).get<uint32_t>());
		result = static_cast<Result>(obj.at(RESULT).get<uint32_t>());

		if (obj.count(USER_ID) != 0) user_id = obj.at(USER_ID).get<int64_t>();
		if (obj.count(MESSAGE) != 0) message = obj.at(MESSAGE).get<std::string>();

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
	return "{" + quot(NAME) + ":{" + 
		quot(ACTION) + ":" + std::to_string(static_cast<int32_t>(action)) + "," +
		quot(RESULT) + ":" + std::to_string(static_cast<int32_t>(result)) +
		(user_id != -1 ? "," + quot(USER_ID) + ":" + std::to_string(user_id) : "") +
		(!message.empty() ? "," + quot(MESSAGE) + ":" + quot(Common::JSON::Screen(message)) : "") + "}}";
}

}
}
