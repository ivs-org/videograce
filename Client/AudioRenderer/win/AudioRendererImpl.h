/**
 * AudioRendererImpl.h - Contains audio renderer's header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#pragma once

#include <AudioRenderer/IAudioRenderer.h>

#include <AudioRenderer/win/AudioRendererWASAPI.h>
#include <AudioRenderer/win/AudioRendererWave.h>

namespace AudioRenderer
{

class AudioRendererImpl : public IAudioRenderer, public Transport::ISocket
{
public:
    AudioRendererImpl();
	virtual ~AudioRendererImpl();

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
    bool use_wasapi;
    AudioRendererWASAPI wasapi;
    AudioRendererWave wave;
};

}
