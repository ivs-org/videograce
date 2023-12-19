/**
 * Member.cpp - Contains member structure impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/Member.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

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

bool Member::Parse(const boost::property_tree::ptree &pt)
{
	try
	{
		id = pt.get<int64_t>(ID);

		auto state_opt = pt.get_optional<uint32_t>(STATE);
		if (state_opt) state = static_cast<MemberState>(state_opt.get());

		auto login_opt = pt.get_optional<std::string>(LOGIN);
		if (login_opt) login = login_opt.get();

		auto name_opt = pt.get_optional<std::string>(NAME_);
		if (name_opt) name = name_opt.get();

		auto number_opt = pt.get_optional<std::string>(NUMBER);
		if (number_opt) number = number_opt.get();

		auto groups_opt = pt.get_child_optional(GROUPS);
		if (groups_opt)
		{
			auto &groups_ = groups_opt.get();
			for (auto &g : groups_)
			{
				Group group;
				if (group.Parse(g.second))
				{
					groups.emplace_back(group);
				}
			}
		}

		auto icon_opt = pt.get_optional<std::string>(ICON);
		if (icon_opt) icon = icon_opt.get();

		auto avatar_opt = pt.get_optional<std::string>(AVATAR);
		if (avatar_opt) avatar = avatar_opt.get();

		auto max_input_bitrate_opt = pt.get_optional<uint32_t>(MAX_INPUT_BITRATE);
		if (max_input_bitrate_opt) max_input_bitrate = max_input_bitrate_opt.get();

		auto order_opt = pt.get_optional<uint32_t>(ORDER);
		if (order_opt) order = order_opt.get();

		auto has_camera_opt = pt.get_optional<uint8_t>(HAS_CAMERA);
		if (has_camera_opt) has_camera = has_camera_opt.get() != 0;
		
		auto has_microphone_opt = pt.get_optional<uint8_t>(HAS_MICROPHONE);
		if (has_microphone_opt) has_microphone = has_microphone_opt.get() != 0;

        auto has_demonstration_opt = pt.get_optional<uint8_t>(HAS_DEMONSTRATION);
        if (has_demonstration_opt) has_demonstration = has_demonstration_opt.get() != 0;
		
		auto grants_opt = pt.get_optional<uint32_t>(GRANTS);
		if (grants_opt) grants = grants_opt.get();

		auto is_deleted_opt = pt.get_optional<uint8_t>(DELETED);
		if (is_deleted_opt) deleted = is_deleted_opt.get() != 0;
		
		return true;
	}
	catch (std::exception const& e)
	{
		DBGTRACE("Error parsing member, %s\n", e.what());
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
