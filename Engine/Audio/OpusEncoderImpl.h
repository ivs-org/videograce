/**
 * OpusEncoderImpl.h - Contains header of opus encoder header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <memory>

#include <Audio/IAudioEncoder.h>
#include <Transport/ISocket.h>

#include <opus/opus.h>

namespace Audio
{
	class OpusEncoderImpl : public IEncoder, public Transport::ISocket
	{
	public:
		OpusEncoderImpl();
		virtual ~OpusEncoderImpl();

		void SetReceiver(Transport::ISocket *receiver);

		/// Derived from IEncoder
		virtual void SetQuality(int32_t val);
		virtual void SetBitrate(int32_t bitrate);
		virtual int32_t GetBitrate() const;
		virtual void Start(CodecType, uint32_t ssrc);
		virtual void Stop();
		virtual bool IsStarted() const;
		virtual void SetPacketLoss(int32_t val);

		/// Derived from Transport::ISocket (input method)
		virtual void Send(const Transport::IPacket &packet, const Transport::Address *address = nullptr) final;

	private:
		Transport::ISocket *receiver;

		bool runned;

		uint32_t ssrc;

		int32_t quality;
		int32_t bitrate;
		int32_t packetLoss;

        int32_t frameCount;

		std::unique_ptr<uint8_t[]> produceBuffer;

		OpusEncoder *opusEncoder;

		void EncodeFrame(const uint8_t* data, int len, uint32_t timestamp);
	};
}
