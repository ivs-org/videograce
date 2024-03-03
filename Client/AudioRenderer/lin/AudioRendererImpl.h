/**
 * AudioRendererImpl.h - Contains audio renderer's header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014, 2015, 2022
 */

#pragma once

#include <AudioRenderer/AudioRenderer.h>
#include <Transport/ISocket.h>

#include <pulse/simple.h>
#include <pulse/error.h>

#include <atomic>
#include <string>
#include <thread>
#include <vector>

#include <spdlog/spdlog.h>

namespace AudioRenderer
{

class AudioRendererImpl
{
public:
    AudioRendererImpl(std::function<void(Transport::OwnedRTPPacket&)> pcmSource);
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

private:
    std::atomic_bool runned;

    std::string deviceName;
    int32_t sampleFreq;

    uint16_t volume;

    std::atomic_bool mute;

    pa_simple *s;

    size_t subFrame;
    Transport::OwnedRTPPacket packet;

    Transport::ISocket* aecReceiver;
    std::function<void(Transport::OwnedRTPPacket&)> pcmSource;

    std::thread thread;
	void Play();

	std::shared_ptr<spdlog::logger> sysLog, errLog;
};

}
