/**
 * Registrator.cpp - Contains client registrator's impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#include <Registrator/Registrator.h>

namespace Registrator
{

Registrator::Registrator()
    : httpClient(std::bind(&Registrator::OnHTTPError, this, std::placeholders::_1, std::placeholders::_2)),
    sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{
}

Registrator::~Registrator()
{
    Disconnect();
}

void Registrator::Connect(std::string_view url)
{
    httpClient.Connect(url);
}

void Registrator::Disconnect()
{
    httpClient.Disconnect();
}

bool Registrator::Connected()
{
    return httpClient.IsConnected();
}

Proto::USER_UPDATE_RESPONSE::Result Registrator::Register(std::string_view name, std::string_view avatar, std::string_view login, std::string_view password)
{
    auto registerResult = Proto::USER_UPDATE_RESPONSE::Result::Undefined;

    /*if (webSocket.IsConnected())
    {
        webSocket.Send(Proto::USER_UPDATE_REQUEST::Command(Proto::USER_UPDATE_REQUEST::Action::Register,
            0,
            name,
            avatar,
            login,
            password).Serialize());

        readySem.wait_for(2000);
    }*/

    return registerResult;
}

void Registrator::OnHTTPError(int32_t code, std::string_view)
{
    /*switch (method)
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
    }*/
}

}
