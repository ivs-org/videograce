/**
 * Group.h - Contains the group structure
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#pragma once

#include <string>
#include <cstdint>

#include <boost/property_tree/ptree.hpp>

namespace Proto
{
	struct Group
	{
		int64_t id;
		int64_t parent_id;
		
		std::string tag;
		std::string name;

		int64_t owner_id;

		std::string password;
		uint32_t grants;

		int32_t level;

		bool deleted;

		bool rolled; // not transmitted

		Group();
		Group(int64_t id, bool deleted = false);
		Group(int64_t id,
			int64_t parent_id,
			std::string_view tag,
			std::string_view name,
			int64_t owner_id,
			std::string_view password,
			uint32_t grants,
			int32_t level,
			bool deleted = false);

		~Group();

		bool Parse(const boost::property_tree::ptree &pt);
		std::string Serialize();

		inline bool operator==(int64_t id_) const
		{
			return id == id_;
		}
	};
}
