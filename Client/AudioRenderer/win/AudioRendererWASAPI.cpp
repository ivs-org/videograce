/**
 * AudioRenderer.cpp - Contains audio renderer's impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2019
 */

#include <atlbase.h>

#include <mmdeviceapi.h>
#include <audioclient.h>
#include <avrt.h>
#include <FunctionDiscoveryKeys_devpkey.h>
#include <Audiosessiontypes.h>

#include "AudioRendererWASAPI.h"
#include <Transport/RTP/RTPPacket.h>

#include <wui/config/config.hpp>

#include <boost/nowide/convert.hpp>

#include <spdlog/spdlog.h>

#ifndef AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM
#define AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM 0x80000000
#endif
#ifndef AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY
#define AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY 0x08000000
#endif

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

template <class T> inline void SafeRelease(T*& p)
{
	if (p)
	{
		p->Release();
		p = nullptr;
	}
}

#define CHECK_HR(hr, msg) if (!SUCCEEDED(hr)) { if (errorHandler) { errorHandler(hr, msg); } runned = false; return; }

namespace AudioRenderer
{

AudioRendererWASAPI::AudioRendererWASAPI()
	: runned(false), mute(wui::config::get_int("AudioRenderer", "Enabled", 1) == 0),
	volume(static_cast<float>(wui::config::get_int("AudioRenderer", "Volume", 100)) / 100),
	deviceName(),
    sampleFreq(wui::config::get_int("AudioRenderer", "SampleFreq", 48000)),
	enumerator(),
	audioClient(),
	audioRenderClient(),
	audioVolume(),
	bufferFrameCount(0),
	latency(0),
	aecReceiver(nullptr),
    errorHandler()
{
}

AudioRendererWASAPI::~AudioRendererWASAPI()
{
	Stop();
}

bool AudioRendererWASAPI::SetDeviceName(std::string_view name)
{
	if (deviceName == name)
	{
		return false;
	}

	deviceName = name;
	if (runned)
	{
		Stop();
		Start(sampleFreq);
	}
	return true;
}

std::wstring GetDeviceID(const std::wstring &deviceName, IMMDeviceEnumerator &enumerator)
{
	std::wstring out;

	CComPtr<IMMDeviceCollection> devices;
	HRESULT hr = enumerator.EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &devices);
	if (!SUCCEEDED(hr))
	{
		return out;
	}

	UINT cnt = 0;
	devices->GetCount(&cnt);
	for (UINT i = 0; i != cnt; ++i)
	{
		CComPtr<IMMDevice> device;
		hr = devices->Item(i, &device);
		if (SUCCEEDED(hr))
		{
			CComPtr<IPropertyStore> props;
			hr = device->OpenPropertyStore(STGM_READ, &props);
			if (SUCCEEDED(hr))
			{
				PROPVARIANT name;
				PropVariantInit(&name);
				hr = props->GetValue(PKEY_Device_FriendlyName, &name);
				if (SUCCEEDED(hr))
				{
					if (name.bstrVal == deviceName)
					{
						LPWSTR strId = nullptr;
						hr = device->GetId(&strId);
						if (SUCCEEDED(hr))
						{
							out = strId;
						}
						CoTaskMemFree(strId);
					}
				}
				PropVariantClear(&name);
			}
		}
	}

	return out;
}

void AudioRendererWASAPI::Start(int32_t sampleFreq_)
{
	if (runned)
	{
		return;
	}

    sampleFreq = sampleFreq_;

	HRESULT hr = CoCreateInstance(CLSID_MMDeviceEnumerator, 0, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&enumerator);
	CHECK_HR(hr, "MMDeviceEnumerator creating instance")

	if (!deviceName.empty())
	{
		hr = enumerator->GetDevice(GetDeviceID(boost::nowide::widen(deviceName), *enumerator).c_str(), &mmDevice);
		CHECK_HR(hr, "MMDeviceEnumerator GetDevice")
	}
	else
	{
		hr = enumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &mmDevice);
		CHECK_HR(hr, "MMDeviceEnumerator GetDefaultAudioEndpoint")
	}

	// Get its IAudioClient (used to set audio format, latency, and start/stop)
	hr = mmDevice->Activate(IID_IAudioClient, CLSCTX_ALL, nullptr, (void **)&audioClient);
	CHECK_HR(hr, "mmDevice->Activate")

	WAVEFORMATEX wfx = { 0 };
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = 1;
	wfx.nSamplesPerSec = sampleFreq;
	wfx.wBitsPerSample = 16;
	wfx.nBlockAlign = (wfx.wBitsPerSample / 8) * wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

    // {C1ACAFA8-3EAB-4374-AAB4-3C15DCE823E1}
    const GUID guid = { 0xc1acafa8, 0x3eab, 0x4374, { 0xaa, 0xb4, 0x3c, 0x15, 0xdc, 0xe8, 0x23, 0xe1 } };
    	
	hr = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
		AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY, 600000, 600000, &wfx, &guid);
	CHECK_HR(hr, "audioClient->Initialize")

	// Get the actual size (in sample frames) of each half of the circular audio buffer
	hr = audioClient->GetBufferSize(&bufferFrameCount);
	CHECK_HR(hr, "audioClient->GetBufferSize")

	// Get the IAudioRenderClient (used to access the audio buffer)
	hr = audioClient->GetService(IID_IAudioRenderClient, (void **)&audioRenderClient);
	CHECK_HR(hr, "audioClient->GetService(IAudioRenderClient)")

	// Fill the first half of buffer with silence before we start the stream
	BYTE *pData = nullptr;
	hr = audioRenderClient->GetBuffer(bufferFrameCount, &pData);
	CHECK_HR(hr, "audioRenderClient->GetBuffer")

	hr = audioRenderClient->ReleaseBuffer(bufferFrameCount, AUDCLNT_BUFFERFLAGS_SILENT);
	CHECK_HR(hr, "audioRenderClient->ReleaseBuffer")
	
	// Start audio playback
	hr = audioClient->Start();
	CHECK_HR(hr, "audioClient->Start")

	hr = audioClient->GetService(__uuidof(ISimpleAudioVolume), (void**)&audioVolume);
	CHECK_HR(hr, "audioClient->GetService(ISimpleAudioVolume)")

	hr = audioVolume->SetMasterVolume(volume, nullptr);
	CHECK_HR(hr, "audioVolume->SetMasterVolume")

	REFERENCE_TIME latency_;
	if (SUCCEEDED(audioClient->GetStreamLatency(&latency_)))
	{
		REFERENCE_TIME period = 0;
		if (SUCCEEDED(audioClient->GetDevicePeriod(&period, nullptr)))
		{
			latency = (uint32_t)(latency_ / 10000 + period / 10000);
		}
	}

	runned = true;
}

void AudioRendererWASAPI::Stop()
{
	runned = false;
		
	// Stop playing
	if (audioClient)
	{
		audioClient->Stop();
	}

	audioVolume.Release();
	audioRenderClient.Release();
	audioClient.Release();
	mmDevice.Release();
	enumerator.Release();
}

bool AudioRendererWASAPI::SetMute(bool yes)
{
    wui::config::set_int("AudioRenderer", "Enabled", yes ? 0 : 1);
	mute = yes;
	return true;
}

bool AudioRendererWASAPI::GetMute()
{
	return mute;
}

void AudioRendererWASAPI::SetVolume(uint16_t value)
{
    wui::config::set_int("AudioRenderer", "Volume", value);
	volume = static_cast<float>(value) / 100;
	if (runned && audioVolume)
	{
		audioVolume->SetMasterVolume(volume, nullptr);
	}
}

uint16_t AudioRendererWASAPI::GetVolume()
{
    return static_cast<uint16_t>(volume * 100);
}

void AudioRendererWASAPI::Send(const Transport::IPacket &packet_, const Transport::Address *)
{
	if (runned && !mute)
	{
		const auto &packet = *static_cast<const Transport::RTPPacket*>(&packet_);

		uint32_t framesPadding = 0;
		HRESULT hr = audioClient->GetCurrentPadding(&framesPadding);
		CHECK_HR(hr, "audioClient->GetCurrentPadding")

		uint32_t framesAvailable = bufferFrameCount - framesPadding;
		if (framesAvailable * 2 > packet.payloadSize)
		{
			framesAvailable = packet.payloadSize / 2;
		}

		BYTE *pData = nullptr;
		hr = audioRenderClient->GetBuffer(framesAvailable, &pData);
		CHECK_HR(hr, "audioClient->GetBuffer")

		memcpy(pData, packet.payload, framesAvailable * 2);

		hr = audioRenderClient->ReleaseBuffer(framesAvailable, 0);
		CHECK_HR(hr, "audioRenderClient->ReleaseBuffer")

		if (aecReceiver)
		{
			aecReceiver->Send(packet_);
		}
	}
}

std::vector<std::string> AudioRendererWASAPI::GetSoundRenderers()
{
	std::vector<std::string> out;

	CComPtr<IMMDeviceEnumerator> enumerator_;
	
	HRESULT hr = CoCreateInstance(CLSID_MMDeviceEnumerator, 0, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&enumerator_);
	if (!SUCCEEDED(hr) && errorHandler)
	{
        errorHandler(hr, "MMDeviceEnumerator creating instance");
		return out;
	}

	CComPtr<IMMDeviceCollection> devices;
	hr = enumerator_->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &devices);
	if (!SUCCEEDED(hr) && errorHandler)
	{
        errorHandler(hr, "MMDeviceEnumerator EnumAudioEndpoints");
		return out;
	}

	std::wstring defDeviceName;
	CComPtr <IMMDevice> defDevice;
	hr = enumerator_->GetDefaultAudioEndpoint(eRender, eMultimedia, &defDevice);
	if (!SUCCEEDED(hr) && errorHandler)
	{
        errorHandler(hr, "MMDeviceEnumerator GetDefaultAudioEndpoint");
		return out;
	}
	CComPtr<IPropertyStore> props;
	hr = defDevice->OpenPropertyStore(STGM_READ, &props);
	if (SUCCEEDED(hr))
	{
		PROPVARIANT name;
		PropVariantInit(&name);
		hr = props->GetValue(PKEY_Device_FriendlyName, &name);
		if (SUCCEEDED(hr))
		{
			defDeviceName = name.bstrVal;
		}
		PropVariantClear(&name);
	}

	UINT cnt = 0;
	devices->GetCount(&cnt);
	for (UINT i = 0; i != cnt; ++i)
	{
		CComPtr<IMMDevice> device;
		hr = devices->Item(i, &device);
		if (SUCCEEDED(hr))
		{
			CComPtr<IPropertyStore> props;
			hr = device->OpenPropertyStore(STGM_READ, &props);
			if (SUCCEEDED(hr))
			{
				PROPVARIANT name;
				PropVariantInit(&name);
				hr = props->GetValue(PKEY_Device_FriendlyName, &name);
				if (SUCCEEDED(hr))
				{
					if (name.bstrVal != defDeviceName)
					{
						out.emplace_back(boost::nowide::narrow(name.bstrVal));
					}
					else
					{
						out.insert(out.begin(), boost::nowide::narrow(name.bstrVal));
					}
				}
				PropVariantClear(&name);
			}
		}
	}

	return out;
}

uint32_t AudioRendererWASAPI::GetLatency() const
{
	return wui::config::get_int("AudioRenderer", "ManualLatency", 0) == 0 && latency != 0 ?
        latency : wui::config::get_int("AudioRenderer", "Latency", 100);
}

void AudioRendererWASAPI::SetAECReceiver(Transport::ISocket *socket)
{
	aecReceiver = socket;
}

void AudioRendererWASAPI::SetErrorHandler(std::function<void(uint32_t code, std::string_view msg)> errorHandler_)
{
    errorHandler = errorHandler_;
}

}
