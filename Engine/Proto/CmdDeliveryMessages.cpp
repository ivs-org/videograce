/**
 * CmdDeliveryMessages.cpp - Contains protocol command DELIVERY_MESSAGES impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2019, 2023
 */

#include <Proto/CmdDeliveryMessages.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace DELIVERY_MESSAGES
{

Command::Command()
	: messages()
{
}

Command::Command(const std::vector<Message> &messages_)
	: messages(messages_)
{
}

Command::Command(const Message &message)
	: messages()
{
	messages.emplace_back(message);
}

Command::~Command()
{
}

bool Command::Parse(std::string_view message)
{
	try
	{
		auto j = nlohmann::json::parse(message);
		auto deliveries = j.get<nlohmann::json::object_t>().at(NAME);
		for (auto &d : deliveries)
		{
			Message message;
			if (message.Parse(d))
			{
				messages.emplace_back(message);
			}
		}

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
	std::string out = "{" + quot(NAME) + ":[";

	for (auto &m : messages)
	{
		out += m.Serialize() + ",";
	}
	if (!messages.empty())
	{
		out.pop_back(); // drop last ","
	}
	return out + "]}";
}

}
}
