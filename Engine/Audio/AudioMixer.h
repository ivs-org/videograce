/**
 * AudioMixer.h - Contains audio mixer's header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016, 2017
 */

#pragma once

#include <atomic>
#include <mutex>
#include <thread>
#include <queue>
#include <vector>
#include <map>

#include <mt/rw_lock.h>
#include <mt/wf_ring_buffer.h>

#include <Transport/ISocket.h>

#include <Audio/SoundBlock.h>

#include <functional>

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

	struct Input
	{
		int64_t clientId;
		
		int32_t volume;

        mt::ringbuffer<soundblock_t, 4> buffer;
		
		Input(int64_t clientId_)
			: clientId(clientId_), volume(100), buffer()
		{}

		Input(int64_t clientId_, int32_t volume_)
			: clientId(clientId_), volume(volume_), buffer()
		{}
	};
	
	std::atomic<bool> runned;

	mt::rw_lock inputsRWLock;
	std::map<uint32_t, std::unique_ptr<Input>> inputs;
	
	Transport::ISocket* receiver;
	
	std::thread thread;

	void Process();

	void Mix();
};

}
