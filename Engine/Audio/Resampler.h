/**
 * Resampler.h - Contains audio resampler interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2023
 */

#pragma once

#include <Transport/ISocket.h>

#include "webrtc/common_audio/signal_processing/include/signal_processing_library.h"

namespace Audio
{

class Resampler : public Transport::ISocket
{
public:
    Resampler(Transport::ISocket &receiver);
	~Resampler();

    void SetSampleFreq(int32_t inFreq, int32_t outFreq);

	/// Derived from Transport::ISocket (input method)
	virtual void Send(const Transport::IPacket &packet, const Transport::Address *address = nullptr) final;

private:
    Transport::ISocket &receiver;

    int32_t inFreq, outFreq;

    WebRtcSpl_State16khzTo48khz resamplingState16_48;
    WebRtcSpl_State48khzTo16khz resamplingState48_16;
    int32_t *tmpMem;
    int16_t *resampledOut;
};

}
