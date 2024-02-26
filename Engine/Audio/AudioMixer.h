/**
 * AudioMixer.h - Contains audio mixer's header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016, 2017
 */

#pragma once

#include <atomic>
#include <array>
#include <thread>
#include <mutex>
#include <functional>

#include <Transport/ISocket.h>

#include <Audio/SoundBlock.h>

namespace Audio
{

class AudioMixer : public Transport::ISocket
{
public:
	AudioMixer();
	~AudioMixer();
	
	/// AudioMixer interface
	void AddInput(uint32_t ssrc, int64_t clientId);
	void AddInput(uint32_t ssrc, int64_t clientId, int32_t volume);
	void SetInputVolume(uint32_t ssrc, int32_t volume);
	void DeleteInput(uint32_t ssrc);

	void SetReceiver(Transport::ISocket *socket);

	void Start();
	void Stop();

	/// Impl of Transport::ISocket (receive sound from inputs)
	virtual void Send(const Transport::IPacket &packet, const Transport::Address *address = nullptr) final;

private:
	static const uint16_t FRAME_SIZE = 1920;
	static const uint64_t FRAME_DURATION = 20000;

	std::atomic_bool runned;

	std::mutex mutex;
	std::array<soundblock_t, 4> outBuffer;
	size_t outPos;
	
	Transport::ISocket* receiver;
	
	std::thread playThread;

	void Play();
};

}
