/**
 * OpusDecoderImpl.h - Contains header of Opus decoder header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <memory>

#include <Audio/IAudioDecoder.h>
#include <Transport/ISocket.h>
#include <Transport/RTP/RTPPacket.h>

#include <opus/opus.h>

namespace Audio
{
	class OpusDecoderImpl : public IDecoder, public Transport::ISocket
	{
	public:
		OpusDecoderImpl();
		virtual ~OpusDecoderImpl();

		void SetReceiver(Transport::ISocket *receiver);

		/// Derived from IDecoder
		virtual bool SetSampleFreq(int val);
		virtual bool SetChannelsCount(int channelsCount);
		virtual void Start(CodecType);
		virtual void Stop();
		virtual bool IsStarted();
		
		/// Derived from Transport::ISocket (input method)
		virtual void Send(const Transport::IPacket &packet, const Transport::Address *address = nullptr) final;

	private:
		Transport::ISocket *receiver;

		bool runned;

		int sample_freq, channels;

		uint32_t lastSeq;
		
		std::unique_ptr<uint8_t[]> produceBuffer;

		OpusDecoder *opusDecoder;
		
		void DecodeFrame(const uint8_t *data, uint32_t length, const Transport::RTPPacket::RTPHeader &);
	};
}
