/**
 * CreateGroup.cpp - Contains API create group json impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021, 2023
 */

#include <API/CreateGroup.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace API
{
namespace CREATE_GROUP
{

static const std::string PARENT_ID = "parent_id";
static const std::string NAME = "name";
static const std::string TAG = "tag";
static const std::string PASSWORD = "password";
static const std::string LIMITED = "limited";
static const std::string GUID = "guid";
static const std::string OWNER_ID = "owner_id";

Command::Command()
	: parent_id(0), name(), tag(), password(), limited(false), guid(), owner_id(0)
{
}

Command::Command(int64_t parent_id_, std::string_view name_, std::string_view tag_, std::string_view password_, bool limited_, std::string_view guid_, int64_t owner_id_)
	: parent_id(parent_id_), name(name_), tag(tag_), password(password_), limited(limited_), guid(guid_), owner_id(owner_id_)
{
}

Command::~Command()
{
}

bool Command::Parse(std::string_view message_)
{
	try
	{
		spdlog::get("System")->trace("api::create_group :: perform parsing");

		auto j = nlohmann::json::parse(message_);
		
		if (j.count(PARENT_ID) != 0) parent_id = j.at(PARENT_ID).get<int64_t>();
		if (j.count(NAME) != 0) name = j.at(NAME).get<std::string>();
		if (j.count(TAG) != 0) tag = j.at(TAG).get<std::string>();
		if (j.count(PASSWORD) != 0) password = j.at(PASSWORD).get<std::string>();
		if (j.count(LIMITED) != 0) limited = j.at(LIMITED).get<uint8_t>() != 0;
		if (j.count(GUID) != 0) guid = j.at(GUID).get<std::string>();
		if (j.count(OWNER_ID) != 0) owner_id = j.at(OWNER_ID).get<int64_t>();

		return true;
	}
	catch (nlohmann::json::parse_error& ex)
	{
		spdlog::get("Error")->critical("api::create_group :: error parse json (byte: {0}, what: {1})", ex.byte, ex.what());
	}
	return false;
}

std::string Command::Serialize()
{
	return "{" + 
		quot(PARENT_ID) + ":" + std::to_string(parent_id) + "," +
		quot(NAME) + ":" + quot(Common::JSON::Screen(name)) + "," +
		quot(TAG) + ":" + quot(Common::JSON::Screen(tag)) + "," +
		quot(PASSWORD) + ":" + Common::JSON::Screen(password) + "," +
		quot(LIMITED) + (limited ? ":1," : ":0,") +
		quot(GUID) + ":" + Common::JSON::Screen(guid) + "," +
		quot(OWNER_ID) + ":" + std::to_string(owner_id) +
	"}";
}

}
}
