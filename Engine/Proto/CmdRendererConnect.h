/**
 * CmdRendererConnect.h - Contains protocol command RENDERER_CONNECT
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <string>
#include <cstdint>

namespace Proto
{
namespace RENDERER_CONNECT
{
	static const std::string NAME = "renderer_connect";

	struct Command
	{
		uint32_t device_id;
		uint32_t ssrc;

		Command();
		Command(uint32_t device_id, uint32_t ssrc);

		~Command();

		bool Parse(const std::string &message);
		std::string Serialize();
	};
}
}
