/**
 * Group.cpp - Contains group structure impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021, 2023
 */

#include <Proto/Group.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <spdlog/spdlog.h>

namespace Proto
{

static const std::string ID = "id";
static const std::string PARENT_ID = "parent_id";
static const std::string TAG = "tag";
static const std::string NAME_ = "name";
static const std::string OWNER_ID = "owner_id";
static const std::string PASSWORD = "password";
static const std::string GRANTS = "grants";
static const std::string LEVEL = "level";
static const std::string DELETED = "deleted";

Group::Group()
	: id(0), parent_id(-1), tag(), name(), owner_id(-1), password(), grants(0), deleted(false), level(-1), rolled(false)
{
}

Group::Group(int64_t id_, bool deleted_)
	: id(id_), parent_id(-1), tag(), name(), password(), grants(0), deleted(deleted_), level(-1), rolled(false)
{

}

Group::Group(int64_t id_, int64_t parent_id_, std::string_view tag_, std::string_view name_, int64_t owner_id_, std::string_view password_, uint32_t grants_, int32_t level_, bool deleted_)
	: id(id_), parent_id(parent_id_), tag(tag_), name(name_), owner_id(owner_id_), password(password_), grants(grants_), deleted(deleted_), level(level_), rolled(false)
{
}

Group::~Group()
{
}

bool Group::Parse(const nlohmann::json::object_t &obj)
{
	try
	{
		id = obj.at(ID).get<int64_t>();

		if (obj.count(PARENT_ID) != 0) parent_id = obj.at(PARENT_ID).get<int64_t>();
		if (obj.count(TAG) != 0) tag = obj.at(TAG).get<std::string>();
		if (obj.count(NAME_) != 0) name = obj.at(NAME_).get<std::string>();
		if (obj.count(OWNER_ID) != 0) owner_id = obj.at(OWNER_ID).get<int64_t>();
		if (obj.count(PASSWORD) != 0) password = obj.at(PASSWORD).get<std::string>();
		if (obj.count(GRANTS) != 0) grants = obj.at(GRANTS).get<uint32_t>();
		if (obj.count(LEVEL) != 0) level = obj.at(LEVEL).get<int32_t>();
		if (obj.count(DELETED) != 0) deleted = obj.at(DELETED).get<uint8_t>();

		return true;
	}
	catch (nlohmann::json::parse_error& ex)
	{
		spdlog::get("Error")->critical("proto::group :: error parse json (byte: {0}, what: {1})", ex.byte, ex.what());
	}
	return false;
}

std::string Group::Serialize()
{
	return "{" + quot(ID) + ":" + std::to_string(id) +
		(parent_id != -1 ? "," + quot(PARENT_ID) + ":" + std::to_string(parent_id) : "") +
		(!tag.empty() ? "," + quot(TAG) + ":" + quot(Common::JSON::Screen(tag)) : "") +
		(!name.empty() ? "," + quot(NAME_) + ":" + quot(Common::JSON::Screen(name)) : "") +
		(owner_id != -1 ? "," + quot(OWNER_ID) + ":" + std::to_string(owner_id) : "") +
		(!password.empty() ? "," + quot(PASSWORD) + ":" + quot(Common::JSON::Screen(password)) : "") +
		(grants != 0 ? "," + quot(GRANTS) + ":" + std::to_string(grants) : "") +
		(level != -1 ? "," + quot(LEVEL) + ":" + std::to_string(level) : "") +
		(deleted ? "," + quot(DELETED) + ":1" : "") + "}";
}

}
