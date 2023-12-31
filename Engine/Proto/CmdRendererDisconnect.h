/**
 * CmdRendererDisconnect.h - Contains protocol command RENDERER_DISCONNECT
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <string>
#include <cstdint>

namespace Proto
{
namespace RENDERER_DISCONNECT
{
	static const std::string NAME = "renderer_disconnect";

	struct Command
	{
		uint32_t device_id;
		uint32_t ssrc;

		Command();
		Command(uint32_t device_id, uint32_t ssrc);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
