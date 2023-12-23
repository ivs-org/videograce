/**
 * CmdDeliveryBlobs.h - Contains protocol command SEND_BLOB_REQUEST
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2023
 */

#pragma once

#include <string>
#include <cstdint>

#include <Proto/Blob.h>

namespace Proto
{
namespace DELIVERY_BLOBS
{
	static const std::string NAME = "delivery_blobs";

	struct Command
	{
		std::vector<Blob> blobs;

		Command();
		Command(std::vector<Blob> &blobs);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
