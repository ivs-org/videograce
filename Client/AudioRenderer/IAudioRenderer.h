/**
 * IAudioRenderer.h - Contains interface of audio renderer
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014, 2022
 */

#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include <functional>

#include <Transport/ISocket.h>

namespace AudioRenderer
{

class IAudioRenderer
{
public:
	virtual bool SetDeviceName(std::string_view name) = 0;

	virtual void Start(int32_t sampleFreq) = 0;
	virtual void Stop() = 0;

	virtual std::vector<std::string> GetSoundRenderers() = 0;

	virtual bool SetMute(bool yes) = 0;
	virtual bool GetMute() = 0;
	virtual void SetVolume(uint16_t val) = 0;
    virtual uint16_t GetVolume() = 0;

	virtual uint32_t GetLatency() const = 0;

	virtual void SetAECReceiver(Transport::ISocket *socket) = 0;

    virtual void SetErrorHandler(std::function<void(uint32_t code, std::string_view msg)>) = 0;

	virtual ~IAudioRenderer() {}
};

}
