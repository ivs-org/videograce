/**
 * Conference.cpp - Contains the conference structure impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <Proto/Conference.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <spdlog/spdlog.h>

namespace Proto
{

static const std::string ID = "id";
static const std::string NAME_ = "name";
static const std::string STATE = "state";
static const std::string TAG = "tag";
static const std::string DESCR = "descr";
static const std::string FOUNDER = "founder";
static const std::string FOUNDER_ID = "founder_id";
static const std::string TYPE = "type";
static const std::string GRANTS = "grants";
static const std::string DURATION = "duration";
static const std::string MEMBERS = "members";
static const std::string CONNECT_MEMBERS = "connect_members";
static const std::string TEMP = "temp";
static const std::string DELETED = "deleted";

Conference::Conference()
	: id(0), tag(), name(), descr(), founder(), founder_id(0), type(ConferenceType::Undefined), grants(0), duration(0), members(), connect_members(false), deleted(false), temp(false), unreaded_count(0), rolled(true)
{
}

Conference::Conference(int64_t id_)
	: id(id_), tag(), name(), descr(), founder(), founder_id(0), type(ConferenceType::Undefined), grants(0), duration(0), members(), connect_members(false), deleted(false), temp(false), unreaded_count(0), rolled(true)
{
}

Conference::Conference(std::string_view tag_)
    : id(0), tag(tag_), name(), descr(), founder(), founder_id(0), type(ConferenceType::Undefined), grants(0), duration(0), members(), connect_members(false), deleted(false), temp(false), unreaded_count(0), rolled(true)
{
}

Conference::Conference(int64_t id_, bool deleted_)
	: id(id_), tag(), name(), descr(), founder(), founder_id(0), type(ConferenceType::Undefined), grants(0), duration(0), members(), connect_members(false), deleted(deleted_), temp(false), unreaded_count(0), rolled(true)
{
}

Conference::Conference(int64_t id_,
		std::string_view tag_,
		std::string_view name_,
		std::string_view descr_,
		std::string_view founder_,
		int64_t founder_id_,
		ConferenceType type_,
		uint32_t grants_,
		uint32_t duration_,
		const std::vector<Member> &members_,
		bool connect_members_,
		bool temp_,
		bool deleted_,
		bool rolled_)
	: id(id_),
	tag(tag_),
	name(name_),
	descr(descr_),
	founder(founder_),
	founder_id(founder_id_),
	type(type_),
	grants(grants_),
	duration(duration_),
	members(members_),
	connect_members(connect_members_),
	temp(temp_),
	deleted(deleted_),
	unreaded_count(0),
	rolled(rolled_)
{
}

Conference::~Conference()
{
}

bool Conference::Parse(const nlohmann::json::object_t& obj)
{
	try
	{
		id = obj.at(ID).get<int64_t>();

		if (obj.count(TAG) != 0) tag = obj.at(TAG).get<std::string>();
		if (obj.count(NAME_) != 0) name = obj.at(NAME_).get<std::string>();
		if (obj.count(DESCR) != 0) descr = obj.at(DESCR).get<std::string>();
		if (obj.count(FOUNDER) != 0) founder = obj.at(FOUNDER).get<std::string>();
		if (obj.count(FOUNDER_ID) != 0) founder_id = obj.at(FOUNDER_ID).get<uint64_t>();
		if (obj.count(TYPE) != 0) type = static_cast<ConferenceType>(obj.at(TYPE).get<uint32_t>());
		if (obj.count(GRANTS) != 0) grants = obj.at(GRANTS).get<uint32_t>();
		if (obj.count(DURATION) != 0) duration = obj.at(DURATION).get<uint32_t>();
		if (obj.count(CONNECT_MEMBERS) != 0) connect_members = obj.at(CONNECT_MEMBERS).get<uint32_t>();
		if (obj.count(TEMP) != 0) temp = obj.at(TEMP).get<uint32_t>();
		if (obj.count(DELETED) != 0) deleted = obj.at(DELETED).get<uint32_t>();

		auto users = obj.at(MEMBERS);
		for (auto &u : users)
		{
			Member member;
			if (member.Parse(u))
			{
				members.emplace_back(member);
			}
		}
		
		return true;
	}
	catch (nlohmann::json::parse_error& ex)
	{
		spdlog::get("Error")->critical("proto::conference :: error parse json (byte: {0}, what: {1})", ex.byte, ex.what());
	}
	return false;
}

std::string Conference::Serialize()
{
	std::string out = "{" + quot(ID) + ":" + std::to_string(id) + "," +
		(!tag.empty() ? quot(TAG) + ":" + quot(Common::JSON::Screen(tag)) + "," : "") +
		(!name.empty() ? quot(NAME_) + ":" + quot(Common::JSON::Screen(name)) + "," : "") +
		(!descr.empty() ? quot(DESCR) + ":" + quot(Common::JSON::Screen(descr)) + "," : "") +
		(!founder.empty() ? quot(FOUNDER) + ":" + quot(Common::JSON::Screen(founder)) + "," : "") +
		(founder_id != 0 ? quot(FOUNDER_ID) + ":" + std::to_string(founder_id) + "," : "") +
		(type != ConferenceType::Undefined ? quot(TYPE) + ":" + std::to_string(static_cast<int32_t>(type)) + "," : "") +
		(grants != 0 ? quot(GRANTS) + ":" + std::to_string(grants) + "," : "") +
		(duration != 0 ? quot(DURATION) + ":" + std::to_string(duration) + "," : "") +
		(connect_members ? quot(CONNECT_MEMBERS) + ":1," : "") +
		(temp ? quot(TEMP) + ":1," : "") +
		(deleted ? quot(DELETED) + ":1," : "") +
		quot(MEMBERS) + ":[";

	for (auto &m : members)
	{
		out += m.Serialize() + ",";
	}
	if (!members.empty())
	{
		out.pop_back(); // drop last ","
	}

	return out + "]}";
}

void Conference::Clear()
{
	id = 0;
	tag.clear();
	name.clear();
	descr.clear();
	founder.clear();
	founder_id = 0;
	type = ConferenceType::Undefined;
	grants = 0;
	duration = 0;
	members.clear();
	connect_members = 0;
	deleted = false;
	rolled = true;
	unreaded_count = 0;
}

}
