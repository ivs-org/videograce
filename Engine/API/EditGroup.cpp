/**
 * EditGroup.cpp - Contains API edit group json impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <API/EditGroup.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

namespace API
{
namespace EDIT_GROUP
{

static const std::string ID = "id";
static const std::string PARENT_ID = "parent_id";
static const std::string NAME = "name";
static const std::string TAG = "tag";
static const std::string PASSWORD = "password";
static const std::string LIMITED = "limited";
static const std::string GUID = "guid";
static const std::string OWNER_ID = "owner_id";

Command::Command()
	: id(-1), parent_id(-1), name(), tag(), password(), limited(false), guid(), owner_id(0)
{
}

Command::Command(int64_t id_, int64_t parent_id_, std::string_view name_, std::string_view tag_, std::string_view password_, bool limited_, std::string_view guid_, int64_t owner_id_)
	: id(id_), parent_id(parent_id_), name(name_), tag(tag_), password(password_), limited(limited_), guid(guid_), owner_id(owner_id_)
{
}

Command::~Command()
{
}

bool Command::Parse(std::string_view message_)
{
	using boost::property_tree::ptree;

	std::stringstream ss;
	ss << message_;

	try
	{
		ptree pt;
		read_json(ss, pt);

		auto id_opt = pt.get_optional<int64_t>(ID);
		if (id_opt) id = id_opt.get();

		auto parent_id_opt = pt.get_optional<int64_t>(PARENT_ID);
		if (parent_id_opt) parent_id = parent_id_opt.get();

		auto name_opt = pt.get_optional<std::string>(NAME);
		if (name_opt) name = name_opt.get();
		
		auto tag_opt = pt.get_optional<std::string>(TAG);
		if (tag_opt) tag = tag_opt.get();

		auto password_opt = pt.get_optional<std::string>(PASSWORD);
		if (password_opt) password = password_opt.get();

		auto limited_opt = pt.get_optional<uint8_t>(LIMITED);
		if (limited_opt) limited = limited_opt.get() != 0;

		auto guid_opt = pt.get_optional<std::string>(GUID);
		if (guid_opt) guid = guid_opt.get();

		auto owner_id_opt = pt.get_optional<int64_t>(OWNER_ID);
		if (owner_id_opt) owner_id = owner_id_opt.get();

		return true;
	}
	catch (std::exception const& e)
	{
		DBGTRACE("Error parsing EditGroup %s\n", e.what());
	}
	return false;
}

std::string Command::Serialize()
{
	return "{" +
		quot(ID) + ":" + std::to_string(id) + "," +
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
