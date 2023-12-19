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
#include <Proto/CmdCredentialsResponse.h>

#include <mt/semaphore.h>

#include <Transport/WebSocket/WebSocket.h>

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

    void GetCredentials(std::string_view guid, bool &ok, std::string &login, std::string &password);

private:
    Transport::WebSocket webSocket;
    mt::semaphore readySem;

    Proto::USER_UPDATE_RESPONSE::Result registerResult;

    Proto::CREDENTIALS_RESPONSE::Command credentialsResponse;

    std::shared_ptr<spdlog::logger> sysLog, errLog;

    void OnWebSocket(Transport::WSMethod method, std::string_view message);
};

}
