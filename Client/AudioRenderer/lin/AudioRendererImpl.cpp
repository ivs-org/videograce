/**
* AudioRendererImpl.cpp - Contains audio renderer's impl
*
* Author: Anton (ud) Golovkov, udattsk@gmail.com
* Copyright (C), Infinity Video Soft LLC, 2014, 2015, 2022
*/

#include <AudioRenderer/lin/AudioRendererImpl.h>

#include <algorithm>

#include <Transport/RTP/RTPPacket.h>

#include <wui/config/config.hpp>

#include <Version.h>

namespace AudioRenderer
{

AudioRendererImpl::AudioRendererImpl()
	: runned(false),
	deviceName("default"),
	volume(wui::config::get_int("AudioRenderer", "Volume", 100)),
    mute(wui::config::get_int("AudioRenderer", "Enabled", 1) == 0),
	s(nullptr),
	aecReceiver(nullptr),
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

void AudioRendererImpl::Start(int32_t sampleFreq)
{
	if (runned)
	{
		return;
	}

	static const pa_sample_spec ss = {
			.format = PA_SAMPLE_S16LE,
			.rate = static_cast<uint32_t>(sampleFreq),
			.channels = 1
	};

	int error = 0;

	pa_buffer_attr ba = {
		.maxlength = static_cast<uint32_t>((sampleFreq / 25) * 2)
	};
	// Create a new playback stream
	if (!(s = pa_simple_new(NULL, SYSTEM_NAME "Client", PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, &ba, &error)))
	{
		return errLog->critical("pa_simple_new() failed: {0}", pa_strerror(error));
	}

	runned = true;

	sysLog->info("AudioRenderer {0} was started", deviceName);
}

void AudioRendererImpl::Stop()
{
	if (!runned)
	{
		return;
	}

	runned = false;

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

void AudioRendererImpl::Send(const Transport::IPacket &packet_, const Transport::Address *)
{
	if (!runned || mute)
	{
		return;
	}
	
	const Transport::RTPPacket *packet = static_cast<const Transport::RTPPacket*>(&packet_);
	{
		int error = 0;
		if (pa_simple_write(s, packet->payload, packet->payloadSize, &error) < 0)
		{
			errLog->critical("pa_simple_write() failed: {0}", pa_strerror(error));
		}
	}

	if (aecReceiver)
	{
		aecReceiver->Send(packet_);
	}
}

}
