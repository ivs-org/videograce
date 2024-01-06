/**
 * DeleteGroup.cpp - Contains API delete group json impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#include <API/DeleteGroup.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace API
{
namespace DELETE_GROUP
{

static const std::string ID = "id";

Command::Command()
	: id(-1)
{
}

Command::Command(int64_t id_)
	: id(id_)
{
}

Command::~Command()
{
}

bool Command::Parse(std::string_view message_)
{
	try
	{
		auto j = nlohmann::json::parse(message_);

		if (j.count(ID) != 0) id = j.at(ID).get<int64_t>();

		return true;
	}
	catch (nlohmann::json::parse_error& ex)
	{
		spdlog::get("Error")->critical("api::delete_group :: error parse json (byte: {0}, what: {1})", ex.byte, ex.what());
	}

	return false;
}

std::string Command::Serialize()
{
	return "{" + 
		quot(ID) + ":" + std::to_string(id) + 
	"}";
}

}
}
