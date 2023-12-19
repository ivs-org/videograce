/**
 * MicrophoneDShow.h - Contains DirectShow microphone's header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014, 2023
 */

#pragma once

#include <Microphone/IMicrophone.h>
#include <Microphone/win/MicrophoneDSG.h>

#include <Transport/ISocket.h>

#include <Common/TimeMeter.h>

#include <Audio/SoundBlock.h>

#include <UI/DeviceNotifies.h>

#include <thread>
#include <atomic>
#include <string>

#include <spdlog/spdlog.h>

namespace MicrophoneNS
{

class MicrophoneDShow : public IMicrophone
{
public:
    MicrophoneDShow(Common::TimeMeter &timeMeter, Transport::ISocket &receiver);
    ~MicrophoneDShow();

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
		
    CComQIPtr<IMediaControl, &IID_IMediaControl> mediaControl;
    MicroMixerPtr mixer;

    Client::DeviceNotifyCallback deviceNotifyCallback;

	std::string deviceName;
	uint32_t deviceId;

    int32_t frequency, gain;
	bool mute;

	const int32_t bandwidth = 16;
	const bool stereo = false;
				
	std::atomic<bool> runned;
	std::atomic<bool> restarting;

	std::thread thread;

	std::shared_ptr<spdlog::logger> sysLog, errLog;

	HRESULT Run();

	static void CALLBACK OutputCallback(unsigned char* data, int len, void *instance);
};

}
