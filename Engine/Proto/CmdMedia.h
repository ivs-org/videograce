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

enum class MediaType
{
	Undefined,
	RTP,
	RTCP
};

struct Command
{
	MediaType media_type;
	uint32_t ssrc;
	std::string addr; /// Destination UDP address:port needed to transmit on server side
	std::string data; /// RTP/RTCP packet with header on base 64
		
	Command();
	Command(MediaType media_type, uint32_t ssrc, std::string_view addr, std::string_view data);

	~Command();
	
	bool Parse(std::string_view message);
	std::string Serialize();
};

}
}
