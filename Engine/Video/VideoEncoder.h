/**
 * VideoEncoder.h - Contains video encoder impl interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <memory>

#include <Video/IVideoEncoder.h>
#include <Transport/ISocket.h>

namespace Video
{
	class Encoder : public IEncoder, public Transport::ISocket
	{
	public:
		Encoder();
		~Encoder();

		/// Derived from IEncoder
		virtual void SetReceiver(Transport::ISocket *receiver);
		virtual void SetResolution(Resolution resolution);
		virtual void SetBitrate(int bitrate);
		virtual void SetScreenContent(bool yes);
		virtual int GetBitrate();
		virtual void Start(CodecType type, uint32_t ssrc);
		virtual void Stop();
		virtual bool IsStarted();
		virtual void ForceKeyFrame(uint32_t lastRecvSeq);

		/// Derived from Transport::ISocket (input method)
		virtual void Send(const Transport::IPacket &packet, const Transport::Address *address = nullptr) final;

	private:
		std::unique_ptr<IEncoder> impl;
		Transport::ISocket *receiver;

		CodecType type;
		Resolution resolution;
		int bitrate;

		uint32_t forceKFSeq;

		bool screenContent;
	};
}
