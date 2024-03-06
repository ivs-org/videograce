/**
 * VP8EncoderImpl.cpp - Contains impl of VP8 encoder impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014, 2016
 */

#include <Video/VP8EncoderImpl.h>

#include <Transport/RTP/RTPPacket.h>
#include <Transport/RTP/RTPPayloadType.h>

#include <stdlib.h>
#include <string.h>

#include <Common/Common.h>

namespace Video
{

static uint32_t MaxIntraTarget(uint32_t optimalBuffersize, uint32_t frameRate)
{
	// Set max to the optimal buffer level (normalized by target BR),
	// and scaled by a scalePar.
	// Max target size = scalePar * optimalBufferSize * targetBR[Kbps].
	// This values is presented in percentage of perFrameBw:
	// perFrameBw = targetBR[Kbps] * 1000 / frameRate.
	// The target in % is as follows:

	float scalePar = 0.5;
	uint32_t targetPct = (uint32_t)(optimalBuffersize * scalePar * frameRate / 10);

	// Don't go below 3 times the per frame bandwidth.
	const uint32_t minIntraTh = 300;
	return (targetPct < minIntraTh) ? minIntraTh : targetPct;
}

VP8EncoderImpl::VP8EncoderImpl()
	: receiver(nullptr),
	runned(false),
	forceKF(false),
	resolution(Video::rHD),
	bitrate(1024),
	screenContent(false),
	codec(),
	cfg(),
	raw()
{
}

VP8EncoderImpl::~VP8EncoderImpl()
{
	VP8EncoderImpl::Stop();
}

void VP8EncoderImpl::SetReceiver(Transport::ISocket *receiver_)
{
	receiver = receiver_;
}

void VP8EncoderImpl::SetResolution(Resolution resolution_)
{
	resolution = resolution_;
	if (runned)
	{
		Stop();
		Start(CodecType::VP8);
	}
}

void VP8EncoderImpl::SetBitrate(int bitrate_)
{
	bitrate = bitrate_;
	if (runned)
	{
		cfg.rc_target_bitrate = bitrate;
		vpx_codec_enc_config_set(&codec, &cfg);
	}
}

void VP8EncoderImpl::SetScreenContent(bool yes)
{
	screenContent = yes;
	if (runned)
	{
		vpx_codec_control(&codec, VP8E_SET_SCREEN_CONTENT_MODE, screenContent ? 2 : 0);
	}
}

int VP8EncoderImpl::GetBitrate()
{
	return bitrate;
}

void VP8EncoderImpl::Start(CodecType)
{
	if (runned)
	{
		return;
	}
	
	/* Populate encoder configuration */
	vpx_codec_err_t res = vpx_codec_enc_config_default(vpx_codec_vp8_cx(), &cfg, 0);
	if (res)
	{
		DBGTRACE("VP8EncoderImpl Failed to get config: %s\n", vpx_codec_err_to_string(res));
		return;
	}
	/* Update the default configuration with our settings */
	Video::ResolutionValues rv = Video::GetValues(resolution);
	cfg.rc_target_bitrate = bitrate;
	cfg.g_w = rv.width;
	cfg.g_h = rv.height;
	cfg.rc_end_usage = VPX_CBR;
	cfg.g_error_resilient = VPX_ERROR_RESILIENT_DEFAULT | VPX_ERROR_RESILIENT_PARTITIONS;
	cfg.g_timebase.den = 50;
	cfg.g_timebase.num = 1;
	
	if (rv.height >= 720)
	{
		cfg.g_threads = 3; // 3 threads for 1080p.
	}
	else if (rv.height >= 480)
	{
		cfg.g_threads = 2; // 2 threads for qHD/HD.
	}
	else
	{
		cfg.g_threads = 1; // 1 thread for less than VGA
	}

	cfg.rc_dropframe_thresh = 0;
	cfg.g_pass = VPX_RC_ONE_PASS;
	cfg.rc_undershoot_pct = 95;
	cfg.rc_overshoot_pct = 5;
	cfg.g_lag_in_frames = 0;
	cfg.rc_buf_initial_sz = 500;
	cfg.rc_buf_optimal_sz = 600;
	cfg.rc_buf_sz = 1000;
	cfg.kf_mode = VPX_KF_DISABLED;
	
	vpx_codec_flags_t flags = 0;
	
	// Initialize codec 
	if (vpx_codec_enc_init(&codec, vpx_codec_vp8_cx(), &cfg, flags) != VPX_CODEC_OK)
	{
		DieCodec(&codec, "VP8EncoderImpl Failed to initialize encoder");
	}

	vpx_codec_control(&codec, VP8E_SET_STATIC_THRESHOLD, 1);
	vpx_codec_control(&codec, VP8E_SET_TOKEN_PARTITIONS, VP8_ONE_TOKENPARTITION);
	vpx_codec_control(&codec, VP8E_SET_NOISE_SENSITIVITY, 4);
	vpx_codec_control(&codec, VP8E_SET_MAX_INTRA_BITRATE_PCT, MaxIntraTarget(cfg.rc_buf_optimal_sz, 40)); /// 40 ms - 25 frames per second
	vpx_codec_control(&codec, VP8E_SET_TUNING, VP8_TUNE_SSIM);
	vpx_codec_control(&codec, VP8E_SET_ENABLEAUTOALTREF, 1);
	vpx_codec_control(&codec, VP8E_SET_SCREEN_CONTENT_MODE, screenContent ? 2 : 0);

	if (!vpx_img_alloc(&raw, VPX_IMG_FMT_I420, cfg.g_w, cfg.g_h, 1))
	{
		DBGTRACE("Fail in allocate vpx image");
		return;
	}

	ForceKeyFrame(0);

	runned = true;
}

void VP8EncoderImpl::Stop()
{
	if (!runned)
	{
		return;
	}

	runned = false;

	if (vpx_codec_destroy(&codec))
	{
		DieCodec(&codec, "vp8 encoder failed to destroy codec");
	}

	vpx_img_free(&raw);
}

bool VP8EncoderImpl::IsStarted()
{
	return runned;
}

void VP8EncoderImpl::ForceKeyFrame(uint32_t)
{
	forceKF = true;
	//DBGTRACE("VP8EncoderImpl KeyFrameForced\n");
}

void VP8EncoderImpl::Send(const Transport::IPacket &packet_, const Transport::Address *)
{
	if (!runned)
	{
		return;
	}

	const Transport::RTPPacket &packet = *static_cast<const Transport::RTPPacket*>(&packet_);

	int chromaPlaneSize = cfg.g_w * cfg.g_h;
	int colorPlaneSize = (cfg.g_w / 2) * (cfg.g_h / 2);

	raw.planes[0] = const_cast<uint8_t*>(packet.payload);
	raw.planes[1] = const_cast<uint8_t*>(packet.payload + chromaPlaneSize);
	raw.planes[2] = const_cast<uint8_t*>(packet.payload + chromaPlaneSize + colorPlaneSize);

	EncodeFrame(&raw, packet.rtpHeader);
}

void VP8EncoderImpl::EncodeFrame(vpx_image_t *img, const Transport::RTPPacket::RTPHeader &header)
{
	int flags = 0;

	if (forceKF)
	{
		flags |= VPX_EFLAG_FORCE_KF;
		flags |= VP8_EFLAG_NO_REF_LAST;
		forceKF = false;
	}

	const vpx_codec_cx_pkt_t *pkt = NULL;
	const vpx_codec_err_t res = vpx_codec_encode(&codec, img, header.seq, 1, flags, VPX_DL_REALTIME);

	if (res != VPX_CODEC_OK)
	{
		DieCodec(&codec, "Failed to encode frame");
	}

	vpx_codec_iter_t iter = NULL;
	while ((pkt = vpx_codec_get_cx_data(&codec, &iter)) != NULL)
	{
		if (pkt->kind == VPX_CODEC_CX_FRAME_PKT)
		{
			//const int keyframe = (pkt->data.frame.flags & VPX_FRAME_IS_KEY) != 0;

			Transport::RTPPacket packet;
			packet.rtpHeader = header;
			packet.rtpHeader.pt = static_cast<uint8_t>(Transport::RTPPayloadType::ptVP8);
			packet.payload = (uint8_t*)pkt->data.frame.buf;
			packet.payloadSize = (uint32_t)pkt->data.frame.sz;

			receiver->Send(packet);
		}
	}
}

void VP8EncoderImpl::DieCodec(vpx_codec_ctx_t *ctx, const char *s)
{
	const char *detail = vpx_codec_error_detail(ctx);

	DBGTRACE("%s: %s\n", s, vpx_codec_error(ctx));
	if (detail)
	{
		DBGTRACE("%s\n", detail);
	}
}

}
