/**
 * AudioRendererWASAPI.h - Contains audio renderer's header, based on WASAPI (for Vista+ systems)
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2019, 2024
 */

#pragma once

#include <AudioRenderer/AudioRenderer.h>

#include <Transport/ISocket.h>
#include <Audio/SoundBlock.h>

#include <memory>
#include <thread>
#include <string>
#include <atomic>

#include <atlcomcli.h>

struct IMMDeviceEnumerator;
struct IMMDevice;
struct IAudioClient;
struct IAudioRenderClient;
struct ISimpleAudioVolume;

namespace AudioRenderer
{

class AudioRendererWASAPI
{
public:
	AudioRendererWASAPI(std::function<void(Transport::OwnedRTPPacket&)> pcmSource);
	~AudioRendererWASAPI();

	/// Impl of IAudioRenderer
	bool SetDeviceName(std::string_view name);
	void Start(int32_t sampleFreq);
	void Stop();
	std::vector<std::string> GetSoundRenderers();
	bool SetMute(bool yes);
	bool GetMute();
	void SetVolume(uint16_t val);
    uint16_t GetVolume();
	uint32_t GetLatency() const;

	void SetAECReceiver(Transport::ISocket *socket);
    void SetErrorHandler(std::function<void(uint32_t code, std::string_view msg)>);
	
private:
	bool runned, mute;
	float volume;

	std::string deviceName;
    int32_t sampleFreq;

	CComPtr<IMMDeviceEnumerator> enumerator;
	CComPtr<IMMDevice> mmDevice;
	CComPtr<IAudioClient> audioClient;
	CComPtr<IAudioRenderClient> audioRenderClient;
	CComPtr<ISimpleAudioVolume> audioVolume;

	HANDLE playReadyEvent, closeEvent;

	uint32_t bufferFrameCount;

	uint32_t latency;

	Transport::OwnedRTPPacket packet;
	size_t subFrame;

	Transport::ISocket* aecReceiver;

	std::function<void(Transport::OwnedRTPPacket&)> pcmSource;
    std::function<void(uint32_t code, std::string_view msg)> errorHandler;

	std::thread thread;
	void Play();
};

}
