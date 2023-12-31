/**
 * Client.h - Contains WebSocket Media client impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2023
 */

#pragma once

#include <memory>
#include <string>
#include <cstdint>

namespace Transport
{

class WSMClientImpl;

class WSMClient
{
public:
    WSMClient();
    ~WSMClient();

    void SetServer(std::string_view address, uint16_t port, std::string_view access_token);

    uint16_t CreatePipe(); /// Return the local UDP port and start the component if needed

    void EndSession(); /// On end conference flush the work

    static constexpr bool WITH_TRACES = false;

private:
    std::unique_ptr <WSMClientImpl> impl;

};

}
