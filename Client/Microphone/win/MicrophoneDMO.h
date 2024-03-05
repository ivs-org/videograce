/**
 * Microphone.h - Contains DMO microphone's header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014, 2023
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

class MicrophoneDMO : public IMicrophone
{
public:
    MicrophoneDMO(Common::TimeMeter &timeMeter, Transport::ISocket &receiver);
    ~MicrophoneDMO();

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

    Client::DeviceNotifyCallback deviceNotifyCallback;

	std::string deviceName;
	uint32_t deviceId;

    std::wstring dmoDeviceID;

    int32_t gain;
	bool mute;
    				
	std::atomic<bool> runned;
	std::atomic<bool> restarting;

	std::thread thread;

	std::shared_ptr<spdlog::logger> sysLog, errLog;

	HRESULT Run();
};

}
