/**
 * Result.cpp - Contains API result json impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <API/Result.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

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

Command::Command(uint32_t code_, const std::string &message_, const std::string &opt_)
	: code(code_), message(message_), opt(opt_)
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

		auto code_opt = pt.get_optional<uint32_t>(CODE);
		if (code_opt) code = code_opt.get();

		auto message_opt = pt.get_optional<std::string>(MESSAGE);
		if (message_opt) message = message_opt.get();
						
		return true;
	}
	catch (std::exception const& e)
	{
		DBGTRACE("Error parsing Result %s\n", e.what());
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
