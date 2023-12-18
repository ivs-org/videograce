/**
 * Conference.h - Contains the conference structure
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include <Proto/Member.h>

namespace Proto
{
	enum class ConferenceType
	{
		Undefined = 0,

		Symmetric,
		Asymmetric,
		AsymmetricWithSymmetricSound
	};

	struct Conference
	{
		int64_t id;
		std::string tag;
		std::string name;
		std::string descr;
		std::string founder;
		int64_t founder_id;
		ConferenceType type;
		uint32_t grants;
		uint64_t duration;
		std::vector<Member> members;
		bool connect_members;
		bool temp;
		bool deleted;

		uint32_t unreaded_count; // not trasmitted

		bool rolled; // not transmitted

		Conference();
		Conference(int64_t id);
        Conference(std::string_view tag);
		Conference(int64_t id, bool deleted);
		Conference(int64_t id,
			std::string_view tag,
			std::string_view name,
			std::string_view descr,
			std::string_view founder,
			int64_t founder_id,
			ConferenceType type,
			uint32_t grants,
			uint32_t duration,
			const std::vector<Member> &members,
			bool connect_members,
			bool temp,
			bool deleted = false,
			bool rolled = false);

		~Conference();

		bool Parse(const boost::property_tree::ptree &pt);
		std::string Serialize();

		void Clear();

		inline bool operator==(uint32_t id_)
		{
			return id == id_;
		}

		inline bool operator==(std::string_view tag_)
		{
			return tag == tag_;
		}
	};
}
