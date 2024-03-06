/**
 * RendererAudioSession.cpp - Contains impl of renderer audio session
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014 - 2024
 * 
 *                                                  ,-> [Decoder] -> [JitterBuffer] <-> [AudioMixer] <- [AudioRenderer]
 * [NetSocket] -> [Decryptor] -> [RecordSplitter] -<
 *                                       ^          `-> [Recorder]
 *                                       '- <- [Local Capturer]
 */

#include "RendererAudioSession.h"

#include <Transport/RTP/RTCPPacket.h>

#include <wui/config/config.hpp>

#include <Common/Common.h>

namespace RendererSession
{

RendererAudioSession::RendererAudioSession(Common::TimeMeter &timeMeter_, Audio::AudioMixer &audioMixer_)
	: deviceNotifyCallback(),
	audioMixer(audioMixer_),
	recorder(nullptr),
	videoRecorder(nullptr),
	recordSplitter(),
	decryptor(),
	decoder(),
	jitterBuffer(timeMeter_),
	rtpSocket(),
	wsmSocket(),
	outSocket(&rtpSocket),
	pinger(),
	runned(false), my(false), mute(false),
	volume(100),
	name(),
	metadata(),
	decoderType(Audio::CodecType::Opus),
	receiverSSRC(0), authorSSRC(0),
	clientId(0), deviceId(0),
	secureKey(),
	pingCnt(0),
	lastPacketLoss(0),
	wsAddr(), accessToken(), wsDestAddr(),
	sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{
    rtpSocket.SetReceiver(&decryptor, nullptr);
	wsmSocket.SetReceiver(&decryptor, nullptr);
	decryptor.SetReceiver(&recordSplitter);
	recordSplitter.SetReceiver0(&decoder);
	decoder.SetReceiver(&jitterBuffer);
}

RendererAudioSession::~RendererAudioSession()
{
	Stop();
}

void RendererAudioSession::SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback_)
{
    deviceNotifyCallback = deviceNotifyCallback_;
}

void RendererAudioSession::SetVolume(int vol)
{
	volume = vol;
	audioMixer.SetInputVolume(authorSSRC, vol);
}

void RendererAudioSession::SetMute(bool yes)
{
	mute = yes;
	recordSplitter.SetReceiver0(yes ? nullptr : &decoder);
}

bool RendererAudioSession::GetMute()
{
	return mute;
}

void RendererAudioSession::SetDecoderType(Audio::CodecType dt)
{
	decoderType = dt;
}

void RendererAudioSession::SetName(std::string_view name_)
{
	name = name_;
}
std::string_view RendererAudioSession::GetName()
{
	return name;
}

void RendererAudioSession::SetMetadata(std::string_view metadata_)
{
	metadata = metadata_;
}
std::string_view RendererAudioSession::GetMetadata()
{
	return metadata;
}

void RendererAudioSession::SetMy(bool my_)
{
	my = my_;
}
bool RendererAudioSession::GetMy()
{
	return my;
}

Transport::ISocket* RendererAudioSession::GetDirectReceiver()
{
	return &recordSplitter;
}

void RendererAudioSession::SetRTPParams(std::string_view recvFromAddr, uint16_t recvFromRTPPort)
{
	wsAddr.clear();
	accessToken.clear();
	wsDestAddr.clear();

	rtpSocket.SetDefaultAddress(recvFromAddr, recvFromRTPPort);

	outSocket = &rtpSocket;
}

void RendererAudioSession::SetWSMParams(std::string_view addr, std::string_view accessToken_, std::string_view wsDestAddr_)
{
	wsAddr = addr;
	accessToken = accessToken_;
	wsDestAddr = wsDestAddr_;

	outSocket = &wsmSocket;
}

void RendererAudioSession::SetRecorder(Recorder::Recorder* recorder_)
{
	recorder = recorder_;
	recordSplitter.SetReceiver1(recorder);
}

void RendererAudioSession::SetClientId(int64_t clientId_)
{
	clientId = clientId_;
}

int64_t RendererAudioSession::GetClientId() const
{
	return clientId;
}

uint32_t RendererAudioSession::GetDeviceId() const
{
	return deviceId;
}

uint32_t RendererAudioSession::GetSSRC() const
{
	return receiverSSRC;
}

uint32_t RendererAudioSession::GetAuthorSSRC() const
{
	return authorSSRC;
}

std::string RendererAudioSession::GetSecureKey() const
{
	return secureKey;
}

void RendererAudioSession::Start(uint32_t receiverSSRC_, uint32_t authorSSRC_, uint32_t deviceId_, std::string_view secureKey_)
{
	if (runned)
	{
		return;
	}
	runned = true;

	receiverSSRC = receiverSSRC_;
	authorSSRC = authorSSRC_;
	deviceId = deviceId_;
	secureKey = secureKey_;

	lastPacketLoss = 0;

	if (!secureKey.empty())
	{
		decryptor.Start(secureKey);
	}

	if (recorder)
	{
		recorder->AddAudio(authorSSRC, clientId);
	}

	decoder.Start(decoderType);
	if (!decoder.IsStarted())
	{
		errLog->info("Can't start audio renderer session because no memory to decoder, client id: {0:d}, device id: {0:1}, receiver ssrc: {2:d}, author ssrc: {3:d}", clientId, deviceId, receiverSSRC, authorSSRC);
		if (deviceNotifyCallback)
		{
			deviceNotifyCallback(name, Client::DeviceNotifyType::MemoryError, Proto::DeviceType::AudioRenderer, deviceId, 0);
		}
		return;
	}

	if (!my)
	{
		if (wsAddr.empty())
		{
			rtpSocket.Start();
		}
		else
		{
			wsmSocket.Start(wsAddr, accessToken, wsDestAddr);
		}
	
		jitterBuffer.Start(JB::Mode::Sound, name);
		if (!jitterBuffer.IsStarted())
		{
			errLog->info("Can't start audio renderer session because no memory to jitter buffer, client id: {0:d}, device id: {0:1}, receiver ssrc: {2:d}, author ssrc: {3:d}", clientId, deviceId, receiverSSRC, authorSSRC);
			if (deviceNotifyCallback)
			{
				deviceNotifyCallback(name, Client::DeviceNotifyType::MemoryError, Proto::DeviceType::VideoRenderer, deviceId, 0);
			}
			return;
		}

		pingCnt = 80;
		pinger = std::thread(&RendererAudioSession::EstablishConnection, this);

		audioMixer.AddInput(authorSSRC, clientId, std::bind(&JB::JB::GetFrame, &jitterBuffer, std::placeholders::_1));
	}
	
	sysLog->info("Started audio renderer session, client id: {0:d}, device id: {0:1}, receiver ssrc: {2:d}, author ssrc: {3:d}", clientId, deviceId, receiverSSRC, authorSSRC);
}

bool RendererAudioSession::Started() const
{
	return runned;
}

void RendererAudioSession::Stop()
{
    if (!runned)
	{
		return;
	}
	runned = false;

	if (pinger.joinable()) pinger.join();

    rtpSocket.Stop();
	wsmSocket.Stop();
    jitterBuffer.Stop();
    decoder.Stop();
    decryptor.Stop();
    audioMixer.DeleteInput(authorSSRC);

	if (recorder)
	{
		recorder->DeleteAudio(authorSSRC);
	}

    sysLog->info("Stoped audio renderer session, client id: {0:d}, device id: {1:d}, receiver ssrc: {2:d}, author ssrc: {3:d}", clientId, deviceId, receiverSSRC, authorSSRC);
}

void RendererAudioSession::Pause()
{
	if (!my)
	{
		rtpSocket.Stop();
	}
	else
	{
		decoder.Stop();
	}
}

void RendererAudioSession::Resume()
{
	if (!my)
	{
		rtpSocket.Start();
	}
	else
	{
		decoder.Start(decoderType);
	}
}

/*void RendererAudioSession::ReceiveStreamParams(uint32_t jitter, uint32_t packetLoss)
{
	DBGTRACE("Audio renderer[%d] jitter: %d, packet loss: %d\n", deviceId, jitter, packetLoss);
	
	if (lastPacketLoss != packetLoss)
	{
		Transport::RTCPPacket statPacket;

		statPacket.rtcp_common.pt = Transport::RTCPPacket::RTCP_APP;
		statPacket.rtcp_common.length = 1;
		statPacket.rtcps[0].r.app.ssrc = receiverSSRC;
		statPacket.rtcps[0].r.app.messageType = Transport::RTCPPacket::amtStat;
		*reinterpret_cast<uint32_t*>(statPacket.rtcps[0].r.app.payload) = htonl(packetLoss);
		*reinterpret_cast<uint32_t*>(statPacket.rtcps[0].r.app.payload + 4) = htonl(jitter);

		std::lock_guard<std::mutex> lock(socketMutex);
		rtpSocket.Send(statPacket);
	}
	lastPacketLoss = packetLoss;
}*/

void RendererAudioSession::EstablishConnection()
{
	while (runned)
	{
		if (pingCnt++ >= 80)
		{
			Transport::RTPPacket packet;
			packet.rtpHeader.ssrc = receiverSSRC;

			outSocket->Send(packet);

			pingCnt = 0;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}

}
