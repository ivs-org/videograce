/**
 * AudioMixer.h - Contains audio mixer's header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016, 2017, 2024
 */

#pragma once

#include <atomic>
#include <array>
#include <functional>

#include <Transport/RTP/OwnedRTPPacket.h>
#include <Audio/SoundBlock.h>

namespace Audio
{

class AudioMixer
{
public:
	AudioMixer();
	~AudioMixer();
	
	/// Input must provide a method that gives PCM data 48000, 16, 1
	void AddInput(uint32_t ssrc,
		int64_t clientId,
		std::function<void(Transport::OwnedRTPPacket&)> pcmCallback,
		int32_t volume = 0);

	void SetInputVolume(uint32_t ssrc, int32_t volume);
	void DeleteInput(uint32_t ssrc);

	void Start(uint32_t sampleFreq);
	void Stop();

	/// Return PCM data 48000, 16, 1 
	void GetSound(Transport::OwnedRTPPacket& outputBuffer);

private:
	uint16_t frameSize; // 4 frames of 48 k mono

	struct Input
	{
		uint32_t ssrc;
		int64_t clientId;
		std::function<void(Transport::OwnedRTPPacket&)> pcmCallback;
		int32_t volume;
	};

	std::vector<Input> inputs;

	std::atomic_bool runned;
};

}
