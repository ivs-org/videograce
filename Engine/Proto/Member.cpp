/**
 * Member.cpp - Contains member structure impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <Proto/Member.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <spdlog/spdlog.h>

namespace Proto
{

static const std::string ID = "id";
static const std::string STATE = "state";
static const std::string LOGIN = "login";
static const std::string NAME_ = "name";
static const std::string NUMBER = "number";
static const std::string GROUPS = "groups";
static const std::string ICON = "icon";
static const std::string AVATAR = "avatar";
static const std::string MAX_INPUT_BITRATE = "max_input_bitrate";
static const std::string ORDER = "order";
static const std::string HAS_CAMERA = "has_camera";
static const std::string HAS_MICROPHONE = "has_microphone";
static const std::string HAS_DEMONSTRATION = "has_demonstration";
static const std::string GRANTS = "grants";
static const std::string DELETED = "deleted";

Member::Member()
	: id(0), state(MemberState::Undefined), login(), name(), number(), groups(), icon(), avatar(), max_input_bitrate(0), order(0), has_camera(false), has_microphone(false), has_demonstration(false), grants(0), deleted(false), unreaded_count(0)
{
}

Member::Member(int64_t id_)
	: id(id_), state(MemberState::Undefined), login(), name(), number(), groups(), icon(), avatar(), max_input_bitrate(0), order(0), has_camera(false), has_microphone(false), has_demonstration(false), grants(0), deleted(false), unreaded_count(0)
{
}

Member::Member(int64_t id_, std::string_view name_)
    : id(id_), state(MemberState::Undefined), login(), name(name_), number(), groups(), icon(), avatar(), max_input_bitrate(0), order(0), has_camera(false), has_microphone(false), has_demonstration(false), grants(0), deleted(false), unreaded_count(0)
{
}

Member::Member(int64_t id_, bool deleted_)
	: id(id_), state(MemberState::Undefined), login(), name(), number(), groups(), icon(), avatar(), max_input_bitrate(0), order(0), has_camera(false), has_microphone(false), has_demonstration(false), grants(0), deleted(deleted_), unreaded_count(0)
{
}

Member::Member(int64_t id_, MemberState state_)
	: id(id_), state(state_), login(), name(), number(), groups(), icon(), avatar(), max_input_bitrate(0), order(0), has_camera(false), has_microphone(false), has_demonstration(false), grants(0), deleted(false), unreaded_count(0)
{
}

Member::Member(int64_t id_, MemberState state_, std::string_view login_, std::string_view name_, std::string_view number_, const std::vector<Group> &groups_)
	: id(id_), state(state_), login(login_), name(name_), number(number_), groups(groups_), icon(), avatar(), max_input_bitrate(0), order(0), has_camera(false), has_microphone(false), has_demonstration(false), grants(0), deleted(false), unreaded_count(0)
{
}

Member::Member(int64_t id_, MemberState state_, std::string_view login_, std::string_view name_, std::string_view number_, const std::vector<Group> &groups_, uint32_t grants_)
	: id(id_), state(state_), login(login_), name(name_), number(number_), groups(groups_), icon(), avatar(), max_input_bitrate(0), order(0), has_camera(false), has_microphone(false), has_demonstration(false), grants(grants_), deleted(false), unreaded_count(0)
{
}

Member::Member(int64_t id_, MemberState state_, std::string_view login_, std::string_view name_, std::string_view number_, const std::vector<Group> &groups_, std::string_view icon_, std::string_view avatar_, uint32_t max_input_bitrate_, uint32_t order_, bool has_camera_, bool has_microphone_, bool has_demonstration_, uint32_t grants_, bool deleted_)
	: id(id_), state(state_), login(login_), name(name_), number(number_), groups(groups_), icon(icon_), avatar(avatar_), max_input_bitrate(max_input_bitrate_), order(order_), has_camera(has_camera_), has_demonstration(has_demonstration_), has_microphone(has_microphone_), grants(grants_), deleted(deleted_), unreaded_count(0)
{
}

Member::~Member()
{
}

bool Member::Parse(const nlohmann::json::object_t &obj)
{
	try
	{
		id = obj.at(ID).get<int64_t>();

		if (obj.count(STATE) != 0) state = static_cast<MemberState>(obj.at(STATE).get<uint32_t>());
		if (obj.count(LOGIN) != 0) login = obj.at(LOGIN).get<std::string>();
		if (obj.count(NAME_) != 0) name = obj.at(NAME_).get<std::string>();
		if (obj.count(NUMBER) != 0) number = obj.at(NUMBER).get<std::string>();

		if (obj.count(GROUPS) != 0)
		{
			auto groups_ = obj.at(GROUPS);
			for (auto &g : groups_)
			{
				Group group;
				if (group.Parse(g))
				{
					groups.emplace_back(group);
				}
			}
		}

		if (obj.count(ICON) != 0) icon = obj.at(ICON).get<std::string>();
		if (obj.count(AVATAR) != 0) avatar = obj.at(AVATAR).get<std::string>();
		if (obj.count(MAX_INPUT_BITRATE) != 0) max_input_bitrate = obj.at(MAX_INPUT_BITRATE).get<uint32_t>();
		if (obj.count(ORDER) != 0) order = obj.at(ORDER).get<uint32_t>();
		if (obj.count(HAS_CAMERA) != 0) has_camera = obj.at(HAS_CAMERA).get<uint8_t>();
		if (obj.count(HAS_MICROPHONE) != 0) has_microphone = obj.at(HAS_MICROPHONE).get<uint8_t>();
		if (obj.count(HAS_DEMONSTRATION) != 0) has_demonstration = obj.at(HAS_DEMONSTRATION).get<uint8_t>();
		if (obj.count(GRANTS) != 0) grants = obj.at(GRANTS).get<uint32_t>();
		if (obj.count(DELETED) != 0) deleted = obj.at(DELETED).get<uint8_t>();

		return true;
	}
	catch (nlohmann::json::parse_error& ex)
	{
		spdlog::get("Error")->critical("proto::member :: error parse json (byte: {0}, what: {1})", ex.byte, ex.what());
	}
	return false;
}

std::string Member::Serialize()
{
	std::string out = "{" + quot(ID) + ":" + std::to_string(id) +
		(state != MemberState::Undefined ? "," + quot(STATE) + ":" + std::to_string(static_cast<int32_t>(state)) : "") +
		(!login.empty() ? "," + quot(LOGIN) + ":" + quot(Common::JSON::Screen(login)) : "") +
		(!name.empty() ? "," + quot(NAME_) + ":" + quot(Common::JSON::Screen(name)) : "") +
		(!number.empty() ? "," + quot(NUMBER) + ":" + quot(Common::JSON::Screen(number)) : "");

	if (!groups.empty())
	{
		out += "," + quot(GROUPS) + ":[";
		for (auto &g : groups)
		{
			out += g.Serialize() + ",";
		}
		out.pop_back(); // drop last ","
		out += "]";
	}

	out += (!icon.empty() ? "," + quot(ICON) + ":" + quot(Common::JSON::Screen(icon)) : "") +
		(!avatar.empty() ? "," + quot(AVATAR) + ":" + quot(Common::JSON::Screen(avatar)) : "") +
		(max_input_bitrate != 0 ? "," + quot(MAX_INPUT_BITRATE) + ":" + std::to_string(max_input_bitrate) : "") +
		(order != 0 ? "," + quot(ORDER) + ":" + std::to_string(order) : "") +
		(has_camera ? "," + quot(HAS_CAMERA) + ":1" : "") +
		(has_microphone ? "," + quot(HAS_MICROPHONE) + ":1" : "") +
        (has_demonstration ? "," + quot(HAS_DEMONSTRATION) + ":1" : "") +
		(grants != 0 ? "," + quot(GRANTS) + ":" + std::to_string(grants) : "") +
		(deleted ? "," + quot(DELETED) + ":1" : "") + "}";

	return out;
}

}
