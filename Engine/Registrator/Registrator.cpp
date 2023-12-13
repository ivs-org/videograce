/**
 * Registrator.cpp - Contains client registrator's impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#include <Registrator/Registrator.h>

#include <Proto/CommandType.h>
#include <Proto/CmdGroupList.h>
#include <Proto/CmdUserUpdateRequest.h>
#include <Proto/CmdCredentialsRequest.h>
#include <Proto/CmdPing.h>

namespace Registrator
{

Registrator::Registrator()
    : webSocket(*this),
    readySem(),
    registerResult(Proto::USER_UPDATE_RESPONSE::Result::Undefined),
    credentialsResponse(),
    sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{
}

Registrator::~Registrator()
{
    Disconnect();
}

void Registrator::Connect(const std::string &url)
{
    webSocket.Connect(url);
    readySem.wait_for(2000);
}

void Registrator::Disconnect()
{
    webSocket.Disconnect();
}

bool Registrator::Connected()
{
    return webSocket.IsConnected();
}

Proto::USER_UPDATE_RESPONSE::Result Registrator::Register(const std::string &name, const std::string &avatar, const std::string &login, const std::string &password)
{
    registerResult = Proto::USER_UPDATE_RESPONSE::Result::Undefined;

    if (webSocket.IsConnected())
    {
        webSocket.Send(Proto::USER_UPDATE_REQUEST::Command(Proto::USER_UPDATE_REQUEST::Action::Register,
            0,
            name,
            avatar,
            login,
            password).Serialize());

        readySem.wait_for(2000);
    }

    return registerResult;
}

void Registrator::GetCredentials(const std::string &guid, bool &ok, std::string &login, std::string &password)
{
    if (webSocket.IsConnected())
    {
        webSocket.Send(Proto::CREDENTIALS_REQUEST::Command(guid).Serialize());

        readySem.wait_for(2000);

        ok = credentialsResponse.result == Proto::CREDENTIALS_RESPONSE::Result::OK;
        login = credentialsResponse.login;
        password = credentialsResponse.password;
    }
}

void Registrator::OnWebSocket(Transport::WSMethod method, const std::string &message)
{
    switch (method)
    {
        case Transport::WSMethod::Open:
            sysLog->info("Registrator's connection to server established");
        break;
        case Transport::WSMethod::Message:
        {
            Proto::CommandType commandType = Proto::GetCommandType(message);

            switch (commandType)
            {
                case Proto::CommandType::UserUpdateResponse:
                {
                    Proto::USER_UPDATE_RESPONSE::Command cmd;
                    cmd.Parse(message);

                    registerResult = cmd.result;
                }
                break;
                case Proto::CommandType::CredentialsResponse:
                    credentialsResponse.Parse(message);
                break;
            }
        }
        break;
        case Transport::WSMethod::Close:
            sysLog->info("Registrator's connection to server closed");
        break;
        case Transport::WSMethod::Error:
            errLog->critical("Registrator's WebSocket error {0}", message);
        break;
    }

    readySem.notify();
}

}
