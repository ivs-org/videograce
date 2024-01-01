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

static const std::string SRC_PORT = "src";
static const std::string DST_PORT = "dst";
static const std::string RTP = "rtp";

Command::Command()
    : src_port(0),
    dst_port(0),
    rtp()
{
}

Command::Command(uint16_t src_port_, uint16_t dst_port_, std::string_view rtp_)
    : src_port(src_port_), dst_port(dst_port_), rtp(rtp_)
{
}

Command::~Command()
{
}

bool Command::Parse(std::string_view message)
{
    try
    {
        spdlog::get("System")->trace("proto::{0} :: perform parsing", NAME);

        auto j = nlohmann::json::parse(message);
        auto obj = j.get<nlohmann::json::object_t>().at(NAME);

        if (obj.count(SRC_PORT) != 0) src_port = obj.at(SRC_PORT).get<uint16_t>();
        if (obj.count(DST_PORT) != 0) dst_port = obj.at(DST_PORT).get<uint16_t>();
        if (obj.count(RTP) != 0) rtp = obj.at(RTP).get<std::string>();

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
        quot(RTP) + ":" + quot(rtp) +
        (src_port != 0 ? "," + quot(SRC_PORT) + ":" + std::to_string(src_port) : "") +
        (dst_port != 0 ? "," + quot(DST_PORT) + ":" + std::to_string(dst_port) : "") + "}}";
}

}
}
