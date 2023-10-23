/**
 * CmdDeliveryMessages.cpp - Contains protocol command DELIVERY_MESSAGES impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2019
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/CmdDeliveryMessages.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

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

bool Command::Parse(const std::string &message)
{
	using boost::property_tree::ptree;

	std::stringstream ss;
	ss << message;

	try
	{
		ptree pt;
		read_json(ss, pt);

		auto &deliveries = pt.get_child(NAME);
		for (auto &d : deliveries)
		{
			Message message;
			if (message.Parse(d.second))
			{
				messages.emplace_back(message);
			}
		}

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
