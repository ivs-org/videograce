/**
 * ServerInfo.h - Contains API server info json header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#pragma once

#include <string>
#include <cstdint>

namespace API
{
namespace SERVER_INFO
{
	struct Command
	{
		std::string company;
		std::string server_id;
		std::string address;
		uint16_t port;
		bool enabled_crypt;
		uint16_t translators_count;
		std::string admin_email;
		
		Command();
		Command(const std::string &company,
			const std::string &server_id,
			const std::string &address,
			uint16_t port,
			bool enabled_crypt,
			uint16_t translators_count,
			const std::string &admin_email);

		~Command();

		bool Parse(const std::string &message);
		std::string Serialize();
	};
}
}
