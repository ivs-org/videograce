/**
 * Member.h - Contains the member structure
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <string>
#include <cstdint>

#include <Proto/Group.h>

#include <boost/property_tree/ptree.hpp>

namespace Proto
{
	enum class MemberState
	{
		Undefined = 0,

		Offline,
		Online,

		Conferencing
	};

	struct Member
	{
		int64_t id;
		MemberState state;

		std::string login;
		std::string name;
		std::string number;

		std::string icon;
		std::string avatar;

		std::vector<Group> groups;

		uint32_t max_input_bitrate;
		uint32_t order;
		bool has_camera, has_microphone, has_demonstration;
		uint32_t grants;

		bool deleted;

		uint32_t unreaded_count; // not transmitted

		Member();
		Member(int64_t id);
        Member(int64_t id, std::string_view name);
		Member(int64_t id, bool deleted);
		Member(int64_t id, MemberState state, std::string_view login, std::string_view name, std::string_view number, const std::vector<Group> &groups);
		Member(int64_t id, MemberState state, std::string_view login, std::string_view name, std::string_view number, const std::vector<Group> &groups, uint32_t grants);
		Member(int64_t id, MemberState state);
		Member(int64_t id,
			MemberState state,
			std::string_view login,
			std::string_view name,
			std::string_view number,
			const std::vector<Group> &groups,
			std::string_view icon,
			std::string_view avatar,
			uint32_t max_input_bitrate,
			uint32_t order,
			bool has_camera,
			bool has_microphone,
            bool has_demonstration,
			uint32_t grants,
			bool deleted = false);

		~Member();

		bool Parse(const boost::property_tree::ptree &pt);
		std::string Serialize();

		inline bool operator==(int64_t id_)
		{
			return id == id_;
		}
	};
}
