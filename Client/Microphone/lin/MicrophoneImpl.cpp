/**
* Microphone.cpp - Contains microphone's impl
*
* Author: Anton (ud) Golovkov, udattsk@gmail.com
* Copyright (C), Infinity Video Soft LLC, 2014
*/

#include <Microphone/lin/MicrophoneImpl.h>

#include <Transport/RTP/RTPPacket.h>
#include <Transport/RTP/RTPPayloadType.h>

#include <Common/ShortSleep.h>

#include <Version.h>

#include <assert.h>

#include <wui/config/config.hpp>

namespace MicrophoneNS
{

MicrophoneImpl::MicrophoneImpl(Common::TimeMeter &timeMeter_, Transport::ISocket &receiver_)
    : timeMeter(timeMeter_),
    receiver(receiver_),
	deviceName(),
	deviceId(0),
	ssrc(0),
	seq(0),
	sampleFreq(wui::config::get_int("SoundSystem", "SampleFreq", 48000)),
    gain(wui::config::get_int("CaptureDevices", "MicrophoneGain", 100)),
	mute(false),
	runned(false),
	thread(),
	s(nullptr),
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
		Start(ssrc);
	}
}

void MicrophoneImpl::SetDeviceId(uint32_t id)
{
	deviceId = id;
}

void MicrophoneImpl::Start(ssrc_t ssrc_)
{
	if (runned)
	{
		return;
	}

	ssrc = ssrc_;
	seq = 0;

	static const pa_sample_spec ss = {
			.format = PA_SAMPLE_S16LE,
			.rate = static_cast<uint32_t>(sampleFreq),
			.channels = 1
	};

	int error = 0;

	pa_buffer_attr ba = {
		.maxlength = (uint32_t) -1,
    	.tlength = (uint32_t) -1,
    	.prebuf = (uint32_t) -1,
    	.minreq = (uint32_t) -1,
    	.fragsize = static_cast<uint32_t>((sampleFreq / 100) * 2)
	};
	// Create a new record stream
	if (!(s = pa_simple_new(NULL, SYSTEM_NAME "Client", PA_STREAM_RECORD, NULL, "record", &ss, NULL, &ba, &error)))
	{
		return errLog->critical("MicrophoneImpl :: pa_simple_new() failed: {0}", pa_strerror(error));
	}

	runned = true;
	thread = std::thread(&MicrophoneImpl::run, this);

	sysLog->info("Microphone {0} was started", deviceName);
}

void MicrophoneImpl::Stop()
{
	if (!runned)
	{
		return;
	}

	runned = false;
	if (thread.joinable()) thread.join();
	
	pa_simple_free(s);

	sysLog->info("Microphone {0} was stoped", deviceName);
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
    sampleFreq = freq_;
    if (runned)
    {
        Stop();
        Start(ssrc);
    }
}

int32_t MicrophoneImpl::GetSampleFreq() const
{
    return sampleFreq;
}

void MicrophoneImpl::SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback)
{

}

void MicrophoneImpl::run()
{
	using namespace std::chrono;
	int64_t packetDuration = 40000;
	
	const uint32_t readCount = (sampleFreq / 100) * 2; // 10 ms frame

	int error = 0;

	uint8_t buf[readCount * 4] = { 0 };
	int32_t subFrame = 0;
	while (runned)
	{
		auto start = high_resolution_clock::now();

		if (pa_simple_read(s, buf + (subFrame * readCount), readCount, &error) < 0)
		{
        	return errLog->critical("MicrophoneImpl :: pa_simple_read() failed: {0}", pa_strerror(error));
		}

		++subFrame;
		if (subFrame > 3)
		{
			if (!mute)
			{
				Transport::RTPPacket packet;
				packet.rtpHeader.ts = timeMeter.Measure() / 1000;
				packet.rtpHeader.ssrc = ssrc;
				packet.rtpHeader.seq = ++seq;
				packet.rtpHeader.pt = static_cast<uint32_t>(Transport::RTPPayloadType::ptPCM);
				packet.payload = buf;
				packet.payloadSize = readCount;
				receiver.Send(packet);
			}
			subFrame = 0;
			memset(buf, 0, readCount);
		}
	}
}

}
