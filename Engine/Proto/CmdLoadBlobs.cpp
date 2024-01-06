/**
 * CmdLoadBlobs.cpp - Contains protocol command LOAD_BLOBS impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2023
 */

#include <Proto/CmdLoadBlobs.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace LOAD_BLOBS
{

Command::Command()
	: guids()
{
}

Command::Command(const std::vector<std::string>& guids_)
	: guids(guids_)
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
		auto gs = j.get<nlohmann::json::object_t>().at(NAME);
		for (auto& g : gs)
		{
			guids.emplace_back(g);
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

	for (auto& g : guids)
	{
		out += quot(g) + ",";
	}
	if (!guids.empty())
	{
		out.pop_back(); // drop last ","
	}
	return out + "]}";
}

}
}
