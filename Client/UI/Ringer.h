/**
 * Ringer.h : Defines the alert sound ringer
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018, 2022
 */

#pragma once

#include <Audio/AudioMixer.h>
#include <Transport/RTP/OwnedRTPPacket.h>

#include <atomic>
#include <functional>
#include <string>

namespace Client
{

class Ringer
{
	static const uint32_t START_POS = 48;
	static const uint16_t FRAME_SIZE = 1920;
	static const uint64_t FRAME_DURATION = 20000; /// 20 ms out duration
	static const uint32_t OUT_SSRC = 1;
public:
	enum class RingType
	{
		CallIn,
		CallOut,
		
		ScheduleConnectQuick,
        ScheduleConnectLong,
		
		Dial,
		Hangup,

		NewMessage,

		SoundCheck
	};

    Ringer(Audio::AudioMixer &mixer_, std::function<void(RingType)> endCallback_);
    ~Ringer();

    void Ring(RingType ringType_);
    void Stop();
    bool Runned() const;

	void GetFrame(Transport::OwnedRTPPacket&);

private:
	Audio::AudioMixer &mixer;
	RingType ringType;

    std::function<void(RingType)> endCallback;

	std::atomic_bool runned;

	int32_t currentRingCount, targetRingCount,
		playPosition;
	
    std::string snd;

#ifdef _WIN32
    void Load(int res);
#else
    void Load(std::string_view fileName);
#endif
};

}
