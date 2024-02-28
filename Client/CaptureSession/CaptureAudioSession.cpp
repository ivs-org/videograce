/**
 * CaptureAudioSession.cpp - Contains impl of capture audio session
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#include <wui/config/config.hpp>

#include "CaptureAudioSession.h"

#include <Transport/RTP/RTCPPacket.h>

#include <Microphone/Microphone.h>

namespace CaptureSession
{

CaptureAudioSession::CaptureAudioSession(Common::TimeMeter &timeMeter_)
	: deviceNotifyCallback(),
	audioRenderer(nullptr),
	rtpSocket(),
	wsmSocket(),
	localReceiverSplitter(),
	encryptor(),
	encoder(),
	silentDetector(*this),
	silentSplitter(),
	aec(),
    resampler(*aec.GetMicrophoneReceiver()),
	microphone(timeMeter_, resampler),
	runned(false),
	ssrc(0), deviceId(0),
	name(),
	encoderType(Audio::CodecType::Opus),
	wsAddr(), accessToken(), wsDestAddr(),
	sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{
    aec.SetReceiver(&silentSplitter);
	silentSplitter.SetReceiver0(&localReceiverSplitter);
	silentSplitter.SetReceiver1(&silentDetector);
	encoder.SetReceiver(&localReceiverSplitter);
	localReceiverSplitter.SetReceiver0(&encryptor);
	encoder.SetReceiver(&encryptor);
	encryptor.SetReceiver(&rtpSocket);
	rtpSocket.SetReceiver(nullptr, this);
	wsmSocket.SetReceiver(nullptr, this);
}

CaptureAudioSession::~CaptureAudioSession()
{
	Stop();
}

void CaptureAudioSession::SetAudioRenderer(AudioRenderer::AudioRenderer *audioRenderer_)
{
	audioRenderer = audioRenderer_;
}

void CaptureAudioSession::SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback_)
{
    deviceNotifyCallback = deviceNotifyCallback_;
	microphone.SetDeviceNotifyCallback(deviceNotifyCallback);
}

void CaptureAudioSession::SetDeviceName(std::string_view name_)
{
	name = name_;
	microphone.SetDeviceName(name_);
}

void CaptureAudioSession::EnableAEC(bool yes)
{
	aec.EnableAEC(yes);
}

void CaptureAudioSession::EnableNS(bool yes)
{
	aec.EnableNS(yes);
}

void CaptureAudioSession::EnableAGC(bool yes)
{
	aec.EnableAGC(yes);
}

void CaptureAudioSession::SetBitrate(int32_t bitrate)
{
	encoder.SetBitrate(bitrate);
}

void CaptureAudioSession::SetQuality(int32_t quality)
{
	encoder.SetQuality(quality);
}

void CaptureAudioSession::SetGain(uint16_t gain)
{
	microphone.SetGain(gain);
	aec.SetMicLevel(gain);
}

uint16_t CaptureAudioSession::GetGain() const
{
    return microphone.GetGain();
}

void CaptureAudioSession::SetSampleFreq(int32_t freq)
{
    if (runned)
    {
        microphone.Stop();
    }

    resampler.SetSampleFreq(freq, 48000);
    microphone.SetSampleFreq(freq);

    if (runned)
    {
        microphone.Start();
    }
}

int32_t CaptureAudioSession::GetSampleFreq() const
{
    return microphone.GetSampleFreq();
}

void CaptureAudioSession::SetRenderLatency(int32_t latency)
{
	aec.SetRenderLatency(latency);
}

void CaptureAudioSession::SetMute(bool yes)
{
	microphone.SetMute(yes);
}

bool CaptureAudioSession::GetMute() const
{
	return microphone.GetMute();
}

void CaptureAudioSession::SetEncoderType(Audio::CodecType et)
{
	encoderType = et;
}

void CaptureAudioSession::SetRTPParams(std::string_view addr, uint16_t port)
{
	wsAddr.clear();
	accessToken.clear();
	wsDestAddr.clear();

	rtpSocket.SetDefaultAddress(addr, port);
}

void CaptureAudioSession::SetWSMParams(std::string_view addr, std::string_view accessToken_, std::string_view wsDestAddr_)
{
	wsAddr = addr;
	accessToken = accessToken_;
	wsDestAddr = wsDestAddr_;
}

std::string_view CaptureAudioSession::GetName() const
{
	return name;
}

uint32_t CaptureAudioSession::GetDeviceId() const
{
	return deviceId;
}

int CaptureAudioSession::GetBitrate() const
{
	return encoder.GetBitrate();
}

void CaptureAudioSession::SetLocalReceiver(Transport::ISocket* receiver)
{
	localReceiverSplitter.SetReceiver1(receiver);
}

void CaptureAudioSession::Start(uint32_t ssrc_, uint32_t deviceId_, std::string_view secureKey)
{
	if (runned)
	{
		return;
	}
	runned = true;

	ssrc = ssrc_;
	deviceId = deviceId_;

    resampler.SetSampleFreq(wui::config::get_int("CaptureDevices", "MicrophoneSampleFreq", 48000), 48000);

	if (audioRenderer)
	{
		audioRenderer->SetAECReceiver(aec.GetSpeakerReceiver());
	}
	
	if (wsAddr.empty())
	{
		encryptor.SetReceiver(&rtpSocket);
		rtpSocket.Start();
	}
	else
	{
		encryptor.SetReceiver(&wsmSocket);
		wsmSocket.Start(wsAddr, accessToken, wsDestAddr);
	}
	
	if (!secureKey.empty())
	{
		encryptor.Start(secureKey);
	}

	microphone.SetDeviceId(deviceId);
	microphone.Start();
	
	encoder.Start(encoderType, ssrc);
	/*if (!encoder.IsStarted())
	{
		errLog->info("Can't start microphone session because no memory, device id: {0:d}, ssrc: {1:d}", deviceId, ssrc);
		if (deviceNotifies)
		{
			deviceNotifies->ReceiveDeviceNotify(name, Client::DeviceNotifyType::MemoryError, Proto::dtMicrophone, deviceId, 0);
		}
		return;
	}*/
	
	aec.Start();

	sysLog->info("Started microphone session, device id: {0:d}, ssrc: {1:d}", deviceId, ssrc);
}

bool CaptureAudioSession::Started() const
{
	return runned;
}

void CaptureAudioSession::Stop()
{
	if (!runned)
	{
		return;
	}
	runned = false;

	if (audioRenderer)
	{
		audioRenderer->SetAECReceiver(nullptr);
	}
	
	microphone.Stop();
	aec.Stop();
	encoder.Stop();
	encryptor.Stop();
	rtpSocket.Stop();
	wsmSocket.Stop();
	
	sysLog->info("Stoped microphone session, device id: {0:d}, ssrc: {1:d}", deviceId, ssrc);
}

void CaptureAudioSession::Send(const Transport::IPacket &packet_, const Transport::Address *)
{
	/// Receive RTCP to set packet loss

	const Transport::RTCPPacket &packet = *static_cast<const Transport::RTCPPacket*>(&packet_);
	if (packet.rtcp_common.pt == Transport::RTCPPacket::RTCP_APP && packet.rtcps[0].r.app.messageType == Transport::RTCPPacket::amtStat)
	{
		encoder.SetPacketLoss(ntohl(*reinterpret_cast<const uint32_t*>(packet.rtcps[0].r.app.payload)));
	}
}

void CaptureAudioSession::SilentChanged(Audio::SilentMode silentMode)
{
	if (deviceNotifyCallback)
	{
        deviceNotifyCallback(name,
			silentMode == Audio::SilentMode::Speak ? Client::DeviceNotifyType::MicrophoneSpeak : Client::DeviceNotifyType::MicrophoneSilent,
			Proto::DeviceType::Microphone,
			deviceId,
			0);
	}
}

}
