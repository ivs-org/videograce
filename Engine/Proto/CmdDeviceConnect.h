/**
 * CmdDeviceConnect.h - Contains protocol command DEVICE_CONNECT
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#pragma once

#include <string>
#include <cstdint>

#include <Proto/DeviceType.h>

#include <Video/ColorSpace.h>

namespace Proto
{
namespace DEVICE_CONNECT
{
	static const std::string NAME = "device_connect";

	enum class ConnectType
	{
		Undefined = 0,
		CreatedDevice,
		ConnectRenderer
	};

	struct Command
	{
		ConnectType connect_type;
		DeviceType device_type;
		uint32_t device_id;
		int64_t client_id;
		std::string metadata;
		uint32_t receiver_ssrc;
		uint32_t author_ssrc;
		std::string address;
		uint16_t port;
		std::string name;
		uint32_t resolution;
        Video::ColorSpace color_space;
		bool my;
		std::string secure_key;

		Command();
		Command(ConnectType connect_type, DeviceType device_type, uint32_t device_id, int64_t client_id, const std::string &metadata, uint32_t receiver_ssrc, uint32_t author_ssrc, const std::string &address, uint32_t port, const std::string &name, uint32_t resolution, Video::ColorSpace color_space, bool my, const std::string &secure_key);

		~Command();

		bool Parse(std::string_view message);
		std::string Serialize();
	};
}
}
