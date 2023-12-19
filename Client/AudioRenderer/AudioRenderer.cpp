/**
 * AudioRenderer.cpp - Contains audio renderer's impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <AudioRenderer/AudioRenderer.h>

#ifdef _WIN32
#include <AudioRenderer/win/AudioRendererImpl.h>
#elif __linux__
#include <AudioRenderer/lin/AudioRendererImpl.h>
#endif

namespace AudioRenderer
{

AudioRenderer::AudioRenderer()
	: impl(new AudioRendererImpl())
{
}

AudioRenderer::~AudioRenderer()
{
}

bool AudioRenderer::SetDeviceName(std::string_view name)
{
    return impl->SetDeviceName(name);
}

void AudioRenderer::Start(int32_t sampleFreq)
{
    impl->Start(sampleFreq);
}

void AudioRenderer::Stop()
{
    impl->Stop();
}

bool AudioRenderer::SetMute(bool yes)
{
    return impl->SetMute(yes);
}

bool AudioRenderer::GetMute()
{
    return impl->GetMute();
}

void AudioRenderer::SetVolume(uint16_t value)
{
    impl->SetVolume(value);
}

uint16_t AudioRenderer::GetVolume()
{
    return impl->GetVolume();
}

void AudioRenderer::Send(const Transport::IPacket &packet_, const Transport::Address *)
{
    impl->Send(packet_, nullptr);
}

std::vector<std::string> AudioRenderer::GetSoundRenderers()
{
    return impl->GetSoundRenderers();
}

uint32_t AudioRenderer::GetLatency() const
{
    return impl->GetLatency();
}

void AudioRenderer::SetAECReceiver(Transport::ISocket *socket)
{
    impl->SetAECReceiver(socket);
}

void AudioRenderer::SetErrorHandler(std::function<void(uint32_t code, std::string_view msg)> handler)
{
    impl->SetErrorHandler(handler);
}

}
