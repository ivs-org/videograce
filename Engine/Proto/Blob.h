/**
 * Blob.h - Contains the BLOB structure
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2023
 */

#pragma once

#include <string>
#include <cstdint>

#include <nlohmann/json.hpp>

namespace Proto
{
	enum class BlobAction
	{
		Undefined,

		SpeedTest,

		FileToStorage,
		P2PTransfer
	};

	enum class BlobStatus
	{
		Undefined = 0,
		Created,
		Sended,
		Delivered,
		Modified,
		Deleted
	};

	struct Blob
	{
		int64_t id;
		int64_t owner_id;
		std::string guid;

		BlobAction action;
		BlobStatus status;
		
		std::string data;
		std::string name;
		std::string description;

		bool deleted;

		Blob();
		Blob(int64_t id,
			int64_t owner_id,
			std::string_view guid,
			BlobAction action,
			BlobStatus status,
			std::string_view data,
			std::string_view name,
			std::string_view description,
			bool deleted = false);

		~Blob();

		bool Parse(const nlohmann::json::object_t &pt);
		std::string Serialize();

		inline bool operator==(int64_t id_) const
		{
			return id == id_;
		}
		inline bool operator==(std::string_view guid_) const
		{
			return guid == guid_;
		}
	};
}
