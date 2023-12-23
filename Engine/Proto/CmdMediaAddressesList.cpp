/**
 * CmdMediaAddressesList.cpp - Contains protocol command RTP_ADDRESSES_LIST impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018, 2023
 */

#include <Proto/CmdMediaAddressesList.h>

#include <Common/Quoter.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace MEDIA_ADDRESSES_LIST
{

Command::Command()
	: addresses()
{
}

Command::Command(const std::vector<std::string> &addresses_)
	: addresses(addresses_)
{

}

Command::~Command()
{
}

bool Command::Parse(std::string_view message)
{
	try
	{
		spdlog::get("System")->trace("proto::{0} :: perform parsing", NAME);

		auto j = nlohmann::json::parse(message);
		auto addresses_ = j.get<nlohmann::json::object_t>().at(NAME);
		for (auto &a : addresses_)
		{
			addresses.emplace_back(a.get<std::string>());
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

	for (auto &a : addresses)
	{
		out += "\"" + a + "\",";
	}
	if (!addresses.empty())
	{
		out.pop_back(); // drop last ","
	}
	return out + "]}";
}

}
}
