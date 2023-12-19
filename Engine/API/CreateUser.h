/**
 * CreateUser.h - Contains API create user json header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#pragma once

#include <string>
#include <cstdint>

namespace API
{
namespace CREATE_USER
{
	struct Command
	{
		int64_t group_id;
		std::string name;
		std::string login;
		std::string password;
		uint32_t number;
		uint64_t time_limit;
		bool allow_create_conference;
		bool use_only_tcp;
		std::string guid;
		
		Command();
		Command(int64_t group_id, std::string_view name, std::string_view login, std::string_view password, uint32_t number, uint64_t time_limit, bool allow_create_conference, bool use_only_tcp, std::string_view guid);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
