/**
 * CmdDeliveryBlobs.cpp - Contains protocol command DELIVERY_BLOBS impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020, 2023
 */

#include <Proto/CmdDeliveryBlobs.h>

#include <Common/JSONSymbolsScreener.h>
#include <Common/Quoter.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace DELIVERY_BLOBS
{

Command::Command()
	: blobs()
{
}

Command::Command(const std::vector<Blob> &blobs_)
	: blobs(blobs_)
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
		auto bs = j.get<nlohmann::json::object_t>().at(NAME);
		for (auto& d : bs)
		{
			Blob blob;
			if (blob.Parse(d))
			{
				blobs.emplace_back(blob);
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

	for (auto& b : blobs)
	{
		out += b.Serialize() + ",";
	}
	if (!blobs.empty())
	{
		out.pop_back(); // drop last ","
	}
	return out + "]}";
}

}
}
