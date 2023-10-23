/**
 * CmdTurnSpeaker.h - Contains protocol command TURN_SPEAKER
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <string>
#include <cstdint>

namespace Proto
{
namespace TURN_SPEAKER
{
	static const std::string NAME = "turn_speaker";

	struct Command
	{
		Command();
		
		~Command();

		bool Parse(const std::string &message);
		std::string Serialize();
	};
}
}
