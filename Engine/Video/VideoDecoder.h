/**
 * VideoDecoder.h - Contains video decoder impl interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <memory>

#include <Video/IVideoDecoder.h>
#include <Transport/ISocket.h>

namespace Video
{
	class Decoder : public IDecoder, public Transport::ISocket
	{
	public:
		Decoder();
		~Decoder();

		/// Derived from IDecoder
		virtual void SetReceiver(Transport::ISocket *receiver);
		virtual void SetCallback(IPacketLossCallback *callback);
		virtual bool SetResolution(Resolution resolution);
		virtual void Start(CodecType type, Video::ColorSpace outputType);
		virtual void Stop();
		virtual bool IsStarted();

		// Derived from Transport::ISocket (input method)
		virtual void Send(const Transport::IPacket &packet, const Transport::Address *address = nullptr) final;

	private:
		std::unique_ptr<IDecoder> impl;
		Transport::ISocket *receiver;
		IPacketLossCallback *callback;

		CodecType type;
		Video::ColorSpace outputType;
		Resolution resolution;
	};
}
