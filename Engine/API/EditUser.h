/**
 * EditUser.h - Contains API edit user json header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#pragma once

#include <string>
#include <cstdint>

#include <Proto/Group.h>

namespace API
{
namespace EDIT_USER
{
	struct Command
	{
		int64_t id;
		std::string name;
		std::string login;
		std::string password;
		uint32_t number;
		std::vector<Proto::Group> groups;
		uint64_t time_limit;
		bool allow_create_conference;
		bool use_only_tcp;
		std::string guid;
		
		Command();
		Command(int64_t id, std::string_view name, std::string_view login, std::string_view password, uint32_t number, const std::vector<Proto::Group> &groups, uint64_t time_limit, bool allow_create_conference, bool use_only_tcp, std::string_view guid);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
