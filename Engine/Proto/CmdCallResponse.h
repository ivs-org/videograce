/**
 * CmdCallResponse.h - Contains protocol command CALL_RESPONSE
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#pragma once

#include <string>
#include <cstdint>

namespace Proto
{
namespace CALL_RESPONSE
{
	static const std::string NAME = "call_response";

	enum class Type
	{
		Undefined = 0,

		AutoCall,
		NotConnected,

		Accept,
		Refuse,
		Busy,
		Timeout
	};

	struct Command
	{
		int64_t id;
		uint32_t connection_id;
		std::string name;
		Type type;
		uint64_t time_limit;
		
		Command();
		Command(int64_t id, uint32_t connection_id, const std::string &name, Type type, uint64_t time_limit);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
