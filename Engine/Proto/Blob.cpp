/**
 * Blob.cpp - Contains blob structure impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2023
 */

#include <Proto/Blob.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <spdlog/spdlog.h>

namespace Proto
{

static const std::string ID = "id";
static const std::string OWNER_ID = "owner_id";
static const std::string GUID = "guid";
static const std::string ACTION = "action";
static const std::string STATUS = "status";
static const std::string DATA = "data";
static const std::string NAME = "name";
static const std::string DESCRIPTION = "description";
static const std::string DELETED = "deleted";

Blob::Blob()
	: id(0), owner_id(0), guid(), action(BlobAction::Undefined), status(BlobStatus::Undefined), data(), name(), description(), deleted(false)
{
}

Blob::Blob(int64_t id_, int64_t owner_id_, std::string_view guid_, BlobAction action_, BlobStatus status_, std::string_view data_, std::string_view name_, std::string_view description_, bool deleted_)
	: id(id_), owner_id(owner_id_), guid(guid_), action(action_), status(status_), data(data_), name(name_), description(description_), deleted(deleted_)
{
}

Blob::~Blob()
{
}

bool Blob::Parse(const nlohmann::json::object_t &obj)
{
	try
	{
		spdlog::get("System")->trace("proto::blob :: perform parsing");

		id = obj.at(ID).get<int64_t>();

		if (obj.count(OWNER_ID) != 0) owner_id = obj.at(OWNER_ID).get<int64_t>();
		if (obj.count(GUID) != 0) guid = obj.at(GUID).get<std::string>();
		if (obj.count(ACTION) != 0) action = static_cast<BlobAction>(obj.at(ACTION).get<uint32_t>());
		if (obj.count(STATUS) != 0) status = static_cast<BlobStatus>(obj.at(STATUS).get<uint32_t>());
		if (obj.count(DATA) != 0) data = obj.at(DATA).get<std::string>();
		if (obj.count(NAME) != 0) name = obj.at(NAME).get<std::string>();
		if (obj.count(DESCRIPTION) != 0) description = obj.at(DESCRIPTION).get<std::string>();
		if (obj.count(DELETED) != 0) deleted = obj.at(DELETED).get<uint8_t>();

		return true;
	}
	catch (nlohmann::json::parse_error& ex)
	{
		spdlog::get("Error")->critical("proto::blob :: error parse json (byte: {0}, what: {1})", ex.byte, ex.what());
	}
	return false;
}

std::string Blob::Serialize()
{
	return "{" + quot(ID) + ":" + std::to_string(id) +
		(owner_id != -1 ? "," + quot(OWNER_ID) + ":" + std::to_string(owner_id) : "") +
		(!guid.empty() ? "," + quot(GUID) + ":" + quot(Common::JSON::Screen(guid)) : "") +
		(action != BlobAction::Undefined ? "," + quot(ACTION) + ":" + std::to_string(static_cast<uint32_t>(action)) : "") +
		(status != BlobStatus::Undefined ? "," + quot(STATUS) + ":" + std::to_string(static_cast<uint32_t>(status)) : "") +
		(!data.empty() ? "," + quot(DATA) + ":" + quot(Common::JSON::Screen(data)) : "") +
		(!name.empty() ? "," + quot(NAME) + ":" + quot(Common::JSON::Screen(name)) : "") +
		(!description.empty() ? "," + quot(DESCRIPTION) + ":" + quot(Common::JSON::Screen(description)) : "") +
		(deleted ? "," + quot(DELETED) + ":1" : "") + "}";
}

}
