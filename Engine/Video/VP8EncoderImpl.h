/**
 * VP8EncoderImpl.h - Contains header of VP8 encoder impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014, 2016
 */

#pragma once

#include <Video/IVideoEncoder.h>
#include <Transport/ISocket.h>
#include <Transport/RTP/RTPPacket.h>

#include <vpx/vpx_encoder.h>
#include <vpx/vp8cx.h>

#include <memory>
#include <cstddef>

namespace Video
{

class VP8EncoderImpl : public IEncoder, public Transport::ISocket
{
public:
	VP8EncoderImpl();
	virtual ~VP8EncoderImpl();

	void SetReceiver(Transport::ISocket *receiver);
		
	/// Derived from IEncoder
	virtual void SetResolution(Resolution resolution);
	virtual void SetBitrate(int32_t bitrate);
	virtual void SetScreenContent(bool yes);
	virtual int GetBitrate();
	virtual void Start(CodecType);
	virtual void Stop();
	virtual bool IsStarted();
	virtual void ForceKeyFrame(uint32_t);

	/// Derived from Transport::ISocket (input method)
	virtual void Send(const Transport::IPacket &packet, const Transport::Address *address = nullptr) final;

private:
	Transport::ISocket *receiver;

	bool runned;

	bool forceKF;

	Video::Resolution resolution;
	int32_t bitrate;

	bool screenContent;

	vpx_codec_ctx_t codec;
	vpx_codec_enc_cfg_t cfg;
	vpx_image_t raw;
	
	void EncodeFrame(vpx_image_t *img, const Transport::RTPPacket::RTPHeader&);
	void DieCodec(vpx_codec_ctx_t *ctx, const char *s);
};

}
