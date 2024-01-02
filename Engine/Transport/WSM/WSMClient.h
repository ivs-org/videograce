/**
 * WSMClient.h - Contains WebSocket Media client impl
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

    void Start(std::string_view address, std::string_view access_token);

    uint16_t CreatePipe(uint16_t serverUDPPort); /// Return the local UDP port who receives rtp packets and send it to web socket

    void Stop();

    static constexpr bool WITH_TRACES = false;

private:
    std::unique_ptr <WSMClientImpl> impl;

};

}
