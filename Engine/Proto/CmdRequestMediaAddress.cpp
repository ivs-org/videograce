/**
 * CmdRequestMediaAddresses.cpp - Contains protocol command REQUEST_MEDIA_ADDRESSES impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018, 2023
 */

#include <Proto/CmdRequestMediaAddresses.h>

#include <Common/Quoter.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace REQUEST_MEDIA_ADDRESSES
{

Command::Command()
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

		return j.count(NAME) != 0;
	}
	catch (nlohmann::json::parse_error& ex)
	{
		spdlog::get("Error")->critical("proto::{0} :: error parse json (byte: {1}, what: {2})", NAME, ex.byte, ex.what());
	}
	return false;
}

std::string Command::Serialize()
{
	return "{" + quot(NAME) + "}";
}

}
}
