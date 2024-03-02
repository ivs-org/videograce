/**
 * VP8DecoderImpl.h - Contains header of VP8 decoder header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <memory>

#include <Video/IVideoDecoder.h>
#include <Transport/ISocket.h>
#include <Transport/RTP/RTPPacket.h>

#include <vpx/vpx_decoder.h>
#include <vpx/vp8dx.h>

namespace Video
{
	class VP8DecoderImpl : public IDecoder, public Transport::ISocket
	{
	public:
		VP8DecoderImpl();
		virtual ~VP8DecoderImpl();

		/// Derived from IDecoder
		virtual void SetReceiver(Transport::ISocket *receiver);
		virtual void SetCallback(IPacketLossCallback *callback);
		virtual bool SetResolution(Resolution resolution);
		virtual void Start(CodecType, Video::ColorSpace outputType);
		virtual void Stop();
		virtual bool IsStarted();

		/// Derived from Transport::ISocket (input method)
		virtual void Send(const Transport::IPacket &packet, const Transport::Address *address = nullptr) final;

	private:
		Transport::ISocket *receiver;
		IPacketLossCallback *callback;

		bool runned;

		bool keyFrameNeeded;
		uint32_t keyFrameForceTime;

		uint16_t lastFrameSeq;

		Video::Resolution resolution;

		uint32_t bufferSize, colorQuantizer;

		Video::ColorSpace outputType;

		std::unique_ptr<uint8_t[]> produceBuffer;

		vpx_codec_ctx_t      codec;
		vpx_codec_dec_cfg_t  cfg;

		void DecodeRGB32(const uint8_t *data, uint32_t length, const Transport::RTPPacket::RTPHeader&);
		void DecodeRGB24(const uint8_t *data, uint32_t length, const Transport::RTPPacket::RTPHeader&);
		void DecodeI420(const uint8_t *data, uint32_t length, const Transport::RTPPacket::RTPHeader&);

		void ForceKeyFrame(uint32_t seq, uint32_t ts);

		void DieCodec(vpx_codec_ctx_t *ctx, const char *s);
	};
}
