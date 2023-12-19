/**
* Microphone.cpp - Contains microphone's impl
*
* Author: Anton (ud) Golovkov, udattsk@gmail.com
* Copyright (C), Infinity Video Soft LLC, 2014
*/

#include <Microphone/Microphone.h>

#ifdef _WIN32
#include <Microphone/win/MicrophoneImpl.h>
#elif __linux__
#include <Microphone/lin/MicrophoneImpl.h>
#endif

namespace MicrophoneNS
{

Microphone::Microphone(Common::TimeMeter &timeMeter_, Transport::ISocket &receiver_)
    : impl(new MicrophoneImpl(timeMeter_, receiver_))
{
}

Microphone::~Microphone()
{
}

void Microphone::SetDeviceName(std::string_view name)
{
    impl->SetDeviceName(name);
}

void Microphone::SetDeviceId(uint32_t id)
{
    impl->SetDeviceId(id);
}

void Microphone::Start()
{
    impl->Start();
}

void Microphone::Stop()
{
    impl->Stop();
}

void Microphone::SetGain(uint16_t gain_)
{
    impl->SetGain(gain_);
}

uint16_t Microphone::GetGain() const
{
    return impl->GetGain();
}

void Microphone::SetMute(bool yes)
{
    impl->SetMute(yes);
}

bool Microphone::GetMute() const
{
    return impl->GetMute();
}

void Microphone::SetSampleFreq(int32_t freq)
{
    impl->SetSampleFreq(freq);
}

int32_t Microphone::GetSampleFreq() const
{
    return impl->GetSampleFreq();
}

void Microphone::SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback_)
{
    impl->SetDeviceNotifyCallback(deviceNotifyCallback_);
}

}
