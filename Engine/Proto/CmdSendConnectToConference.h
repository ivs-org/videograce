/**
 * CmdSendConnectToConference.h - Contains protocol command SEND_CONNECT_TO_CONFERENCE
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <string>
#include <cstdint>

namespace Proto
{
namespace SEND_CONNECT_TO_CONFERENCE
{
    enum class Flag
    {
        InviteCall = 0,
        AddMember
    };

	static const std::string NAME = "send_connect_to_conference";

	struct Command
	{
		std::string tag;
		int64_t connecter_id;
		uint32_t connecter_connection_id;
        uint32_t flags;
		
		Command();
		Command(std::string_view tag, int64_t connecter_id, uint32_t connecter_connection_id, uint32_t flags);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
