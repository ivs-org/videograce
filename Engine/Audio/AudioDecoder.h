/**
 * AudioDecoder.h - Contains audio decoder impl interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <memory>

#include <Audio/IAudioDecoder.h>
#include <Transport/ISocket.h>

namespace Audio
{
	class Decoder : public IDecoder, public Transport::ISocket
	{
	public:
		Decoder();
		~Decoder();

		/// Derived from IDecoder
		virtual void SetReceiver(Transport::ISocket *receiver);
		virtual bool SetSampleFreq(int val);
		virtual bool SetChannelsCount(int channelsCount);
		virtual void Start(CodecType type);
		virtual void Stop();
		virtual bool IsStarted();
		virtual void Decode(const Transport::IPacket& in, Transport::IPacket& out);
		
		// Derived from Transport::ISocket (input method)
		virtual void Send(const Transport::IPacket &packet, const Transport::Address *address = nullptr) final;

	private:
		std::unique_ptr<IDecoder> impl;
		Transport::ISocket *receiver;

		CodecType type;

		int sampleFreq;
		int channels;
	};
}
