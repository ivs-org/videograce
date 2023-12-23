/**
 * Result.cpp - Contains API result json impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020, 2023
 */

#include <API/Result.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace API
{
namespace RESULT
{

static const std::string CODE = "code";
static const std::string MESSAGE = "message";

Command::Command()
	: code(0), message(), opt()
{
}

Command::Command(uint32_t code_, std::string_view message_, std::string_view opt_)
	: code(code_), message(message_), opt(opt_)
{
}

Command::~Command()
{
}

bool Command::Parse(std::string_view message_)
{
	try
	{
		spdlog::get("System")->trace("api::redirect_user :: perform parsing");

		auto j = nlohmann::json::parse(message_);

		if (j.count(CODE) != 0) code = j.at(CODE).get<std::string>();
		if (j.count(MESSAGE) != 0) message = j.at(MESSAGE).get<std::string>();

		return true;
	}
	catch (nlohmann::json::parse_error& ex)
	{
		spdlog::get("Error")->critical("api::redirect_user :: error parse json (byte: {0}, what: {1})", ex.byte, ex.what());
	}
	return false;
}

std::string Command::Serialize()
{
	return "{" + 
		(code != 0 ? quot(CODE) + ":" + std::to_string(code) : "") +
		(!message.empty() ? (code != 0 ? "," : "") + quot(MESSAGE) + ":" + quot(Common::JSON::Screen(message)) : "") +
		(!opt.empty() ? (code != 0 || !message.empty() ? "," : "") + opt : "") + "}";
}

}
}
