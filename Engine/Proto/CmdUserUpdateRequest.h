/**
 * CmdUserUpdateRequest.h - Contains protocol command USER_UPDATE_REQUEST
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#pragma once

#include <string>
#include <cstdint>

namespace Proto
{
namespace USER_UPDATE_REQUEST
{
	enum class Action
	{
		Undefined = 0,

		Register,
		ChangeMeta
	};

	static const std::string NAME = "user_update_request";

	struct Command
	{
		Action action;

		int64_t id;

		std::string name;
		std::string avatar;

		std::string login;
		std::string password;

		Command();
		Command(Action action,
			int64_t id,
			std::string_view name,
			std::string_view avatar,
			std::string_view login,
			std::string_view password);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
