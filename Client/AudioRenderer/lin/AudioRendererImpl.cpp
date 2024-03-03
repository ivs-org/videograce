/**
* AudioRendererImpl.cpp - Contains audio renderer's impl
*
* Author: Anton (ud) Golovkov, udattsk@gmail.com
* Copyright (C), Infinity Video Soft LLC, 2014, 2015, 2022
*/

#include <AudioRenderer/lin/AudioRendererImpl.h>

#include <algorithm>

#include <Transport/RTP/RTPPacket.h>

#include <Common/ShortSleep.h>

#include <wui/config/config.hpp>

#include <Version.h>

namespace AudioRenderer
{

AudioRendererImpl::AudioRendererImpl(std::function<void(Transport::OwnedRTPPacket&)> pcmSource_)
	: runned(false),
	deviceName("default"),
	sampleFreq(48000),
	volume(wui::config::get_int("AudioRenderer", "Volume", 100)),
    mute(wui::config::get_int("AudioRenderer", "Enabled", 1) == 0),
	s(nullptr),
	subFrame(0),
	packet(480 * 2 * 4),
	aecReceiver(nullptr),
	pcmSource(pcmSource_),
	thread(),
	sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{
}

AudioRendererImpl::~AudioRendererImpl()
{
	Stop();
}

bool AudioRendererImpl::SetDeviceName(std::string_view name)
{
	deviceName = name;
	return true;
}

void AudioRendererImpl::Start(int32_t sampleFreq_)
{
	if (runned)
	{
		return;
	}

	setenv("PULSE_PROP_media.role", "phone", 1);

	sampleFreq = sampleFreq_;

	static const pa_sample_spec ss = {
			.format = PA_SAMPLE_S16LE,
			.rate = static_cast<uint32_t>(sampleFreq),
			.channels = 1
	};

	int error = 0;

	/*pa_buffer_attr ba = {
		.maxlength = static_cast<uint32_t>((sampleFreq / 100) * 4)
	};*/
	// Create a new playback stream
	if (!(s = pa_simple_new(NULL, SYSTEM_NAME "Client", PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error)))
	{
		return errLog->critical("pa_simple_new() failed: {0}", pa_strerror(error));
	}

	runned = true;
	thread = std::thread(std::bind(&AudioRendererImpl::Play, this));

	sysLog->info("AudioRenderer {0} was started", deviceName);
}

void AudioRendererImpl::Stop()
{
	if (!runned)
	{
		return;
	}

	runned = false;
	if (thread.joinable()) thread.join();

	int error = 0;
	if (pa_simple_drain(s, &error) < 0)
	{
		errLog->critical("pa_simple_drain() failed: {0}", pa_strerror(error));
	}
	pa_simple_free(s);

	sysLog->info("AudioRenderer {0} was stoped", deviceName);
}

std::vector<std::string> AudioRendererImpl::GetSoundRenderers()
{
    return { "default" };
}

bool AudioRendererImpl::SetMute(bool yes)
{
    wui::config::set_int("AudioRenderer", "Enabled", yes ? 0 : 1);
    mute = yes;
	
	sysLog->info("AudioRenderer {0}", mute ? "muted" : "unmuted");
	
	packet.Clear();

    return true;
}

bool AudioRendererImpl::GetMute()
{
    return mute;
}

void AudioRendererImpl::SetVolume(uint16_t value)
{
    wui::config::set_int("AudioRenderer", "Volume", value);

	volume = value;
}

uint16_t AudioRendererImpl::GetVolume()
{
    return volume;
}

uint32_t AudioRendererImpl::GetLatency() const
{
    int error = 0;
    uint32_t latency = pa_simple_get_latency(s, &error) / 1000;

	return wui::config::get_int("AudioRenderer", "ManualLatency", 0) == 0 && latency != 0 ?
        latency : wui::config::get_int("AudioRenderer", "Latency", 100);
}

void AudioRendererImpl::SetAECReceiver(Transport::ISocket *socket)
{
	aecReceiver = socket;
}

void AudioRendererImpl::SetErrorHandler(std::function<void(uint32_t code, std::string_view msg)>)
{
}

void AudioRendererImpl::Play()
{
	using namespace std::chrono;
	int64_t packetDuration = 40000;
	
	Transport::OwnedRTPPacket packet(480 * 2 * 4);
	
	while (runned)
	{
		auto start = high_resolution_clock::now();

		const uint32_t FRAMES_COUNT = sampleFreq / 100;

		if (subFrame == 0)
		{
			pcmSource(packet);
		}

		if (!mute) /// We have to keep picking up packets from the jitter buffers
		{
			if (aecReceiver)
			{
				Transport::RTPPacket rtp;
				rtp.rtpHeader = packet.header;
				rtp.payload = packet.data + (subFrame * 960);
				rtp.payloadSize = FRAMES_COUNT * 2;
				aecReceiver->Send(rtp);
			}

			int error = 0;
			if (pa_simple_write(s, packet.data + (subFrame * 960), FRAMES_COUNT * 2, &error) < 0)
			{
				errLog->critical("pa_simple_write() failed: {0}", pa_strerror(error));
			}

			//int64_t duration = duration_cast<microseconds>(high_resolution_clock::now() - start).count();
			//sysLog->trace("pd: {0}", duration);
		}
		else
		{
			Common::ShortSleep(packetDuration - duration_cast<microseconds>(high_resolution_clock::now() - start).count());
		}

		++subFrame;
		if (subFrame > 3)
		{
			subFrame = 0;
			packet.Clear();
		}
	}
}

}
