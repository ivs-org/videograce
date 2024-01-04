/**
 * ServerInfo.h - Contains API server info json header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#pragma once

#include <string>
#include <string_view>
#include <cstdint>

namespace API
{
namespace SERVER_INFO
{
	struct Command
	{
		std::string company;
		std::string server_id;

		std::string installation_id;
		std::string license_key;
		int64_t key_expired; // Unix timestamp
		int32_t channels_count;
		
		std::string address;
		std::string admin_email;

		int8_t enabled_crypt;

		uint16_t command_port, first_av_port, translators_count;
		
		std::string server_version;
		std::string system;
		
		Command();
		Command(std::string_view company,
			std::string_view server_id,

			std::string_view installation_id,
			std::string_view license_key,
			int64_t key_expired,
			int32_t channels_count,

			std::string_view address,
			std::string_view admin_email,

			bool enabled_crypt,

			uint16_t command_port,
			uint16_t first_av_port,
			uint16_t translators_count,
			
			std::string_view server_version,
			std::string_view system);

		Command(std::string_view company,
			std::string_view server_id,

			std::string_view address,
			std::string_view admin_email,

			std::string_view server_version,
			std::string_view system);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
