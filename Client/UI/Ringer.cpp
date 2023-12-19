/**
 * Ringer.h : Defines the alert sound ringer
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <UI/Ringer.h>

#include <wui/config/config.hpp>

#include <Transport/RTP/RTPPacket.h>

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
	snd(),
	thread(),
	timeMeter(),
	processTime(0)
{
    mixer.AddInput(OUT_SSRC, 0);
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

    Stop();

    ringType = ringType_;
    snd.clear();
    
    runned = true;
    thread = std::thread(&Ringer::run, this);
}
void Ringer::Stop()
{
	runned = false;
	if (thread.joinable()) thread.join();
}

bool Ringer::Runned() const
{
    return runned;
}

void Ringer::run()
{
#ifndef _WIN32
    auto resoursesPath = wui::config::get_string("Application", "Resources", "~/." CLIENT_USER_FOLDER "/res");
#endif

    int32_t ringCount = 0;

	switch (ringType)
	{
	case RingType::CallIn:
	{
		auto autoAnswerRingingCount = wui::config::get_int("User", "AutoAnswerRinging", 1);
		bool autoAnswer = wui::config::get_int("User", "CallAutoAnswer", 0) != 0;
		
#ifdef _WIN32
        Load(SND_CALLIN);
#else
        Load(resoursesPath + "/" + SND_CALLIN);
#endif

		while (runned && ringCount < 10)
		{
			if (ringCount == 0)
				mixer.SetInputVolume(OUT_SSRC, 40);
			else if (ringCount == 1)
				mixer.SetInputVolume(OUT_SSRC, 60);
			else if (ringCount == 2)
				mixer.SetInputVolume(OUT_SSRC, 80);
			else
				mixer.SetInputVolume(OUT_SSRC, 100);

			Play();
            ++ringCount;

			if (autoAnswer && ringCount >= autoAnswerRingingCount)
			{
				break;
			}
		}
		if (runned && endCallback)
		{
            runned = false;

            endCallback(ringType);
		}
	}
	break;
	case RingType::CallOut:
	{
		mixer.SetInputVolume(OUT_SSRC, 70);

#ifdef _WIN32
        Load(SND_CALLOUT);
#else
        Load(resoursesPath + "/" + SND_CALLOUT);
#endif
	
		while (runned && ringCount < 10)
		{
			Play();
            ++ringCount;
		}
		if (runned && endCallback)
		{
            runned = false;

            endCallback(ringType);
		}
	}
	break;
	case RingType::ScheduleConnectQuick: case RingType::ScheduleConnectLong:
	{
#ifdef _WIN32
        Load(SND_SCHEDULECONNECT);
#else
        Load(resoursesPath + "/" + SND_SCHEDULECONNECT);
#endif

        while (runned && ringCount < (ringType == RingType::ScheduleConnectQuick ? 2 : 10))
		{
			if (ringCount == 0)
				mixer.SetInputVolume(OUT_SSRC, 40);
			else if (ringCount == 1)
				mixer.SetInputVolume(OUT_SSRC, 60);
			else if (ringCount == 2)
				mixer.SetInputVolume(OUT_SSRC, 80);
			else
				mixer.SetInputVolume(OUT_SSRC, 100);

			Play();
            ++ringCount;
		}
		if (runned && endCallback)
		{
            runned = false;

            endCallback(ringType);
		}
	}
	break;
	case RingType::Dial:
	{
#ifdef _WIN32
        Load(SND_DIAL);
#else
        Load(resoursesPath + "/" + SND_DIAL);
#endif

		Play();

		runned = false;
	}
	break;
	case RingType::Hangup:
	{
#ifdef _WIN32
        Load(SND_HANGUP);
#else
        Load(resoursesPath + "/" + SND_HANGUP);
#endif

		Play();
			
		runned = false;
	}
	break;
	case RingType::NewMessage:
	{
#ifdef _WIN32
        Load(SND_NEW_MESSAGE);
#else
        Load(resoursesPath + "/" + SND_NEW_MESSAGE);
#endif

		Play();

		runned = false;
	}
	break;
	case RingType::SoundCheck:
	{
#ifdef _WIN32
        Load(SND_SCHEDULECONNECT);
#else
        Load(resoursesPath + "/" + SND_SCHEDULECONNECT);
#endif

		Play();

		runned = false;
	}
	break;
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

void Ringer::Play()
{
    if (snd.empty())
    {
        return;
    }

    auto playPosition = START_POS;

    while (runned && playPosition <= snd.size() - (FRAME_SIZE * 2))
    {
        auto startTime = timeMeter.Measure();
        while (runned && processTime < FRAME_DURATION - 1000 && timeMeter.Measure() - startTime < FRAME_DURATION - processTime - 1000)
        {
            Common::ShortSleep();
        }

        auto sendTime = timeMeter.Measure();

        playPosition += FRAME_SIZE;

        Transport::RTPPacket packet;
        packet.rtpHeader.ssrc = OUT_SSRC;
        packet.payload = (uint8_t*)snd.c_str() + playPosition;
        packet.payloadSize = FRAME_SIZE;
        mixer.Send(packet);
        
        processTime = timeMeter.Measure() - sendTime;
    }
}

}
