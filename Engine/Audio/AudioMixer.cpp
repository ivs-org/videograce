/**
 * AudioMixer.cpp - Contains audio mixer's impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016 - 2018, 2024
 */

#include <mt/thread_priority.h>

#include <Transport/RTP/RTPPacket.h>

#include "webrtc/common_audio/signal_processing/include/signal_processing_library.h"

#ifdef _WIN32
#include <windows.h>
#include <avrt.h>
#include <Common/WindowsVersion.h>
#endif

#include <Common/ShortSleep.h>

#include "AudioMixer.h"

#include <iostream>
#include <algorithm>

namespace Audio
{

/// AudioMixer

using namespace std::chrono;

AudioMixer::AudioMixer()
    : inputs(),
    runned(false)
{
}

AudioMixer::~AudioMixer()
{
    Stop();
}

void AudioMixer::AddInput(uint32_t ssrc,
    int64_t clientId,
    std::function<void(Transport::OwnedRTPPacket&)> pcmCallback,
    int32_t volume)
{
    if (std::find_if(inputs.begin(), inputs.end(), [ssrc](const Input& p) { return p.ssrc == ssrc; }) == inputs.end())
    {
        inputs.emplace_back(Input{ ssrc, clientId, pcmCallback, volume });
    }
}

void AudioMixer::SetInputVolume(uint32_t ssrc, int32_t volume)
{
    auto input = std::find_if(inputs.begin(), inputs.end(), [ssrc](const Input& p) { return p.ssrc == ssrc; });
    if (input != inputs.end())
    {
        input->volume = volume;
    }
}

void AudioMixer::DeleteInput(uint32_t ssrc)
{
    auto input = std::find_if(inputs.begin(), inputs.end(), [ssrc](const Input& p) { return p.ssrc == ssrc; });
    if (input != inputs.end())
    {
        inputs.erase(input);
    }
}

void AudioMixer::Start()
{
    if (!runned)
    {
        runned = true;
    }
}

void AudioMixer::Stop()
{
    if (runned)
    {
        runned = false;
    }
}

void AudioMixer::GetSound(Transport::OwnedRTPPacket& outputBuffer)
{
    for (auto& input : inputs)
    {
        Transport::OwnedRTPPacket inputPacket(FRAME_SIZE * 2);
        input.pcmCallback(inputPacket);

        if (inputPacket.size == 0) continue;

        for (uint16_t i = 0; i != FRAME_SIZE; i += 2)
        {
            const auto availableFrame = *reinterpret_cast<const int16_t*>(outputBuffer.data + i);
            const auto addingFrame = *reinterpret_cast<const int16_t*>(inputPacket.data + i);

            int32_t resultFrame = availableFrame + addingFrame;

            // Saturate (to 32767) so that the signal does not overflow
            resultFrame = WEBRTC_SPL_SAT(static_cast<int32_t>(32767), resultFrame, static_cast<int32_t>(-32768));

            *reinterpret_cast<int16_t*>(outputBuffer.data + i) = resultFrame;
        }
    }
}

}
