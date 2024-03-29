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

AudioRendererImpl::AudioRendererImpl(std::function<void(Transport::OwnedRTPPacket&)> pcmSource)
    : use_wasapi(Common::IsWindowsVistaOrGreater()), wasapi(pcmSource), wave(pcmSource)
{
}

AudioRendererImpl::~AudioRendererImpl()
{
}

bool AudioRendererImpl::SetDeviceName(std::string_view name)
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

void AudioRendererImpl::SetErrorHandler(std::function<void(uint32_t code, std::string_view msg)> handler)
{
    use_wasapi ? wasapi.SetErrorHandler(handler) : wave.SetErrorHandler(handler);
}

}
