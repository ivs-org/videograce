/**
 * CmdConferencesList.cpp - Contains protocol command CONFERENCES_LIST impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018, 2023
 */

#include <Proto/CmdConferencesList.h>

#include <Common/Quoter.h>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace CONFERENCES_LIST
{

Command::Command()
	: conferences()
{
}

Command::Command(const std::vector<Conference> &conferences_)
	: conferences(conferences_)
{
}

Command::~Command()
{
}

bool Command::Parse(std::string_view message)
{
	try
	{
		auto j = nlohmann::json::parse(message);
		auto confs = j.get<nlohmann::json::object_t>().at(NAME);

		for (auto &c : confs)
		{
			Conference conference;
			if (conference.Parse(c))
			{
				conferences.emplace_back(conference);
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

	for (auto &c : conferences)
	{
		out += c.Serialize() + ",";
	}
	if (!conferences.empty())
	{
		out.pop_back(); // drop last ","
	}

	return out + "]}";
}

}
}
