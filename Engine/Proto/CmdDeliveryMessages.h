/**
 * CmdDeliveryMessage.h - Contains protocol command DELIVERY_MESSAGES
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2019
 */

#pragma once

#include <string>
#include <cstdint>
#include <vector>

#include <Proto/Message.h>

namespace Proto
{
namespace DELIVERY_MESSAGES
{
	static const std::string NAME = "delivery_messages";

	struct Command
	{
		std::vector<Message> messages;

		Command();
		Command(const std::vector<Message> &message);
		Command(const Message &message);

		~Command();

		bool Parse(const std::string &message);
		std::string Serialize();
	};
}
}
