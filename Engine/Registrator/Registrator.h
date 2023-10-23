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

class Registrator : public Transport::IWebSocketCallback
{
public:
    Registrator();
    ~Registrator();

    void Connect(const std::string &url);
    void Disconnect();
    bool Connected();

    Proto::USER_UPDATE_RESPONSE::Result Register(const std::string &name, const std::string &avatar, const std::string &login, const std::string &password);

    void GetCredentials(const std::string &guid, bool &ok, std::string &login, std::string &password);

private:
    Transport::WebSocket webSocket;
    mt::semaphore readySem;

    Proto::USER_UPDATE_RESPONSE::Result registerResult;

    Proto::CREDENTIALS_RESPONSE::Command credentialsResponse;

    std::shared_ptr<spdlog::logger> sysLog, errLog;

    virtual void OnWebSocket(Transport::WSMethod method, const std::string &message);
};

}
