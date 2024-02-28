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

	void Start();
	void Stop();

	/// Return PCM data 48000, 16, 1 various size how much the 
	/// sound card needs at the moment (480 - 3840 bytes)
	void GetSound(Transport::OwnedRTPPacket& outputBuffer);

private:
	static const uint16_t FRAME_SIZE = 1920;
	static const uint64_t FRAME_DURATION = 20000;

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
