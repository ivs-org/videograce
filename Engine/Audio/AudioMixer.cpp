/**
 * AudioMixer.cpp - Contains audio mixer's impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016 - 2018
 */

#include <algorithm>
#include <cmath>

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
	inputsRWLock(),
	inputs(),
	receiver(),
	thread()
{
}

AudioMixer::~AudioMixer()
{
	AudioMixer::Stop();
}

void AudioMixer::AddInput(uint32_t ssrc, int64_t clientId)
{
	mt::scoped_rw_lock lock(&inputsRWLock, true);

	if (inputs.find(ssrc) == inputs.end())
	{
		inputs.insert(std::pair<uint32_t, std::unique_ptr<Input>>(ssrc, std::unique_ptr<Input>(new Input(clientId))));
	}
}

void AudioMixer::AddInput(uint32_t ssrc, int64_t clientId, int32_t volume)
{
	mt::scoped_rw_lock lock(&inputsRWLock, true);

	if (inputs.find(ssrc) == inputs.end())
	{
		inputs.insert(std::pair<uint32_t, std::unique_ptr<Input>>(ssrc, std::unique_ptr<Input>(new Input(clientId, volume))));
	}
}

void AudioMixer::SetInputVolume(uint32_t ssrc, int32_t volume)
{
	mt::scoped_rw_lock lock(&inputsRWLock, false);

	auto it = inputs.find(ssrc);
	if (it != inputs.end())
	{
		it->second->volume = volume;
	}
}

void AudioMixer::DeleteInput(uint32_t ssrc)
{
	mt::scoped_rw_lock lock(&inputsRWLock, true);

	auto it = inputs.find(ssrc);
	if (it != inputs.end())
	{
		inputs.erase(it);
	}
}

void AudioMixer::SetReceiver(Transport::ISocket *socket)
{
	receiver = socket;
}

void AudioMixer::Start()
{
	if (!runned)
	{
		thread = std::thread(&AudioMixer::Process, this);
	}
}

void AudioMixer::Stop()
{
	if (runned)
	{
		runned = false;
		if (thread.joinable()) thread.join();
	}
}

void AudioMixer::Send(const Transport::IPacket &packet, const Transport::Address *)
{
	if (runned)
	{
		mt::scoped_rw_lock lock(&inputsRWLock, false);

		auto &rtpPacket = *static_cast<const Transport::RTPPacket*>(&packet);

		auto it = inputs.find(rtpPacket.rtpHeader.ssrc);
		if (it != inputs.end())
		{
			auto &input = it->second;

            input->buffer.push(soundblock_t(rtpPacket.payload, rtpPacket.payloadSize, rtpPacket.rtpHeader.seq, rtpPacket.rtpHeader.ts));
		}
	}
}

void AudioMixer::Mix()
{
    soundblock_t outBlock;
    outBlock.size = FRAME_SIZE;

	mt::scoped_rw_lock lock(&inputsRWLock, false);

	for (const auto &input_ : inputs)
	{
		const auto &input = input_.second;
        
        soundblock_t inBlock;
        input->buffer.pop(inBlock);

        auto volume = input->volume != 0 ? (exp((double)input->volume / 100) / 2.718281828) : 0;
        for (uint16_t i = 0; i != FRAME_SIZE; i += 2)
        {
            const auto availableFrame = *reinterpret_cast<const int16_t*>(outBlock.data + i);
            const auto addingFrame = static_cast<int16_t>(*reinterpret_cast<const int16_t*>(inBlock.data + i) * volume);
            
            int32_t resultFrame = availableFrame + addingFrame;
            
            // Saturate (to 32767) so that the signal does not overflow
            resultFrame = WEBRTC_SPL_SAT(static_cast<int32_t>(32767), resultFrame, static_cast<int32_t>(-32768));
            
            *reinterpret_cast<int16_t*>(outBlock.data + i) = resultFrame;
        }
        outBlock.ts = inBlock.ts;
	}
	
    Transport::RTPPacket packet;
    packet.rtpHeader.ts = outBlock.ts;
    packet.payload = outBlock.data;
    packet.payloadSize = outBlock.size;

    receiver->Send(packet);
}

void AudioMixer::Process()
{
	runned = true;

#ifdef _WIN32
	HANDLE hTask = 0;
	if (Common::IsWindowsVistaOrGreater())
	{
		DWORD taskIndex = 0;
		hTask = AvSetMmThreadCharacteristics(L"Pro Audio", &taskIndex);
	}
#endif

	const uint64_t APPROACH = 600;

	using namespace std::chrono;

	while (runned)
	{
		auto startTime = high_resolution_clock::now();

		Mix();

		auto stopProc = high_resolution_clock::now();
		
		while (duration_cast<microseconds>(high_resolution_clock::now() - startTime).count() < FRAME_DURATION - APPROACH)
		{
			Common::ShortSleep();
		}

		auto stop = high_resolution_clock::now();
		auto duration = duration_cast<microseconds>(stop - startTime);
		if (duration.count() > 20000)
		{
			auto procDuration = duration_cast<microseconds>(stopProc - startTime);
			auto sleepDuration = duration_cast<microseconds>(stop - stopProc);
			std::cout << duration.count() << ", " << procDuration.count() << ", " << sleepDuration.count() << std::endl;
		}
	}

#ifdef _WIN32
	if (Common::IsWindowsVistaOrGreater())
	{
		AvRevertMmThreadCharacteristics(hTask);
	}
#endif

}

}
