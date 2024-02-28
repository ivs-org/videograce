/**
 * AudioRenderer.h - Contains audio renderer's header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022, 2024
 */

#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <functional>

#include <Transport/ISocket.h>
#include <Transport/RTP/OwnedRTPPacket.h>

namespace AudioRenderer
{

class AudioRendererImpl;

class AudioRenderer
{
public:
	AudioRenderer(std::function<void(Transport::OwnedRTPPacket&)> pcmSource);
	~AudioRenderer();

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
    std::unique_ptr<AudioRendererImpl> impl;
};

}
