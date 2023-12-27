/**
 * CmdLoadBlobs.h - Contains protocol command LOAD_BLOBS
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2023
 */

#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace Proto
{
namespace LOAD_BLOBS
{
	static const std::string NAME = "load_blobs";

	struct Command
	{
		std::vector<std::string> guids;

		Command();
		Command(std::vector<std::string> &guids);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
