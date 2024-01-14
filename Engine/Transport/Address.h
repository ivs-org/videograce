/**
 * Address.h - Contains Transport's address struct
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#pragma once

#include <cstdint>
#include <string>

#ifndef _WIN32
#include <netinet/in.h>
#else
#include <winsock2.h>
#include <ws2ipdef.h>
#endif

namespace Transport
{

struct Address
{
	enum class Type
	{
		Undefined = 0,

		IPv4,
		IPv6,

		Auto
	};

	Type type;

	sockaddr_in v4addr;

	sockaddr_in6 v6addr;
		
	Address();

	Address(std::string_view addr, uint16_t port);

	Address(const sockaddr_storage *addr);

	bool operator==(const Address& lv) const;
				
	std::string toString() const;
};

}
