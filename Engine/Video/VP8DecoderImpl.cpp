/**
 * VP8DecoderImpl.cpp - Contains impl of VP8 decoder impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#include <Video/VP8DecoderImpl.h>

#include <Transport/RTP/RTPPacket.h>

#include <cstdlib>
#include <cstring>
#include <thread>

#include <Common/Common.h>

#include <new>
#include <ippcc.h>
#include <ippi.h>

namespace Video
{

VP8DecoderImpl::VP8DecoderImpl()
	: receiver(nullptr),
	callback(nullptr),
	runned(false),
	keyFrameNeeded(true),
	keyFrameForceTime(0),
	lastFrameSeq(0),
	resolution(Video::rVGA),
	bufferSize(0), colorQuantizer(2),
	outputType(Video::ColorSpace::RGB24),
	produceBuffer(),
	codec(),
	cfg()
{
}

VP8DecoderImpl::~VP8DecoderImpl()
{
	VP8DecoderImpl::Stop();
}

void VP8DecoderImpl::SetReceiver(Transport::ISocket *receiver_)
{
	receiver = receiver_;
}

void VP8DecoderImpl::SetCallback(IPacketLossCallback *callback_)
{
	callback = callback_;
}

bool VP8DecoderImpl::SetResolution(Resolution resolution_)
{
	resolution = resolution_;
	if (runned)
	{
		Stop();
		Start(Video::CodecType::VP8, outputType);
	}
	return true;
}

void VP8DecoderImpl::Start(CodecType, Video::ColorSpace outputType_)
{
	if (runned)
	{
		return;
	}

	outputType = outputType_;
	
	/* Update the default configuration with our settings */
	Video::ResolutionValues rv = Video::GetValues(resolution);
	cfg.w = rv.width;
	cfg.h = rv.height;

	if (rv.height >= 720)
	{
		cfg.threads = 3; // 3 threads for 1080p.
	}
	else if (rv.height >= 480)
	{
		cfg.threads = 2; // 2 threads for qHD/HD.
	}
	else
	{
		cfg.threads = 1; // 1 thread for less than VGA
	}

	int flags = 0;
	flags |= VPX_CODEC_USE_POSTPROC | VPX_CODEC_USE_ERROR_CONCEALMENT;

	/* Initialize codec */
	if (vpx_codec_dec_init(&codec, vpx_codec_vp8_dx(), &cfg, flags))
	{
		DieCodec(&codec, "Failed to initialize decoder");
	}

	switch (outputType)
	{
		case Video::ColorSpace::I420:
			colorQuantizer = 2;
		break;
		case Video::ColorSpace::RGB24:
			colorQuantizer = 3;
		break;
		case Video::ColorSpace::RGB32:
			colorQuantizer = 4;
		break;
		default:
		break;
	}

	bufferSize = rv.width * rv.height * colorQuantizer;

	try
	{
		produceBuffer = std::unique_ptr<uint8_t[]>(new uint8_t[bufferSize]);
	}
	catch (std::bad_alloc &e)
	{
		return;
	}

	lastFrameSeq = 0;
	keyFrameForceTime = 0;

	keyFrameNeeded = true;
	runned = true;
}

void VP8DecoderImpl::Stop()
{
	if (!runned)
	{
		return;
	}
	runned = false;
	
	if (vpx_codec_destroy(&codec))
	{
		DieCodec(&codec, "vp8 decoder failed to destroy codec");
	}

	produceBuffer.reset(nullptr);
}

bool VP8DecoderImpl::IsStarted()
{
	return runned;
}

void VP8DecoderImpl::DecodeRGB32(const uint8_t *data, uint32_t length, const Transport::RTPPacket::RTPHeader& header)
{
	if (vpx_codec_decode(&codec, data, length, NULL, 0) != VPX_CODEC_OK)
	{
		DieCodec(&codec, "vp8 decoder failed to decode frame");
		return;
	}

	vpx_image_t *img = NULL;
	vpx_codec_iter_t iter = NULL;

	while ((img = vpx_codec_get_frame(&codec, &iter)))
	{
		if (bufferSize < img->d_w * img->d_h * colorQuantizer)
		{
			continue;
		}
		const IppiSize  sz = { (int)img->d_w, (int)img->d_h };
		const Ipp8u*    src[3] = { img->planes[VPX_PLANE_Y], img->planes[VPX_PLANE_U], img->planes[VPX_PLANE_V] };
		int             srcStep[3] = { img->stride[VPX_PLANE_Y], img->stride[VPX_PLANE_U], img->stride[VPX_PLANE_V] };

		ippiYCbCr420ToBGR_8u_P3C4R(src, srcStep, produceBuffer.get(), img->d_w * 4, sz, 255);
		
		Transport::RTPPacket packet;
		packet.rtpHeader = header;
		packet.payload = produceBuffer.get();
		packet.payloadSize = img->d_w * img->d_h * 4;
		receiver->Send(packet);
	}
}

void VP8DecoderImpl::DecodeRGB24(const uint8_t *data, uint32_t length, const Transport::RTPPacket::RTPHeader& header)
{
	if (vpx_codec_decode(&codec, data, length, NULL, 0) != VPX_CODEC_OK)
	{
		DieCodec(&codec, "vp8 decoder failed to decode frame");
		return;
	}

	vpx_image_t *img = NULL;
	vpx_codec_iter_t iter = NULL;

	while ((img = vpx_codec_get_frame(&codec, &iter)))
	{
		if (bufferSize < img->d_w * img->d_h * colorQuantizer)
		{
			continue;
		}

		const IppiSize  sz = { (int)img->d_w, (int)img->d_h };
		const Ipp8u*    src[3] = { img->planes[VPX_PLANE_Y], img->planes[VPX_PLANE_U], img->planes[VPX_PLANE_V] };
		int             srcStep[3] = { img->stride[VPX_PLANE_Y], img->stride[VPX_PLANE_U], img->stride[VPX_PLANE_V] };

		ippiYCbCr420ToRGB_8u_P3C3R(src, srcStep, produceBuffer.get(), img->d_w * 3, sz);
		
		Transport::RTPPacket packet;
		packet.rtpHeader = header;
		packet.payload = produceBuffer.get();
		packet.payloadSize = img->d_w * img->d_h * 3;
		receiver->Send(packet);
	}
}

void VP8DecoderImpl::DecodeI420(const uint8_t *data, uint32_t length, const Transport::RTPPacket::RTPHeader& header)
{
	if (vpx_codec_decode(&codec, data, length, NULL, 0) != VPX_CODEC_OK)
	{
		DieCodec(&codec, "vp8 decoder failed to decode frame");
		return;
	}

	vpx_image_t *img = NULL;
	vpx_codec_iter_t iter = NULL;

	while ((img = vpx_codec_get_frame(&codec, &iter)))
	{
		if (bufferSize < img->d_w * img->d_h * colorQuantizer)
		{
			continue;
		}

		int outframeSize = 0;

		for (unsigned int plane = 0; plane < 3; plane++)
		{
			unsigned char *buf = img->planes[plane];

			for (unsigned int y = 0; y < (plane ? (img->d_h + 1) >> 1 : img->d_h); y++)
			{
				int len = (plane ? (img->d_w + 1) >> 1 : img->d_w);

				memcpy(produceBuffer.get() + outframeSize, buf, len);

				outframeSize += len;
				buf += img->stride[plane];
			}
		}

		Transport::RTPPacket packet;
		packet.rtpHeader = header;
		packet.payload = produceBuffer.get();
		packet.payloadSize = outframeSize;
		receiver->Send(packet);
	}
}

inline bool IsKeyFrame(const uint8_t *c)
{
	return !(((c[2] << 16) | (c[1] << 8) | c[0]) & 0x1);
}

void VP8DecoderImpl::ForceKeyFrame(uint32_t seq, uint32_t ts)
{
	if (keyFrameForceTime + 200 <= ts) /// Prevent key frame force flood (no more than once every 200 ms)
	{
		callback->ForceKeyFrame(seq);
		keyFrameForceTime = ts;
	}
}

void VP8DecoderImpl::Send(const Transport::IPacket &packet_, const Transport::Address *)
{
	if (!runned)
	{
		return;
	}

	const Transport::RTPPacket &packet = *static_cast<const Transport::RTPPacket*>(&packet_);

	if (lastFrameSeq != 0 && packet.rtpHeader.seq != 0 && lastFrameSeq + 1 != packet.rtpHeader.seq)
	{
		keyFrameNeeded = true;
	}
	lastFrameSeq = packet.rtpHeader.seq;

	if (keyFrameNeeded)
	{
		if (packet.payloadSize != 0 && IsKeyFrame(packet.payload))
		{
			keyFrameNeeded = false;
		}
		else
		{
			return ForceKeyFrame(lastFrameSeq, packet.rtpHeader.ts);
		}
	}

	switch (outputType)
	{
		case Video::ColorSpace::I420:
			DecodeI420(packet.payload, packet.payloadSize, packet.rtpHeader);
		break;
		case Video::ColorSpace::RGB24:
			DecodeRGB24(packet.payload, packet.payloadSize, packet.rtpHeader);
		break;
		case Video::ColorSpace::RGB32:
			DecodeRGB32(packet.payload, packet.payloadSize, packet.rtpHeader);
		break;
		default:
		break;
	}
}

void VP8DecoderImpl::DieCodec(vpx_codec_ctx_t *ctx, const char *s)
{
	callback->ForceKeyFrame(0);

#ifdef DEBUG
	const char *detail = vpx_codec_error_detail(ctx);

	DBGTRACE("%s: %s\n", s, vpx_codec_error(ctx));
	if (detail)
	{
		DBGTRACE(" 4в  %s\n", detail);
	}
#endif
}

}
