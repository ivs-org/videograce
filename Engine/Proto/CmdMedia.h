/**
 * CmdMedia.h - Contains command to transfer RTP over json via WebSocket header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2024
 */

#pragma once

#include <string>
#include <cstdint>

namespace Proto
{
namespace MEDIA
{
	
static const std::string NAME = "media";

struct Command
{
	uint16_t src_port, dst_port;
	std::string rtp; /// RTP packet with header on base 64
		
	Command();
	Command(uint16_t src_port, uint16_t dst_port, std::string_view rtp);

	~Command();
	
	bool Parse(std::string_view message);
	std::string Serialize();
};

}
}
