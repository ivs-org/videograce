/**
 * Resampler.cpp - Contains audio resampler implementation
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2023, 2024
 */

#include <Audio/Resampler.h>

#include <cstdlib>

namespace Audio
{

Resampler::Resampler()
	: inFreq(48000),
    outFreq(16000),
    resamplingState16_48(), resamplingState48_16(),
    tmpMem(static_cast<int32_t*>(malloc(3 * 336 * sizeof(int32_t)))),
    resampledOut(static_cast<int16_t*>(malloc(480 * sizeof(int16_t) * 4)))
{
}

Resampler::~Resampler()
{
    free(tmpMem);
    free(resampledOut);
}

void Resampler::SetSampleFreq(int32_t inFreq_, int32_t outFreq_)
{
    inFreq = inFreq_;
    outFreq = outFreq_;
}

void Resampler::Resample(const Transport::RTPPacket& inPacket, Transport::RTPPacket& out)
{
    if (inFreq == outFreq)
    {
        out = inPacket;
    }
    else if (inFreq == 16000 && outFreq == 48000)
    {
        for (int i = 0; i != inPacket.payloadSize / 3; i += 160)
        {
            WebRtcSpl_Resample16khzTo48khz(reinterpret_cast<const int16_t*>(inPacket.payload) + i,
                resampledOut + i * 3,
                &resamplingState16_48,
                tmpMem);
        }

        out = inPacket;
        out.payload = reinterpret_cast<uint8_t*>(resampledOut);
        out.payloadSize = inPacket.payloadSize * 3;
    }
    else if (inFreq == 48000 && outFreq == 16000)
    {
        for (int i = 0; i != inPacket.payloadSize; i += 480)
        {
            WebRtcSpl_Resample48khzTo16khz(reinterpret_cast<const int16_t*>(inPacket.payload) + i,
                resampledOut + i / 3,
                &resamplingState48_16,
                tmpMem);
        }

        out = inPacket;
        out.payload = reinterpret_cast<uint8_t*>(resampledOut);
        out.payloadSize = inPacket.payloadSize / 3;
    }
}

}
