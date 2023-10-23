/**
 * AudioRenderer.cpp - Contains audio renderer's impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2019
 */

#include <AudioRenderer/win/AudioRendererImpl.h>

#include <Common/WindowsVersion.h>

namespace AudioRenderer
{

AudioRendererImpl::AudioRendererImpl()
    : use_wasapi(Common::IsWindowsVistaOrGreater()), wasapi(), wave()
{
}

AudioRendererImpl::~AudioRendererImpl()
{
}

bool AudioRendererImpl::SetDeviceName(const std::string &name)
{
    return use_wasapi ? wasapi.SetDeviceName(name) : wave.SetDeviceName(name);
}

void AudioRendererImpl::Start(int32_t sampleFreq)
{
    use_wasapi ? wasapi.Start(sampleFreq) : wave.Start(sampleFreq);
}

void AudioRendererImpl::Stop()
{
    use_wasapi ? wasapi.Stop() : wave.Stop();
}

bool AudioRendererImpl::SetMute(bool yes)
{
    return use_wasapi ? wasapi.SetMute(yes) : wave.SetMute(yes);
}

bool AudioRendererImpl::GetMute()
{
    return use_wasapi ? wasapi.GetMute() : wave.GetMute();
}

void AudioRendererImpl::SetVolume(uint16_t value)
{
    use_wasapi ? wasapi.SetVolume(value) : wave.SetVolume(value);
}

uint16_t AudioRendererImpl::GetVolume()
{
    return use_wasapi ? wasapi.GetVolume() : wave.GetVolume();
}

void AudioRendererImpl::Send(const Transport::IPacket &packet_, const Transport::Address *)
{
    use_wasapi ? wasapi.Send(packet_, nullptr) : wave.Send(packet_, nullptr);
}

std::vector<std::string> AudioRendererImpl::GetSoundRenderers()
{
    return use_wasapi ? wasapi.GetSoundRenderers() : wave.GetSoundRenderers();
}

uint32_t AudioRendererImpl::GetLatency() const
{
    return use_wasapi ? wasapi.GetLatency() : wave.GetLatency();
}

void AudioRendererImpl::SetAECReceiver(Transport::ISocket *socket)
{
    use_wasapi ? wasapi.SetAECReceiver(socket) : wave.SetAECReceiver(socket);
}

void AudioRendererImpl::SetErrorHandler(std::function<void(uint32_t code, const std::string &msg)> handler)
{
    use_wasapi ? wasapi.SetErrorHandler(handler) : wave.SetErrorHandler(handler);
}

}
