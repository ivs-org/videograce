/**
 * Ringer.h : Defines the alert sound ringer
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <UI/Ringer.h>

#include <wui/config/config.hpp>

#include <Transport/RTP/RTPPacket.h>

#include <Common/ShortSleep.h>

#include <fstream>
#include <wui/system/tools.hpp>
#include <wui/system/path_tools.hpp>

#include <Version.h>

#include <resource.h>

namespace Client
{

Ringer::Ringer(Audio::AudioMixer &mixer_, std::function<void(RingType)> endCallback_)
	: mixer(mixer_),
	ringType(RingType::CallIn),
    endCallback(endCallback_),
	runned(false),
	currentRingCount(0), targetRingCount(1),
	playPosition(START_POS),
	snd()
{
    mixer.AddInput(OUT_SSRC, 0, std::bind(&Ringer::GetFrame, this, std::placeholders::_1));
}

Ringer::~Ringer()
{
    Stop();
    mixer.DeleteInput(OUT_SSRC);
}

void Ringer::Ring(RingType ringType_)
{
    if (ringType == RingType::NewMessage && runned)
    {
        return;
    }

#ifndef _WIN32
    auto resoursesPath = wui::config::get_string("Application", "Resources", "~/." CLIENT_USER_FOLDER "/res");
#endif

    Stop();

    ringType = ringType_;
    snd.clear();
    
    currentRingCount = 0;
	playPosition = START_POS;

	switch (ringType)
	{
	case RingType::CallIn:
	{
		targetRingCount = 10;
#ifdef _WIN32
		Load(SND_CALLIN);
#else
		Load(resoursesPath + "/" + SND_CALLIN);
#endif
	}
	break;
	case RingType::CallOut:
	{
		targetRingCount = 10;
#ifdef _WIN32
		Load(SND_CALLOUT);
#else
		Load(resoursesPath + "/" + SND_CALLOUT);
#endif
	}
	break;
	case RingType::ScheduleConnectQuick: case RingType::ScheduleConnectLong:
	{
		targetRingCount = (ringType == RingType::ScheduleConnectQuick ? 2 : 10);
#ifdef _WIN32
		Load(SND_SCHEDULECONNECT);
#else
		Load(resoursesPath + "/" + SND_SCHEDULECONNECT);
#endif
	}
	break;
	case RingType::Dial:
	{
		targetRingCount = 1;
#ifdef _WIN32
		Load(SND_DIAL);
#else
		Load(resoursesPath + "/" + SND_DIAL);
#endif
	}
	break;
	case RingType::Hangup:
	{
		targetRingCount = 1;
#ifdef _WIN32
		Load(SND_HANGUP);
#else
		Load(resoursesPath + "/" + SND_HANGUP);
#endif
	}
	break;
	case RingType::NewMessage:
	{
		targetRingCount = 1;
#ifdef _WIN32
		Load(SND_NEW_MESSAGE);
#else
		Load(resoursesPath + "/" + SND_NEW_MESSAGE);
#endif
	}
	break;
	case RingType::SoundCheck:
	{
		targetRingCount = 1;
#ifdef _WIN32
		Load(SND_SCHEDULECONNECT);
#else
		Load(resoursesPath + "/" + SND_SCHEDULECONNECT);
#endif
	}
	break;
	}

	runned = true;
}
void Ringer::Stop()
{
	runned = false;
}

bool Ringer::Runned() const
{
    return runned;
}

void Ringer::SetSampleFreq(int32_t freq)
{
	resampler.SetSampleFreq(48000, freq);
}

void Ringer::GetFrame(Transport::OwnedRTPPacket &output)
{
	if (!runned || snd.empty())
	{
		return;
	}

	Transport::RTPPacket snd48;
	snd48.payload = (uint8_t*)snd.c_str() + playPosition;
	snd48.payloadSize = FRAME_SIZE;

	Transport::RTPPacket sndOut;
	resampler.Resample(snd48, sndOut);
	
	output.header.ssrc = OUT_SSRC;
	memcpy(output.data, sndOut.payload, sndOut.payloadSize);
	output.size = sndOut.payloadSize;

	//output.header.ssrc = OUT_SSRC;
	//memcpy(output.data, (uint8_t*)snd.c_str() + playPosition, FRAME_SIZE);
	//output.size = FRAME_SIZE;
	
	playPosition += FRAME_SIZE;

	if (playPosition > snd.size() - (FRAME_SIZE * 2))
	{
		playPosition = START_POS;
		++currentRingCount;
	}

	if (currentRingCount >= targetRingCount)
	{
		runned = false;
	}
}

#ifdef _WIN32
void Ringer::Load(int res)
{
	HINSTANCE hInst = GetModuleHandle(NULL);

	HRSRC hResInfo = FindResource(hInst, MAKEINTRESOURCE(res), L"WAVE");
	if (hResInfo == NULL)
	{
		return;
	}

	auto hMedia = LoadResource(hInst, hResInfo);
	if (!hMedia)
	{
		// todo log
        return;
	}

	auto size = SizeofResource(hInst, hResInfo);

	auto data = static_cast<const char*>(LockResource(hMedia));
	if (!data)
	{
        // todo log
        return;
	}

    snd.clear();
    snd.append(data, size);

    UnlockResource(hMedia);
    FreeResource(hMedia);
}
#else
void Ringer::Load(std::string_view fileName)
{
    std::ifstream file(wui::real_path(fileName), std::ios::in | std::ios::binary | std::ios::ate);

    if (file.is_open())
    {
        file.seekg(0, std::ios::end);
        snd.reserve(file.tellg());
        file.seekg(0, std::ios::beg);

        snd.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }
}
#endif

}
