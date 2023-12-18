/**
 * CmdRequestMediaAddresses.h - Contains protocol command REQUEST_MEDIA_ADDRESSES
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <string>
#include <cstdint>

namespace Proto
{
namespace REQUEST_MEDIA_ADDRESSES
{
	static const std::string NAME = "request_media_addresses";

	struct Command
	{
		Command();
		
		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
