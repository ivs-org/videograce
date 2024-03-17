/**
 * AEC.cpp - Contains acoustic echo canceller impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016 - 2018
 */

#include <AEC/AEC.h>

#include <Common/Common.h>

#include <Transport/RTP/RTPPacket.h>

#include "webrtc/modules/audio_processing/aec/echo_cancellation.h"
#include "webrtc/modules/audio_processing/splitting_filter.h"
#include "webrtc/common_audio/channel_buffer.h"
extern "C" {
#include "webrtc/modules/audio_processing/aec/aec_core.h" 
}
#include "webrtc/modules/audio_processing/ns/noise_suppression_x.h"
#include "webrtc/common_audio/signal_processing/include/signal_processing_library.h"
#include "webrtc/modules/audio_processing/agc/legacy/gain_control.h"

namespace AEC
{

enum
{
	SAMPLING_FREQ = 48000,

	FRAMES_COUNT = 4, /// Two 10 ms frames in one input / output packet
	BANDS_COUNT = 3,  /// Three bands of 16 kHz (16 + 16 + 16 = 48)
	BAND_SIZE = 160   /// One 16KHz band has 160 samples (duration 10 ms)
};

/// Wrappers

AEC::MicrophoneReceiver::MicrophoneReceiver(AEC &aec_)
	: aec(aec_),
	runned(false),
	mutex(),
	splittingFilter(),
	splittingFilterIn(),
	splittingFilterOut()
{}

void AEC::MicrophoneReceiver::Send(const Transport::IPacket &packet_, const Transport::Address *)
{
	std::lock_guard<std::mutex> lock(mutex);

	if (!runned)
	{
		return;
	}

	if (!aec.nsEnabled && !aec.aecEnabled && !aec.agcEnabled)
	{
		return aec.resultReceiver->Send(packet_);
	}

	const Transport::RTPPacket &inPacket = *static_cast<const Transport::RTPPacket*>(&packet_);
		
	webrtc::IFChannelBuffer* bufIn = splittingFilterIn.get();
	webrtc::IFChannelBuffer* bufOut = splittingFilterOut.get();

	memcpy(bufIn->ibuf()->bands(0)[0], inPacket.payload, inPacket.payloadSize);

	splittingFilter->Analysis(bufIn, bufOut);
	
	if (aec.nsEnabled)
	{
		int16_t _nsOut[BANDS_COUNT][BAND_SIZE * FRAMES_COUNT];
		int16_t* nsIn[BANDS_COUNT];
		int16_t* nsOut[BANDS_COUNT];
		for (int i = 0; i != FRAMES_COUNT; ++i)
		{
			for (int j = 0; j != BANDS_COUNT; ++j)
			{
				nsIn[j] = (int16_t*)bufOut->ibuf_const()->bands(0)[j] + (BAND_SIZE * i);
				nsOut[j] = _nsOut[j] + (BAND_SIZE * i);
			}
			WebRtcNsx_Process((NsxHandle*)aec.nsInst, static_cast<const short *const *>(nsIn), BANDS_COUNT, nsOut);
		}
		
		memcpy(bufOut->ibuf()->bands(0)[0], _nsOut[0], BAND_SIZE * FRAMES_COUNT * sizeof(int16_t));
		memcpy(bufOut->ibuf()->bands(0)[1], _nsOut[1], BAND_SIZE * FRAMES_COUNT * sizeof(int16_t));
		memcpy(bufOut->ibuf()->bands(0)[2], _nsOut[2], BAND_SIZE * FRAMES_COUNT * sizeof(int16_t));
	}

	if (aec.aecEnabled)
	{
		const float* aecIn[BANDS_COUNT];
		float* aecOut[BANDS_COUNT];
		float _aecOut[BANDS_COUNT][BAND_SIZE * FRAMES_COUNT];

		for (int i = 0; i != FRAMES_COUNT; ++i)
		{
			for (int j = 0; j != BANDS_COUNT; ++j)
			{
				aecIn[j] = bufOut->fbuf_const()->bands(0)[j] + (BAND_SIZE * i);
				aecOut[j] = _aecOut[j] + (BAND_SIZE * i);
			}
			webrtc::WebRtcAec_Process(aec.aecInst, aecIn, BANDS_COUNT, aecOut, BAND_SIZE, aec.renderLatency, 0);
		}
				
		memcpy(bufOut->fbuf()->bands(0)[0], _aecOut[0], BAND_SIZE * FRAMES_COUNT * sizeof(float));
		memcpy(bufOut->fbuf()->bands(0)[1], _aecOut[1], BAND_SIZE * FRAMES_COUNT * sizeof(float));
		memcpy(bufOut->fbuf()->bands(0)[2], _aecOut[2], BAND_SIZE * FRAMES_COUNT * sizeof(float));
	}

	if (aec.agcEnabled)
	{
		int16_t _agcOut[BANDS_COUNT][BAND_SIZE * FRAMES_COUNT];
		int16_t* agcIn[BANDS_COUNT];
		int16_t* agcOut[BANDS_COUNT];
		uint8_t saturation = 0;
		int32_t outMicLevel = 0;

		for (int i = 0; i != FRAMES_COUNT; ++i)
		{
			for (int j = 0; j != BANDS_COUNT; ++j)
			{
				agcIn[j] = (int16_t*)bufOut->ibuf_const()->bands(0)[j] + (BAND_SIZE * i);
				agcOut[j] = _agcOut[j] + (BAND_SIZE * i);
			}
			WebRtcAgc_AddMic(aec.agcInst, agcIn, BANDS_COUNT, BAND_SIZE);
			WebRtcAgc_Process(aec.agcInst, static_cast<const short *const *>(agcIn), BANDS_COUNT, BAND_SIZE, agcOut, aec.micLevel, &outMicLevel, 0, &saturation);
		}

		memcpy(bufOut->ibuf()->bands(0)[0], _agcOut[0], BAND_SIZE * FRAMES_COUNT * sizeof(int16_t));
		memcpy(bufOut->ibuf()->bands(0)[1], _agcOut[1], BAND_SIZE * FRAMES_COUNT * sizeof(int16_t));
		memcpy(bufOut->ibuf()->bands(0)[2], _agcOut[2], BAND_SIZE * FRAMES_COUNT * sizeof(int16_t));
	}

	splittingFilter->Synthesis(bufOut, bufIn);
		
	Transport::RTPPacket packet;
	packet.rtpHeader = inPacket.rtpHeader;
	packet.payload = (uint8_t*)bufIn->ibuf_const()->bands(0)[0];
	packet.payloadSize = inPacket.payloadSize;
	aec.resultReceiver->Send(packet);
}

void AEC::MicrophoneReceiver::Start()
{
	std::lock_guard<std::mutex> lock(mutex);

	if (!runned)
	{
		runned = true;

		splittingFilter = std::unique_ptr<webrtc::SplittingFilter>(new webrtc::SplittingFilter(1, BANDS_COUNT, FRAMES_COUNT * BANDS_COUNT * BAND_SIZE));
		splittingFilterIn = std::unique_ptr<webrtc::IFChannelBuffer>(new webrtc::IFChannelBuffer(FRAMES_COUNT * BANDS_COUNT * BAND_SIZE, 1, BANDS_COUNT));
		splittingFilterOut = std::unique_ptr<webrtc::IFChannelBuffer>(new webrtc::IFChannelBuffer(FRAMES_COUNT * BANDS_COUNT * BAND_SIZE, 1, BANDS_COUNT));
	}
}

void AEC::MicrophoneReceiver::Stop()
{
	std::lock_guard<std::mutex> lock(mutex);

	runned = false;

	splittingFilter.reset(nullptr);
	splittingFilterIn.reset(nullptr);
	splittingFilterOut.reset(nullptr);
}

AEC::SpeakerReceiver::SpeakerReceiver(AEC &aec_)
	: aec(aec_),
	runned(false),
	mutex(),
	splittingFilter(),
	splittingFilterIn(),
	splittingFilterOut()
{}

void AEC::SpeakerReceiver::Send(const Transport::IPacket &packet_, const Transport::Address *)
{
	std::lock_guard<std::mutex> lock(mutex);

	if (runned && aec.aecEnabled)
	{
		const auto &packet = *static_cast<const Transport::RTPPacket*>(&packet_);

		memcpy(splittingFilterIn->ibuf()->bands(0)[0], packet.payload, packet.payloadSize);
		splittingFilter->Analysis(splittingFilterIn.get(), splittingFilterOut.get());

		for (int i = 0; i != FRAMES_COUNT; ++i)
		{
			webrtc::WebRtcAec_BufferFarend(aec.aecInst, splittingFilterOut->fbuf_const()->bands(0)[0] + (BAND_SIZE * i), BAND_SIZE);
		}
	}
}

void AEC::SpeakerReceiver::Start()
{
	std::lock_guard<std::mutex> lock(mutex);

	if (!runned)
	{
		runned = true;

		splittingFilter = std::unique_ptr<webrtc::SplittingFilter>(new webrtc::SplittingFilter(1, BANDS_COUNT, FRAMES_COUNT * BANDS_COUNT * BAND_SIZE));
		splittingFilterIn = std::unique_ptr<webrtc::IFChannelBuffer>(new webrtc::IFChannelBuffer(FRAMES_COUNT * BANDS_COUNT * BAND_SIZE, 1, BANDS_COUNT));
		splittingFilterOut = std::unique_ptr<webrtc::IFChannelBuffer>(new webrtc::IFChannelBuffer(FRAMES_COUNT * BANDS_COUNT * BAND_SIZE, 1, BANDS_COUNT));
	}
}

void AEC::SpeakerReceiver::Stop()
{
	std::lock_guard<std::mutex> lock(mutex);

	runned = false;

	splittingFilter.reset(nullptr);
	splittingFilterIn.reset(nullptr);
	splittingFilterOut.reset(nullptr);
}

/// Container

AEC::AEC()
	: runned(false), aecEnabled(true), nsEnabled(true), agcEnabled(true),
	micLevel(100),
	renderLatency(100),
	microphoneSource(*this), speakerSource(*this),
	aecInst(nullptr),
	nsInst(nullptr),
	agcInst(nullptr),
	resultReceiver(nullptr),
	sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{
}

AEC::~AEC()
{
	AEC::Stop();
}

void AEC::SetReceiver(Transport::ISocket *receiver_)
{
	resultReceiver = receiver_;
}

Transport::ISocket *AEC::GetMicrophoneReceiver()
{
	return &microphoneSource;
}

Transport::ISocket *AEC::GetSpeakerReceiver()
{
	return &speakerSource;
}

void AEC::Start()
{
	if (!runned)
	{
		runned = true;

		/// Creating AEC
		aecInst = webrtc::WebRtcAec_Create();
		if (aecInst == nullptr)
		{
			errLog->error("AEC :: WebRtcAec_Create() fail");
		}
		
		int32_t ret = webrtc::WebRtcAec_Init(aecInst, SAMPLING_FREQ, SAMPLING_FREQ);
		if (ret != 0)
		{
			errLog->error("AEC :: WebRtcAec_Init() fail, errcode: {0}", ret);
		}

		webrtc::WebRtcAec_InitAec_SSE2();
	
		webrtc::WebRtcAec_SetConfigCore(webrtc::WebRtcAec_aec_core(aecInst), webrtc::kAecNlpAggressive, webrtc::kAecTrue, webrtc::kAecFalse);
		webrtc::WebRtcAec_enable_refined_adaptive_filter(webrtc::WebRtcAec_aec_core(aecInst), webrtc::kAecFalse);
		webrtc::WebRtcAec_enable_extended_filter(webrtc::WebRtcAec_aec_core(aecInst), webrtc::kAecTrue);
		webrtc::WebRtcAec_enable_delay_agnostic(webrtc::WebRtcAec_aec_core(aecInst), webrtc::kAecTrue);

		/// Creating NS
		nsInst = WebRtcNsx_Create();
		if (nsInst == nullptr)
		{
			errLog->error("AEC :: WebRtcNsx_Create() fail");
		}

		ret = WebRtcNsx_Init(static_cast<NsxHandle*>(nsInst), SAMPLING_FREQ);
		if (ret != 0)
		{
			errLog->error("AEC :: WebRtcNsx_Init() fail, errcode: {0}", ret);
		}

		ret = WebRtcNsx_set_policy(static_cast<NsxHandle*>(nsInst), 2);
		if (ret != 0)
		{
			errLog->error("AEC :: WebRtcNsx_set_policy() fail, errcode: {0}", ret);
		}

		/// Creating AGC
		agcInst = WebRtcAgc_Create();
		if (agcInst == nullptr)
		{
			errLog->error("AEC :: WebRtcAgc_Create() fail");
		}

		ret = WebRtcAgc_Init(agcInst, 0, 100, kAgcModeFixedDigital, SAMPLING_FREQ);
		if (ret != 0)
		{
			errLog->error("AEC :: WebRtcAgc_Init() fail, errcode: {0}", ret);
		}

		WebRtcAgcConfig agcConfig;
		agcConfig.compressionGaindB = 9;
		agcConfig.limiterEnable = kAgcTrue;
		agcConfig.targetLevelDbfs = 3;
		ret = WebRtcAgc_set_config(agcInst, agcConfig);
		if (ret != 0)
		{
			errLog->error("AEC :: WebRtcAgc_set_config() fail, errcode: {0}", ret);
		}

		speakerSource.Start();
		microphoneSource.Start();

		sysLog->info("AEC started");
	}
}

void AEC::Stop()
{
	if (runned)
	{
		runned = false;

		microphoneSource.Stop();
		speakerSource.Stop();
		
		webrtc::WebRtcAec_Free(aecInst);
		aecInst = nullptr;

		WebRtcNsx_Free(static_cast<NsxHandle*>(nsInst));
		nsInst = nullptr;

		WebRtcAgc_Free(agcInst);
		agcInst = nullptr;

		sysLog->info("AEC stopped");
	}
}

void AEC::EnableAEC(bool yes)
{
	aecEnabled = yes;
}

bool AEC::AECEnabled()
{
	return aecEnabled;
}

void AEC::EnableNS(bool yes)
{
	nsEnabled = yes;
}

bool AEC::NSEnabled()
{
	return nsEnabled;
}

void AEC::EnableAGC(bool yes)
{
	agcEnabled = yes;
}

bool AEC::AGCEnabled()
{
	return agcEnabled;
}

void AEC::SetMicLevel(int level)
{
	micLevel = level;
}

void AEC::SetRenderLatency(int16_t latency)
{
	renderLatency = latency;
}

}
