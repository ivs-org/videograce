/**
 * Conference.cpp - Contains the conference structure impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/Conference.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

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

Conference::Conference(const std::string &tag_)
    : id(0), tag(tag_), name(), descr(), founder(), founder_id(0), type(ConferenceType::Undefined), grants(0), duration(0), members(), connect_members(false), deleted(false), temp(false), unreaded_count(0), rolled(true)
{
}

Conference::Conference(int64_t id_, bool deleted_)
	: id(id_), tag(), name(), descr(), founder(), founder_id(0), type(ConferenceType::Undefined), grants(0), duration(0), members(), connect_members(false), deleted(deleted_), temp(false), unreaded_count(0), rolled(true)
{
}

Conference::Conference(int64_t id_,
		const std::string &tag_,
		const std::string &name_,
		const std::string &descr_,
		const std::string &founder_,
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

bool Conference::Parse(const boost::property_tree::ptree &pt)
{
	try
	{
		id = pt.get<int64_t>(ID);

		auto tag_opt = pt.get_optional<std::string>(TAG);
		if (tag_opt) tag = tag_opt.get();

		auto name_opt = pt.get_optional<std::string>(NAME_);
		if (name_opt) name = name_opt.get();

		auto descr_opt = pt.get_optional<std::string>(DESCR);
		if (descr_opt) descr = descr_opt.get();

		auto founder_opt = pt.get_optional<std::string>(FOUNDER);
		if (founder_opt) founder = founder_opt.get();
		
		auto founder_id_opt = pt.get_optional<int64_t>(FOUNDER_ID);
		if (founder_id_opt) founder_id = founder_id_opt.get();

		auto type_opt = pt.get_optional<uint32_t>(TYPE);
		if (type_opt) type = static_cast<ConferenceType>(type_opt.get());

		auto grants_opt = pt.get_optional<uint32_t>(GRANTS);
		if (grants_opt) grants = grants_opt.get();

		auto duration_opt = pt.get_optional<uint32_t>(DURATION);
		if (duration_opt) duration = duration_opt.get();
		
		auto connect_members_opt = pt.get_optional<uint32_t>(CONNECT_MEMBERS);
		if (connect_members_opt) connect_members = connect_members_opt.get() != 0;

		auto temp_opt = pt.get_optional<uint32_t>(TEMP);
		if (temp_opt) temp = temp_opt.get() != 0;

		auto deleted_opt = pt.get_optional<uint32_t>(DELETED);
		if (deleted_opt) deleted = deleted_opt.get() != 0;

		auto &users = pt.get_child(MEMBERS);
		for (auto &u : users)
		{
			Member member;
			if (member.Parse(u.second))
			{
				members.emplace_back(member);
			}
		}
		
		return true;
	}
	catch (std::exception const& e)
	{
		DBGTRACE("Error parsing conference, %s\n", e.what());
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
