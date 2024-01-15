/**
 * CmdMedia.cpp - Contains command to transfer RTP over json via WebSocket impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2024
 */

#include <Proto/CmdMedia.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

namespace Proto
{
namespace MEDIA
{

static const std::string SSRC = "ssrc";
static const std::string MEDIA_TYPE = "mt";
static const std::string ADDR = "a";
static const std::string DATA = "d";

Command::Command()
    : ssrc(0),
    media_type(MediaType::Undefined),
    addr(),
    data()
{
}

Command::Command(MediaType media_type_, uint32_t ssrc_, std::string_view addr_, std::string_view data_)
    : media_type(media_type_), ssrc(ssrc_), addr(addr_), data(data_)
{
}

Command::~Command()
{
}

bool Command::Parse(std::string_view message)
{
    try
    {
        auto j = nlohmann::json::parse(message);
        auto obj = j.get<nlohmann::json::object_t>().at(NAME);

        if (obj.count(SSRC) != 0) ssrc = obj.at(SSRC).get<uint32_t>();
        if (obj.count(MEDIA_TYPE) != 0) media_type = static_cast<MediaType>(obj.at(MEDIA_TYPE).get<uint32_t>());
        if (obj.count(ADDR) != 0) addr = obj.at(ADDR).get<std::string>();
        if (obj.count(DATA) != 0) data = obj.at(DATA).get<std::string>();

        return true;
    }
    catch (nlohmann::json::parse_error& ex)
    {
        spdlog::get("Error")->critical("proto::{0} :: error parse json (byte: {1}, what: {2})", NAME, ex.byte, ex.what());
    }
	
	return false;
}

std::string Command::Serialize()
{
    return "{" + quot(NAME) + ":{" +
        quot(DATA) + ":" + quot(data) +
        (ssrc != 0 ? "," + quot(SSRC) + ":" + std::to_string(ssrc) : "") +
        (!addr.empty() ? "," + quot(ADDR) + ":" + quot(addr) : "") +
        (media_type != MediaType::Undefined ? "," + quot(MEDIA_TYPE) + ":" + std::to_string(static_cast<int>(media_type)) : "") + "}}";
}

}
}
