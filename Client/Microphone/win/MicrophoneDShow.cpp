/**
* MicrophoneDShow.cpp - Contains DirectShow microphone's impl
*
* Author: Anton (ud) Golovkov, udattsk@gmail.com
* Copyright (C), Infinity Video Soft LLC, 2014, 2023
*/

#include <Microphone/win/MicrophoneDShow.h>

#include <Transport/RTP/RTPPacket.h>

#include <Common/Common.h>
#include <wui/config/config.hpp>

#include <boost/nowide/convert.hpp>

#include <assert.h>

namespace MicrophoneNS
{

MicrophoneDShow::MicrophoneDShow(Common::TimeMeter &timeMeter_, Transport::ISocket &receiver_)
	: timeMeter(timeMeter_),
    receiver(receiver_),
	mediaControl(),
	mixer(),
    deviceNotifyCallback(),
	deviceName(),
	deviceId(0),
    frequency(wui::config::get_int("CaptureDevices", "MicrophoneSampleFreq", 48000)),
	gain(wui::config::get_int("CaptureDevices", "MicrophoneGain", 80)),
	mute(false),
	runned(false),
	restarting(false),
	thread(),
	sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{
}

MicrophoneDShow::~MicrophoneDShow()
{
	Stop();
}

void MicrophoneDShow::SetDeviceName(std::string_view name)
{
	deviceName = name;
	if (runned)
	{
		Stop();
		Start();
	}
}

void MicrophoneDShow::SetDeviceId(uint32_t id)
{
	deviceId = id;
}

void MicrophoneDShow::Start()
{
    if (!runned)
	{
		runned = true;

		thread = std::thread(&MicrophoneDShow::Run, this);
	}
}

void MicrophoneDShow::Stop()
{
	if (mediaControl)
	{
		mediaControl->Stop();
	}
		
	runned = false;
	if (thread.joinable()) thread.join();
}

void MicrophoneDShow::SetGain(uint16_t gain_)
{
    wui::config::set_int("CaptureDevices", "MicrophoneGain", gain_);

	gain = gain_;
	if (mixer)
	{
		mixer->put_MixLevel(static_cast<double>(gain) / 100);
	}
}

uint16_t MicrophoneDShow::GetGain() const
{
    return gain;
}

void MicrophoneDShow::SetMute(bool yes)
{
	mute = yes;
}

bool MicrophoneDShow::GetMute() const
{
	return mute;
}

void MicrophoneDShow::SetSampleFreq(int32_t freq)
{
    frequency = freq;
    if (runned)
    {
        Stop();
        Start();
    }
}

int32_t MicrophoneDShow::GetSampleFreq() const
{
    return frequency;
}

void MicrophoneDShow::SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback_)
{
    deviceNotifyCallback = deviceNotifyCallback_;
}

HRESULT MicrophoneDShow::Run()
{
	CoInitialize(NULL);
	CComPtr<IFilterGraph2> graph;
	graph.CoCreateInstance(CLSID_FilterGraph);

	HRESULT hr = BuildGraph(graph, deviceName, gain, frequency, stereo, this, &MicrophoneDShow::OutputCallback, mixer);
	if (SUCCEEDED(hr))
	{
		mediaControl = CComQIPtr<IMediaControl, &IID_IMediaControl>(graph);
		hr = mediaControl->Run();

        if (SUCCEEDED(hr))
        {
            sysLog->info("Microphone {0} successfully started", deviceName);
        }
        else
        {
            if (deviceNotifyCallback && runned)
            {
                deviceNotifyCallback(deviceName, Client::DeviceNotifyType::MicrophoneError, Proto::DeviceType::Microphone, deviceId, hr);
            }
            runned = false;
            errLog->critical("Microphone {0} start error {1}", deviceName, (uint32_t)hr);
        }

		mixer->put_MixLevel((double)gain / 100);

		CComQIPtr<IMediaEvent, &IID_IMediaEvent> mediaEvent(graph);

		MSG msg;
		while (runned)
		{
			long ev = 0;
			LONG_PTR p1 = 0, p2 = 0;

			Sleep(250);

			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				DispatchMessage(&msg);
			}

			while (mediaEvent->GetEvent(&ev, &p1, &p2, 0) == S_OK)
			{
				if ((ev == EC_COMPLETE || ev == EC_USERABORT) && !restarting)
				{
					sysLog->warn("[MIC] Audio Capture graph done");
					
					mediaEvent->FreeEventParams(ev, p1, p2);
					runned = false;
				}
				else if (ev == EC_ERRORABORT)
				{
					errLog->error("[MIC] Audio Capture graph error occured {0:x}", (uint32_t)p1);

					mediaControl->Stop();
					if (deviceNotifyCallback)
					{
                        deviceNotifyCallback(deviceName, Client::DeviceNotifyType::MicrophoneError, Proto::DeviceType::Microphone, deviceId, static_cast<uint32_t>(p1));
					}

					mediaEvent->FreeEventParams(ev, p1, p2);
					runned = false;
				}

				mediaEvent->FreeEventParams(ev, p1, p2);
			}
		}
	}
	else
	{
		if (deviceNotifyCallback)
		{
            deviceNotifyCallback(deviceName, Client::DeviceNotifyType::MicrophoneError, Proto::DeviceType::Microphone, deviceId, hr);
		}
	}

	mediaControl.Release();
	mixer.Release();

	CoUninitialize();

	return 0;
}

void CALLBACK MicrophoneDShow::OutputCallback(unsigned char* data, int len, void *instance_)
{
    auto &instance = *static_cast<MicrophoneDShow*>(instance_);

    Transport::RTPPacket packet;
    packet.rtpHeader.ts = static_cast<uint32_t>(instance.timeMeter.Measure() / 1000);
    packet.payload = data;
    packet.payloadSize = len;

	instance.receiver.Send(packet);
}

}
