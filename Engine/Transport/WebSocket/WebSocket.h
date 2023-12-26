/**
 * WebSocket.h - Contains WebSocket's header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016, 2023
 */

#pragma once

#include <memory>
#include <string_view>
#include <functional>

namespace Transport
{

enum class WSMethod
{
    Open,
    Message,
    Close,
    Error
};

using ws_callback = std::function<void(WSMethod method, std::string_view message)>;

class WebSocketImpl;

class WebSocket
{
public:
    WebSocket(ws_callback callback);
    ~WebSocket();

    void Connect(std::string_view url);
    void Send(std::string_view message);
    void SendBinary(const uint8_t *data, size_t size);
    void Disconnect();

    bool IsConnected();
        
private:
    std::unique_ptr<WebSocketImpl> impl;
    ws_callback callback;
};

}
