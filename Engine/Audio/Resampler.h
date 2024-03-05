/**
 * Resampler.h - Contains audio resampler interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2023, 2024
 */

#pragma once

#include <Transport/RTP/RTPPacket.h>

#include "webrtc/common_audio/signal_processing/include/signal_processing_library.h"

namespace Audio
{

class Resampler
{
public:
    Resampler();
	~Resampler();

    void SetSampleFreq(int32_t inFreq, int32_t outFreq);

	void Resample(const Transport::RTPPacket& in, Transport::RTPPacket& out);

private:
    int32_t inFreq, outFreq;

    WebRtcSpl_State16khzTo48khz resamplingState16_48;
    WebRtcSpl_State48khzTo16khz resamplingState48_16;
    int32_t *tmpMem;
    int16_t *resampledOut;
};

}
