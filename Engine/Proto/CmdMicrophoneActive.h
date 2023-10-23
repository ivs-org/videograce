/**
 * CmdMicrophoneActive.h - Contains protocol command MICROPHONE_ACTIVE
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#pragma once

#include <string>
#include <cstdint>

namespace Proto
{
namespace MICROPHONE_ACTIVE
{
	static const std::string NAME = "microphone_active";

	enum class ActiveType
	{
		Undefined = 0,
		Silent,
		Speak
	};

	struct Command
	{
		ActiveType active_type;
		uint32_t device_id;
		int64_t client_id;

		Command();
		Command(ActiveType active_type, uint32_t device_id, int64_t client_id);

		~Command();

		bool Parse(const std::string &message);
		std::string Serialize();
	};
}
}
