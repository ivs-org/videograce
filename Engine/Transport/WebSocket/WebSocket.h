/**
 * WebSocket.h - Contains WebSocket's header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#pragma once

#include <memory>
#include <mutex>
#include "IWebSocket.h"

namespace Transport
{
	class WebSocketImpl;

	class WebSocket : public IWebSocket
	{
	public:
		WebSocket(IWebSocketCallback &callback);
		virtual ~WebSocket();

		virtual void Connect(const std::string &url);
		virtual void Send(const std::string &message);
		virtual void Disconnect();

		virtual bool IsConnected();
		
	private:
		std::recursive_mutex implMutex;
		std::unique_ptr<WebSocketImpl> impl;
		IWebSocketCallback &callback;
	};
}
