/**
 * Microphone.h - Contains microphone's header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2023 -2023
 */

#pragma once

#include <Microphone/IMicrophone.h>

#include <Microphone/win/MicrophoneDMO.h>
#include <Microphone/win/MicrophoneDShow.h>

#include <Transport/ISocket.h>

#include <UI/DeviceNotifies.h>

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
    int32_t frequency;

    bool use_dmo;

    MicrophoneDMO dmo;
    MicrophoneDShow dshow;
};

}
