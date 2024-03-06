/**
* Microphone.cpp - Contains microphone's impl
*
* Author: Anton (ud) Golovkov, udattsk@gmail.com
* Copyright (C), Infinity Video Soft LLC, 2014, 2022
*/

#include <Microphone/win/MicrophoneImpl.h>

#include <Common/WindowsVersion.h>

#include <wui/config/config.hpp>

namespace MicrophoneNS
{

MicrophoneImpl::MicrophoneImpl(Common::TimeMeter &timeMeter, Transport::ISocket &receiver)
	: frequency(wui::config::get_int("SoundSystem", "SampleFreq", 48000)),
    use_dmo(frequency == 16000 && Common::IsWindowsVistaOrGreater()),
    dmo(timeMeter, receiver),
    dshow(timeMeter, receiver)
{
}

MicrophoneImpl::~MicrophoneImpl()
{
    use_dmo ? dmo.Stop() : dshow.Stop();
}

void MicrophoneImpl::SetDeviceName(std::string_view name)
{
    use_dmo ? dmo.SetDeviceName(name) : dshow.SetDeviceName(name);
}

void MicrophoneImpl::SetDeviceId(uint32_t id)
{
    use_dmo ? dmo.SetDeviceId(id) : dshow.SetDeviceId(id);
}

void MicrophoneImpl::Start(ssrc_t ssrc)
{
    use_dmo ? dmo.Start(ssrc) : dshow.Start(ssrc);
}

void MicrophoneImpl::Stop()
{
    use_dmo ? dmo.Stop() : dshow.Stop();
}

void MicrophoneImpl::SetGain(uint16_t gain)
{
    use_dmo ? dmo.SetGain(gain) : dshow.SetGain(gain);
}

uint16_t MicrophoneImpl::GetGain() const
{
    return use_dmo ? dmo.GetGain() : dshow.GetGain();
}

void MicrophoneImpl::SetMute(bool yes)
{
    use_dmo ? dmo.SetMute(yes) : dshow.SetMute(yes);
}

bool MicrophoneImpl::GetMute() const
{
    return use_dmo ? dmo.GetMute() : dshow.GetMute();
}

void MicrophoneImpl::SetSampleFreq(int32_t freq)
{
    use_dmo = frequency == 16000 && Common::IsWindowsVistaOrGreater();
    
    use_dmo ? dmo.SetSampleFreq(freq) : dshow.SetSampleFreq(freq);
}

int32_t MicrophoneImpl::GetSampleFreq() const
{
    return use_dmo ? dmo.GetSampleFreq() : dshow.GetSampleFreq();
}

void MicrophoneImpl::SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback)
{
    use_dmo ? dmo.SetDeviceNotifyCallback(deviceNotifyCallback) : dshow.SetDeviceNotifyCallback(deviceNotifyCallback);
}

}
