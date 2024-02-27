/**
 * AudioMixer.cpp - Contains audio mixer's impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016 - 2018
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

namespace Audio
{

/// AudioMixer

using namespace std::chrono;

AudioMixer::AudioMixer(Transport::ISocket& receiver_)
    : receiver(receiver_),
    runned(false),
    outBuffer{ },
    pos(0),
    playThread()
{
}

AudioMixer::~AudioMixer()
{
    Stop();
}

void AudioMixer::AddInput(uint32_t ssrc, int64_t clientId, int32_t volume)
{
    
}

void AudioMixer::SetInputVolume(uint32_t ssrc, int32_t volume)
{
    
}

void AudioMixer::DeleteInput(uint32_t ssrc)
{
    
}

void AudioMixer::Start()
{
    if (!runned)
    {
        runned = true;

        playThread = std::thread([this]() {

#ifdef _WIN32
            HANDLE hTask = 0;
            if (Common::IsWindowsVistaOrGreater())
            {
                DWORD taskIndex = 0;
                hTask = AvSetMmThreadCharacteristics(L"Pro Audio", &taskIndex);
            }
#endif
            while (runned)
            {
                auto startTime = high_resolution_clock::now();

                Play();

                auto playDuration = duration_cast<microseconds>(high_resolution_clock::now() - startTime).count();

                if (FRAME_DURATION > playDuration) Common::ShortSleep(FRAME_DURATION - playDuration - 1000);

                auto totalDuration = duration_cast<microseconds>(high_resolution_clock::now() - startTime).count();
                
                //if (totalDuration > FRAME_DURATION)
                {
                    //std::cout << "play: " << playDuration << ", " << totalDuration << std::endl;
                    OutputDebugStringA("play: ");
                    OutputDebugStringA(std::to_string(playDuration).c_str());
                    OutputDebugStringA(", ");
                    OutputDebugStringA(std::to_string(totalDuration).c_str());
                    OutputDebugStringA("\n");
                }
            }
#ifdef _WIN32
            if (Common::IsWindowsVistaOrGreater())
            {
                AvRevertMmThreadCharacteristics(hTask);
            }
#endif
        });

        mt::set_thread_priority(playThread, mt::priority_type::real_time);
    }
}

void AudioMixer::Stop()
{
    if (runned)
    {
        runned = false;
        if (playThread.joinable()) playThread.join();
    }
}

void AudioMixer::Send(const Transport::IPacket& packet, const Transport::Address*)
{
    if (!runned)
    {
        return;
    }

    soundblock_t& outBlock = outBuffer[pos];
    auto& rtpPacket = *static_cast<const Transport::RTPPacket*>(&packet);

    outBlock.size = FRAME_SIZE;
    outBlock.ts = rtpPacket.rtpHeader.ts;
    outBlock.seq = rtpPacket.rtpHeader.seq;

    for (uint16_t i = 0; i != FRAME_SIZE; i += 2)
    {
        const auto availableFrame = *reinterpret_cast<const int16_t*>(outBlock.data + i);
        const auto addingFrame = *reinterpret_cast<const int16_t*>(rtpPacket.payload + i);

        int32_t resultFrame = availableFrame + addingFrame;

        // Saturate (to 32767) so that the signal does not overflow
        resultFrame = WEBRTC_SPL_SAT(static_cast<int32_t>(32767), resultFrame, static_cast<int32_t>(-32768));

        *reinterpret_cast<int16_t*>(outBlock.data + i) = resultFrame;
    }
}

void AudioMixer::Play()
{
    soundblock_t outBlock;
    {
        //std::lock_guard<std::mutex> lock(mutex);

        auto readPos = pos > 0 ? pos - 1 : outBuffer.size() - 1;
        
        outBlock = outBuffer[readPos];
        
        memset(outBuffer[readPos].data, 0, sizeof(outBlock.data));
        
        ++pos;
        if (pos > outBuffer.size() - 1) pos = 0;
    }

    if (outBlock.data[0] == 0)
    {
        OutputDebugStringA("e\n");
    }
    else
    {
        OutputDebugStringA("-\n");
    }

    Transport::RTPPacket packet;
    packet.rtpHeader.ts = outBlock.ts;
    packet.payload = outBlock.data;
    packet.payloadSize = FRAME_SIZE;

    receiver.Send(packet);
}

void AudioMixer::PreciseSleep(std::chrono::steady_clock::time_point start)
{
    while (duration_cast<microseconds>(high_resolution_clock::now() - start).count() < FRAME_DURATION)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}

}
