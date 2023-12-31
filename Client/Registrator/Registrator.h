/**
 * Registrator.h - Contains client registrator's header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#pragma once

#include <string>
#include <vector>

#include <Proto/Group.h>
#include <Proto/CmdUserUpdateResponse.h>

#include <Transport/HTTP/HttpClient.h>

#include <spdlog/spdlog.h>

namespace Registrator
{

class Registrator
{
public:
    Registrator();
    ~Registrator();

    void Connect(std::string_view url);
    void Disconnect();
    bool Connected();

    Proto::USER_UPDATE_RESPONSE::Result Register(std::string_view name, std::string_view avatar, std::string_view login, std::string_view password);

private:
    Transport::HTTPClient httpClient;

    std::shared_ptr<spdlog::logger> sysLog, errLog;

    void OnHTTPError(int32_t code, std::string_view msg);
};

}
