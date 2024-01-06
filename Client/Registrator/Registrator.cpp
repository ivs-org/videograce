/**
 * Registrator.cpp - Contains client registrator's impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#include <Registrator/Registrator.h>
#include <API/RegisterUser.h>

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
    if (httpClient.IsConnected())
    {
        auto result = httpClient.Request("/api/v1.0/register_user", "POST", API::REGISTER_USER::Command(name,
            login,
            password,
            "").Serialize());

        sysLog->trace("Registrator::Register (result: {0})", result);

        if (result.find("OK") != std::string::npos)
        {
            return Proto::USER_UPDATE_RESPONSE::Result::OK;
        }
        else if (result.find("duplicated") != std::string::npos)
        {
            return Proto::USER_UPDATE_RESPONSE::Result::DuplicateLogin;
        }
        else if (result.find("Forbidden") != std::string::npos)
        {
            return Proto::USER_UPDATE_RESPONSE::Result::RegistrationDenied;
        }
    }

    return Proto::USER_UPDATE_RESPONSE::Result::Undefined;
}

void Registrator::OnHTTPError(int32_t code, std::string_view message)
{
    errLog->critical("Registrator :: HTTP Error (code: {0}, msg: {1})", code, message);
}

}
