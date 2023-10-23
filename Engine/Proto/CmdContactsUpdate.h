/**
 * CmdContactsUpdate.h - Contains protocol command CONTACTS_UPDATE
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#pragma once

#include <string>
#include <cstdint>

namespace Proto
{
namespace CONTACTS_UPDATE
{
	enum class Action
	{
		Undefined = 0,

		Add,
		Delete
	};

	static const std::string NAME = "contacts_update";

	struct Command
	{
		Action action;

        int64_t client_id;

		Command();
		Command(Action action, int64_t client_id);

		~Command();

		bool Parse(const std::string &message);
		std::string Serialize();
	};
}
}
