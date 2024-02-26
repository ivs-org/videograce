/**
 * AudioMixer.cpp - Contains audio mixer's impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016 - 2018
 */

#include <mt/thread_priority.h>

#include <Transport/RTP/RTPPacket.h>

#include <Common/ShortSleep.h>

#include "webrtc/common_audio/signal_processing/include/signal_processing_library.h"

#ifdef _WIN32
#include <windows.h>
#include <avrt.h>
#include <Common/WindowsVersion.h>
#endif

#include "AudioMixer.h"

#include <iostream>

namespace Audio
{

/// AudioMixer

AudioMixer::AudioMixer()
    : runned(false),
    mutex(),
    outBuffer{ soundblock_t(), soundblock_t(), soundblock_t(), soundblock_t() },
    outPos(0),
    receiver(),
    playThread()
{
}

AudioMixer::~AudioMixer()
{
    AudioMixer::Stop();
}

void AudioMixer::AddInput(uint32_t ssrc, int64_t clientId)
{
    
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

void AudioMixer::SetReceiver(Transport::ISocket *socket)
{
    receiver = socket;
}

void AudioMixer::Start()
{
    using namespace std::chrono;

    if (!runned)
    {
        runned = true;

        playThread = std::thread([this](){

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

                //auto playDuration = duration_cast<microseconds>(high_resolution_clock::now() - startTime).count();

                while (runned && duration_cast<microseconds>(high_resolution_clock::now() - startTime).count() < FRAME_DURATION - 500)
                {
                    Common::ShortSleep();
                }

                /*while (runned && duration_cast<microseconds>(high_resolution_clock::now() - startTime).count() < FRAME_DURATION)
                {
                }*/

                //auto totalDuration = duration_cast<microseconds>(high_resolution_clock::now() - startTime).count();
                
                //if (totalDuration > FRAME_DURATION)
                //{
                    //std::cout << "play: " << playDuration << ", " << totalDuration << std::endl;
                    //OutputDebugStringA("play: ");
                    //OutputDebugStringA(std::to_string(playDuration).c_str());
                    //OutputDebugStringA(", ");
                    //OutputDebugStringA(std::to_string(totalDuration).c_str());
                    //OutputDebugStringA("\n");
                //}
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

void AudioMixer::Send(const Transport::IPacket &packet, const Transport::Address *)
{
    if (!runned)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex);

    soundblock_t &outBlock = outBuffer[outPos];
    auto &rtpPacket = *static_cast<const Transport::RTPPacket*>(&packet);
    
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
        std::lock_guard<std::mutex> lock(mutex);
        outBlock = outBuffer[outPos > 0 ? outPos - 1 : 3];
        memset(outBuffer[outPos > 0 ? outPos - 1 : 3].data, 0, sizeof(outBlock.data));
        
        ++outPos;
        if (outPos > outBuffer.size() - 1) outPos = 0;
    }

    Transport::RTPPacket packet;
    packet.rtpHeader.ts = outBlock.ts;
    packet.payload = outBlock.data;
    packet.payloadSize = FRAME_SIZE;

    receiver->Send(packet);
}

}
