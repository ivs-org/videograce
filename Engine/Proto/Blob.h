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
	enum class BlobType
	{
		Undefined = 0,

		Image,
		Document,
		Voice,
		CircleVideo

	};

	enum class BlobStatus
	{
		Undefined = 0,

		NotFound,

		Created,
		Received,
		Modified,

		Deleted
	};

	enum class BlobAction
	{
		Undefined = 0,

		SpeedTest,

		Storage,
		P2P
	};

	struct Blob
	{
		int64_t id;
		int64_t owner_id;
		std::string guid;

		BlobType type;
		BlobStatus status;
		BlobAction action;

		std::string data;
		std::string preview;
		std::string name;
		std::string description;

		bool deleted;

		Blob();
		Blob(int64_t id,
			int64_t owner_id,
			std::string_view guid,
			BlobType type,
			BlobStatus status,
			BlobAction action,
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
