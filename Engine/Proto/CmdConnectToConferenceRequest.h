/**
 * CmdConnectToConferenceRequest.h - Contains protocol command CONNECT_TO_CONFERENCE_REQUEST
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <string>
#include <cstdint>

namespace Proto
{
namespace CONNECT_TO_CONFERENCE_REQUEST
{
	static const std::string NAME = "connect_to_conference_request";

	struct Command
	{
		std::string tag;
		bool connect_members;
		bool has_camera, has_microphone, has_demonstration;
		
		Command();
		Command(const std::string &tag);
		Command(const std::string &tag, bool connect_members, bool has_camera, bool has_microphone, bool has_demonstration);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
