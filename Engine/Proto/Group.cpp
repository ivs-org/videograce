/**
 * Group.cpp - Contains group structure impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/Group.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

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

bool Group::Parse(const boost::property_tree::ptree &pt)
{
	try
	{
		id = pt.get<int64_t>(ID);

		auto parent_id_opt = pt.get_optional<int64_t>(PARENT_ID);
		if (parent_id_opt) parent_id = parent_id_opt.get();

		auto tag_opt = pt.get_optional<std::string>(TAG);
		if (tag_opt) tag = tag_opt.get();

		auto name_opt = pt.get_optional<std::string>(NAME_);
		if (name_opt) name = name_opt.get();

		auto owner_id_opt = pt.get_optional<int64_t>(OWNER_ID);
		if (owner_id_opt) owner_id = owner_id_opt.get();

		auto password_opt = pt.get_optional<std::string>(PASSWORD);
		if (password_opt) password = password_opt.get();

		auto grants_opt = pt.get_optional<uint32_t>(GRANTS);
		if (grants_opt) grants = grants_opt.get();

		auto level_opt = pt.get_optional<int32_t>(LEVEL);
		if (level_opt) level = level_opt.get();

		auto is_deleted_opt = pt.get_optional<uint8_t>(DELETED);
		if (is_deleted_opt) deleted = is_deleted_opt.get() != 0;
		
		return true;
	}
	catch (std::exception const& e)
	{
		DBGTRACE("Error parsing group, %s\n", e.what());
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
