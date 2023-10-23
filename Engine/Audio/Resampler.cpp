/**
 * Resampler.cpp - Contains audio resampler implementation
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2023
 */

#include <Audio/Resampler.h>

#include <Transport/RTP/RTPPacket.h>

#include <wui/config/config.hpp>

#include <cstdlib>

namespace Audio
{

Resampler::Resampler(Transport::ISocket &receiver_)
	: receiver(receiver_),
    inFreq(wui::config::get_int("CaptureDevices", "MicrophoneSampleFreq", 48000)),
    outFreq(wui::config::get_int("AudioRenderer", "SampleFreq", 48000)),
    resamplingState16_48(), resamplingState48_16(),
    tmpMem(static_cast<int32_t*>(malloc(3 * 336 * sizeof(int32_t)))),
    resampledOut(static_cast<int16_t*>(malloc(160 * 3 * 2 * sizeof(int16_t) * 3)))
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

void Resampler::Send(const Transport::IPacket &packet_, const Transport::Address *)
{
    if (inFreq == outFreq)
    {
        receiver.Send(packet_);
    }
    else if (inFreq == 16000 && outFreq == 48000)
    {
        const Transport::RTPPacket &inPacket = *static_cast<const Transport::RTPPacket*>(&packet_);

        for (int i = 0; i != 320; i += 160)
        {
            WebRtcSpl_Resample16khzTo48khz(reinterpret_cast<const int16_t*>(inPacket.payload) + i,
                resampledOut + i * 3,
                &resamplingState16_48,
                tmpMem);
        }

        Transport::RTPPacket outPacket = inPacket;
        outPacket.payload = reinterpret_cast<uint8_t*>(resampledOut);
        outPacket.payloadSize = inPacket.payloadSize * 3;

        receiver.Send(outPacket);
    }
    else if (inFreq == 48000 && outFreq == 16000)
    {
        const Transport::RTPPacket &inPacket = *static_cast<const Transport::RTPPacket*>(&packet_);

        for (int i = 0; i != 960; i += 480)
        {
            WebRtcSpl_Resample48khzTo16khz(reinterpret_cast<const int16_t*>(inPacket.payload) + i,
                resampledOut + i / 3,
                &resamplingState48_16,
                tmpMem);
        }

        Transport::RTPPacket outPacket = inPacket;
        outPacket.payload = reinterpret_cast<uint8_t*>(resampledOut);
        outPacket.payloadSize = inPacket.payloadSize / 3;

        receiver.Send(outPacket);
    }
}

}
