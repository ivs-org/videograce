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
        Member(int64_t id, const std::string &name);
		Member(int64_t id, bool deleted);
		Member(int64_t id, MemberState state, const std::string &login, const std::string &name, const std::string &number, const std::vector<Group> &groups);
		Member(int64_t id, MemberState state, const std::string &login, const std::string &name, const std::string &number, const std::vector<Group> &groups, uint32_t grants);
		Member(int64_t id, MemberState state);
		Member(int64_t id,
			MemberState state,
			const std::string &login,
			const std::string &name,
			const std::string &number,
			const std::vector<Group> &groups,
			const std::string &icon,
			const std::string &avatar,
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
