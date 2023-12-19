/**
* Microphone.cpp - Contains microphone's impl
*
* Author: Anton (ud) Golovkov, udattsk@gmail.com
* Copyright (C), Infinity Video Soft LLC, 2014
*/

#include <Microphone/lin/MicrophoneImpl.h>
#include <Transport/RTP/RTPPacket.h>

#include <assert.h>

#include <wui/config/config.hpp>

#include <alsa/asoundlib.h>

namespace MicrophoneNS
{

MicrophoneImpl::MicrophoneImpl(Common::TimeMeter &timeMeter_, Transport::ISocket &receiver_)
    : timeMeter(timeMeter_),
    receiver(receiver_),
	deviceName(),
	deviceId(0),
	freq(wui::config::get_int("CaptureDevices", "MicrophoneSampleFreq", 48000)),
    gain(wui::config::get_int("CaptureDevices", "MicrophoneGain", 100)),
	mute(false),
	runned(false),
	thread(),
	processTime(0),
	sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{
}

MicrophoneImpl::~MicrophoneImpl()
{
	Stop();
}

void MicrophoneImpl::SetDeviceName(std::string_view name)
{
	deviceName = name;
	if (runned)
	{
		Stop();
		Start();
	}
}

void MicrophoneImpl::SetDeviceId(uint32_t id)
{
	deviceId = id;
}

void MicrophoneImpl::Start()
{
	if (!runned)
	{
		runned = true;
		thread = std::thread(&MicrophoneImpl::run, this);
	}
}

void MicrophoneImpl::Stop()
{
	if (runned)
	{
		runned = false;
		if (thread.joinable()) thread.join();
	}
}

void MicrophoneImpl::SetGain(uint16_t gain_)
{
	wui::config::set_int("CaptureDevices", "MicrophoneGain", gain_);

	gain = gain_;
}

uint16_t MicrophoneImpl::GetGain() const
{
    return gain;
}

void MicrophoneImpl::SetMute(bool yes)
{
	mute = yes;
}

bool MicrophoneImpl::GetMute() const
{
	return mute;
}

void MicrophoneImpl::SetSampleFreq(int32_t freq_)
{
    freq = freq_;
    if (runned)
    {
        Stop();
        Start();
    }
}

int32_t MicrophoneImpl::GetSampleFreq() const
{
    return freq;
}

void MicrophoneImpl::SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback)
{

}

void MicrophoneImpl::run()
{
	snd_pcm_t *capture_handle;
	snd_pcm_hw_params_t *hw_params;

	int ret = snd_pcm_open(&capture_handle, deviceName.c_str(), SND_PCM_STREAM_CAPTURE, 0);
	if (ret < 0)
	{
		return errLog->critical("Microphone {0} cannot open audio device ({1})", deviceName, snd_strerror(ret));
	}

	ret = snd_pcm_hw_params_malloc(&hw_params);
	if (ret < 0)
	{
		return errLog->critical("Microphone {0} cannot allocate hardware parameter structure ({1})", deviceName, snd_strerror(ret));
	}

	ret = snd_pcm_hw_params_any(capture_handle, hw_params);
	if (ret < 0)
	{
		return errLog->critical("Microphone {0} cannot initialize hardware parameter structure ({1})", deviceName, snd_strerror(ret));
	}

	ret = snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (ret < 0)
	{
		return errLog->critical("Microphone {0} cannot set access type ({1})", deviceName, snd_strerror(ret));
	}

	ret = snd_pcm_hw_params_set_format (capture_handle, hw_params, SND_PCM_FORMAT_S16_LE);
	if (ret < 0)
	{
		return errLog->critical("Microphone {0} cannot set sample format ({1})", deviceName, snd_strerror(ret));
	}

	ret = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, reinterpret_cast<uint32_t*>(&freq), 0);
	if (ret < 0)
	{
		return errLog->critical("Microphone {0} cannot set sample rate ({1})", deviceName, snd_strerror(ret));
	}

	ret = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 1);
	if (ret < 0)
	{
		return errLog->critical("Microphone {0} cannot set channel count ({1})", deviceName, snd_strerror(ret));
	}

	ret = snd_pcm_hw_params (capture_handle, hw_params);
	if (ret < 0)
	{
		return errLog->critical("Microphone {0} cannot set parameters ({1})", deviceName, snd_strerror(ret));
	}

	snd_pcm_hw_params_free (hw_params);

	ret = snd_pcm_prepare (capture_handle);
	if (ret < 0)
	{
		return errLog->critical("Microphone {0} cannot prepare audio interface for use ({1})", deviceName, snd_strerror(ret));
	}

	const int samplesCount = freq == 48000 ? 480 * 2 : 160 * 2;
	const int bufSize = samplesCount * 2;

	char buffer[480 * 4] = { 0 };

	while (runned)
	{
		ret = snd_pcm_readi(capture_handle, buffer, samplesCount);
		if (ret != samplesCount)
		{
			errLog->critical("Microphone {0} read from audio interface failed ret ({1}) != samplesCount ({2}) ({3})", deviceName, ret, samplesCount, snd_strerror(ret));
			break;
		}

		Transport::RTPPacket packet;
		packet.rtpHeader.ts = timeMeter.Measure() / 1000;
		packet.payload = reinterpret_cast<uint8_t*>(buffer);
		packet.payloadSize = bufSize;
		receiver.Send(packet);
	}

	snd_pcm_close(capture_handle);
}

}
