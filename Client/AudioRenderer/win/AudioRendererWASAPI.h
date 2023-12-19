/**
 * AudioRendererWASAPI.h - Contains audio renderer's header, based on WASAPI (for Vista+ systems)
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2019
 */

#pragma once

#include <AudioRenderer/IAudioRenderer.h>

#include <Transport/ISocket.h>
#include <Audio/SoundBlock.h>

#include <memory>
#include <thread>
#include <string>
#include <atomic>
#include <queue>

#include <atlcomcli.h>

struct IMMDeviceEnumerator;
struct IMMDevice;
struct IAudioClient;
struct IAudioRenderClient;
struct ISimpleAudioVolume;

namespace AudioRenderer
{

class AudioRendererWASAPI : public IAudioRenderer, public Transport::ISocket
{
public:
	AudioRendererWASAPI();
	virtual ~AudioRendererWASAPI();

	/// Impl of IAudioRenderer
	virtual bool SetDeviceName(std::string_view name) final;
	virtual void Start(int32_t sampleFreq) final;
	virtual void Stop() final;
	virtual std::vector<std::string> GetSoundRenderers() final;
	virtual bool SetMute(bool yes) final;
	virtual bool GetMute() final;
	virtual void SetVolume(uint16_t val) final;
    virtual uint16_t GetVolume() final;
	virtual uint32_t GetLatency() const final;

	virtual void SetAECReceiver(Transport::ISocket *socket) final;
    virtual void SetErrorHandler(std::function<void(uint32_t code, std::string_view msg)>) final;
	
	/// Impl of Transport::ISocket
	virtual void Send(const Transport::IPacket &packet, const Transport::Address *) final;

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

	uint32_t bufferFrameCount;

	uint32_t latency;

	Transport::ISocket* aecReceiver;

    std::function<void(uint32_t code, std::string_view msg)> errorHandler;
};

}
