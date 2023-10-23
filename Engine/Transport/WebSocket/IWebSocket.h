/**
 * IWebSocket.h - Contains websocket's interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#pragma once

#include <string>

namespace Transport
{
	enum class WSMethod
	{
		Open,
		Message,
		Close,
		Error
	};

	class IWebSocketCallback
	{
	public:
		virtual void OnWebSocket(WSMethod method, const std::string &message) = 0;
	protected:
		~IWebSocketCallback() {}
	};

	class IWebSocket
	{
	public:
		virtual void Connect(const std::string &url) = 0;
		virtual void Send(const std::string &message) = 0;
		virtual void Disconnect() = 0;

		virtual bool IsConnected() = 0;
		
	protected:
		virtual ~IWebSocket() {}
	};
}
