/**
 * Microphone.h - Contains microphone's header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <Microphone/IMicrophone.h>
#include <Transport/ISocket.h>
#include <Common/TimeMeter.h>

#include <UI/DeviceNotifies.h>

#include <thread>
#include <atomic>
#include <string>

#include <spdlog/spdlog.h>

namespace MicrophoneNS
{

class MicrophoneImpl : public IMicrophone
{
public:
    MicrophoneImpl(Common::TimeMeter &timeMeter, Transport::ISocket &receiver);
    ~MicrophoneImpl();

    /// Impl of IMicrophone
    virtual void SetDeviceName(std::string_view name) final;
    virtual void SetDeviceId(uint32_t id) final;

    virtual void Start() final;
    virtual void Stop() final;

    virtual void SetGain(uint16_t gain) final;
    virtual uint16_t GetGain() const final;

    virtual void SetMute(bool yes) final;
    virtual bool GetMute() const final;

    virtual void SetSampleFreq(int32_t freq) final;
    virtual int32_t GetSampleFreq() const final;

    void SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback);

private:
    Common::TimeMeter &timeMeter;
    Transport::ISocket &receiver;

    std::string deviceName;
    uint32_t deviceId;

    int32_t freq, gain;
    bool mute;

    std::atomic<bool> runned;
    std::thread thread;

    uint32_t processTime;

    std::shared_ptr<spdlog::logger> sysLog, errLog;

    void run();
};

}
