/**
 * CmdCallRequest.h - Contains protocol command CALL_REQUEST
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#pragma once

#include <string>
#include <cstdint>

namespace Proto
{
namespace CALL_REQUEST
{
	
static const std::string NAME = "call_request";

enum class Type
{
	Undefined = 0,

	Invocation,
	Cancel
};

struct Command
{
	std::string name;
	int64_t id;
	uint32_t connection_id;
	Type type;
	uint64_t time_limit;
		
	Command();
	Command(std::string_view name, int64_t id, uint32_t connection_id, Type type, uint64_t time_limit);

	~Command();
	
	bool Parse(std::string_view message);
	std::string Serialize();
};

std::string str(Type t);

}
}
