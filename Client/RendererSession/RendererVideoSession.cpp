/**
 * RendererVideoSession.cpp - Contains impl of renderer video session
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014 - 2024
 *
 *                                                                       ,-> [Decoder] -> [Resizer] -> [JitterBuffer] <-> [VideoRenderer]
 * [NetSocket] -> [Decryptor] -> [vp8RTPCollector] -> [RecordSplitter] -<
 *                                                            ^          `-> [Recorder]
 *                                                            '- <- [Local Capturer]
 */

#include <Common/Common.h>

#include "RendererVideoSession.h"

namespace RendererSession
{

RendererVideoSession::RendererVideoSession(Common::TimeMeter &timeMeter)
	: renderer(std::make_shared<VideoRenderer::VideoRenderer>()),
	recorder(nullptr),
	recordSplitter(),
	jitterBuffer(timeMeter),
	resizer(),
	decoder(),
	vp8RTPCollector(),
	decryptor(),
	rtpSocket(),
	outSocket(&rtpSocket),
	wsmSocket(),
	pinger(),
	localCVS(),
	deviceNotifyCallback(),
	runned(false), my(false),
	receiverSSRC(0), authorSSRC(0),
	clientId(0), deviceId(0),
	secureKey(),
	name(),
	decoderType(Video::CodecType::VP8),
	frameRate(25),
	order(0),
	resolution(Video::rVGA),
	pingCnt(0),
	wsAddr(), accessToken(), wsDestAddr(),
	sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{
	renderer->SetResizeCallback(std::bind(&Video::Resizer::SetSize, &resizer, std::placeholders::_1, std::placeholders::_2));
	rtpSocket.SetReceiver(&decryptor, nullptr);
	wsmSocket.SetReceiver(&decryptor, nullptr);
	decryptor.SetReceiver(&vp8RTPCollector);
	vp8RTPCollector.SetReceiver(&recordSplitter);
	recordSplitter.SetReceiver0(&decoder);
	recordSplitter.SetReceiver1(nullptr);
    decoder.SetReceiver(&resizer);
	resizer.SetReceiver(&jitterBuffer);
	decoder.SetCallback(this);
	jitterBuffer.SetSlowRenderingCallback(std::bind(&RendererVideoSession::SlowRenderingCallback, this));
}

RendererVideoSession::~RendererVideoSession()
{
	Stop();
}

void RendererVideoSession::SetLocalCVS(CaptureSession::CaptureVideoSessionPtr_t localCVS_)
{
	localCVS = localCVS_;
}

void RendererVideoSession::SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback_)
{
    deviceNotifyCallback = deviceNotifyCallback_;
	renderer->SetDeviceNotifyCallback(deviceNotifyCallback_);
}

std::shared_ptr<wui::i_control> RendererVideoSession::GetControl()
{
    return renderer;
}

void RendererVideoSession::SetName(std::string_view name_)
{
	name = name_;
	renderer->SetName(name_);
}
std::string_view RendererVideoSession::GetName()
{
	return name;
}

void RendererVideoSession::SetDeviceType(Proto::DeviceType deviceType_)
{
    renderer->SetDeviceType(deviceType_);
}

Proto::DeviceType RendererVideoSession::GetDeviceType()
{
	return renderer->GetDeviceType();
}

void RendererVideoSession::SetMy(bool my_)
{
	my = my_;
}

bool RendererVideoSession::GetMy()
{
	return my;
}

Transport::ISocket* RendererVideoSession::GetDirectReceiver()
{
	return &recordSplitter;
}

void RendererVideoSession::SetResolution(Video::Resolution resolution_)
{
	resolution = resolution_;

	resizer.SetResolution(resolution);

	if (recorder)
	{
		recorder->ChangeVideoResolution(authorSSRC, resolution);
	}

	decoder.SetResolution(resolution);

	if (runned)
	{
		ForceKeyFrame(0);
	}
}

Video::Resolution RendererVideoSession::GetResolution()
{
	return resolution;
}

void RendererVideoSession::SetFrameRate(uint32_t rate)
{
	if (frameRate != rate)
	{
		frameRate = rate;
		jitterBuffer.SetFrameRate(rate);
	}
}

void RendererVideoSession::SetMirrorVideo(bool yes)
{
	resizer.SetMirror(yes);
}

void RendererVideoSession::SetDecoderType(Video::CodecType dt)
{
	decoderType = dt;
}

void RendererVideoSession::SetRTPParams(std::string_view recvFromAddr, uint16_t recvFromRTPPort)
{
	wsAddr.clear();
	accessToken.clear();
	wsDestAddr.clear();

	rtpSocket.SetDefaultAddress(recvFromAddr, recvFromRTPPort);

	outSocket = &rtpSocket;
}

void RendererVideoSession::SetWSMParams(std::string_view addr, std::string_view accessToken_, std::string_view wsDestAddr_)
{
	wsAddr = addr;
	accessToken = accessToken_;
	wsDestAddr = wsDestAddr_;

	outSocket = &wsmSocket;
}

void RendererVideoSession::SetRecorder(Recorder::Recorder* recorder_)
{
	recorder = recorder_;
	recordSplitter.SetReceiver1(recorder);
}

void RendererVideoSession::SetSpeak(bool speak_)
{
	renderer->SetSpeak(speak_);
}

void RendererVideoSession::SetOrder(uint32_t order_)
{
	order = order_;
}

uint32_t RendererVideoSession::GetOrder() const
{
	return order;
}

void RendererVideoSession::SetClientId(int64_t clientId_)
{
	clientId = clientId_;
}

int64_t RendererVideoSession::GetClientId() const
{
	return clientId;
}

void RendererVideoSession::Start(uint32_t receiverSSRC_, uint32_t authorSSRC_, uint32_t deviceId_, std::string_view secureKey_)
{
	if (runned)
	{
		return;
	}
	runned = true;

	renderer->SetId(deviceId_, clientId);
	renderer->SetName(name);

	renderer->Start(std::bind(&JB::JB::GetFrame, &jitterBuffer, std::placeholders::_1));

	resizer.Start();

	receiverSSRC = receiverSSRC_;
	authorSSRC = authorSSRC_;
	deviceId = deviceId_;
	secureKey = secureKey_;

	if (!my && !secureKey.empty())
	{
		decryptor.Start(secureKey);
	}

	if (recorder)
	{
		recorder->AddVideo(authorSSRC, clientId, GetDeviceType() == Proto::DeviceType::Demonstration ? 1 : 0, resolution, this);
	}

	vp8RTPCollector.Reset();

	if (GetDeviceType() != Proto::DeviceType::Avatar)
	{
		jitterBuffer.Start(JB::Mode::video);
		if (!jitterBuffer.IsStarted())
		{
			errLog->info("Can't start video renderer session because no memory to jitter buffer, client id: {0:d}, device id: {0:1}, receiver ssrc: {2:d}, author ssrc: {3:d}", clientId, deviceId, receiverSSRC, authorSSRC);
			if (deviceNotifyCallback)
			{
				deviceNotifyCallback(name, Client::DeviceNotifyType::MemoryError, Proto::DeviceType::VideoRenderer, deviceId, 0);
			}
		}

		decoder.Start(decoderType, Video::ColorSpace::RGB32);

		if (!decoder.IsStarted())
		{
			errLog->info("Can't start video renderer session because no memory to decoder, client id: {0:d}, device id: {0:1}, receiver ssrc: {2:d}, author ssrc: {3:d}", clientId, deviceId, receiverSSRC, authorSSRC);
			if (deviceNotifyCallback)
			{
                deviceNotifyCallback(name, Client::DeviceNotifyType::MemoryError, Proto::DeviceType::VideoRenderer, deviceId, 0);
			}
            return;
		}
	}

	if (!my && GetDeviceType() != Proto::DeviceType::Avatar)
	{
		recordSplitter.SetReceiver0(&decoder);
        
        StartRemote();

		pingCnt = 80;
		pinger = std::thread(&RendererVideoSession::EstablishConnection, this);
	}

	ForceKeyFrame(0);

	sysLog->info("Started video renderer session, client id: {0:d}, device id: {0:1}, receiver ssrc: {2:d}, author ssrc: {3:d}", clientId, deviceId, receiverSSRC, authorSSRC);
}

bool RendererVideoSession::Started() const
{
	return runned;
}

void RendererVideoSession::Stop()
{
	if (!runned)
	{
		return;
	}

	runned = false;

	if (pinger.joinable()) pinger.join();

	StopRemote();

	decryptor.Stop();
	decoder.Stop();
	renderer->Stop();
	resizer.Stop();

	if (recorder)
	{
		recorder->DeleteVideo(authorSSRC);
	}

	sysLog->info("Stoped video renderer session, client id: {0:d}, device id: {1:d}, receiver ssrc: {2:d}, author ssrc: {3:d}", clientId, deviceId, receiverSSRC, authorSSRC);
}

void RendererVideoSession::StartRemote()
{
	if (wsAddr.empty())
	{
		rtpSocket.Start();
	}
	else
	{
		wsmSocket.Start(wsAddr, accessToken, wsDestAddr);
	}
}

void RendererVideoSession::StopRemote()
{
	rtpSocket.Stop();
	wsmSocket.Stop();
}

void RendererVideoSession::Pause()
{
	if (GetDeviceType() == Proto::DeviceType::Avatar)
	{
		return;
	}

	if (!my)
	{
		StopRemote();
	}
	else
	{
		recordSplitter.SetReceiver0(nullptr);
	}

	sysLog->info("Paused video renderer session, client id: {0:d}, device id: {0:1}, receiver ssrc: {2:d}, author ssrc: {3:d}", clientId, deviceId, receiverSSRC, authorSSRC);
}

void RendererVideoSession::Resume()
{
	if (GetDeviceType() == Proto::DeviceType::Avatar)
	{
		return;
	}

	if (!my)
	{
		StartRemote();
	}
	else
	{
		recordSplitter.SetReceiver0(&decoder);
	}

	ForceKeyFrame(0);

	sysLog->info("Resumed video renderer session, client id: {0:d}, device id: {0:1}, receiver ssrc: {2:d}, author ssrc: {3:d}", clientId, deviceId, receiverSSRC, authorSSRC);
}

uint32_t RendererVideoSession::GetDeviceId() const
{
	return deviceId;
}

uint32_t RendererVideoSession::GetSSRC() const
{
	return receiverSSRC;
}

uint32_t RendererVideoSession::GetAuthorSSRC() const
{
	return authorSSRC;
}

std::string RendererVideoSession::GetSecureKey() const
{
	return secureKey;
}

void RendererVideoSession::ForceKeyFrame(uint32_t lastRecvSeq)
{
	if (!my)
	{
		Transport::RTCPPacket fkfPacket;

		fkfPacket.rtcp_common.pt = Transport::RTCPPacket::RTCP_APP;
		fkfPacket.rtcp_common.length = 1;
		fkfPacket.rtcps[0].r.app.ssrc = receiverSSRC;
		fkfPacket.rtcps[0].r.app.messageType = Transport::RTCPPacket::amtForceKeyFrame;
		*reinterpret_cast<uint32_t*>(fkfPacket.rtcps[0].r.app.payload) = htonl(lastRecvSeq);
		
		outSocket->Send(fkfPacket);

		sysLog->trace("Video renderer (device id: {0:d}) send forcing key frame (seq: {1:d})", deviceId, lastRecvSeq);
	}
	else if (localCVS)
	{
		localCVS->ForceKeyFrame();
	}
}

void RendererVideoSession::EstablishConnection()
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

void RendererVideoSession::SetRemoteFrameRate(uint32_t rate)
{
	if (!my)
	{
		Transport::RTCPPacket fkfPacket;

		fkfPacket.rtcp_common.pt = Transport::RTCPPacket::RTCP_APP;
		fkfPacket.rtcp_common.length = 1;
		fkfPacket.rtcps[0].r.app.ssrc = receiverSSRC;
		fkfPacket.rtcps[0].r.app.messageType = Transport::RTCPPacket::amtSetFrameRate;
		*reinterpret_cast<uint32_t*>(fkfPacket.rtcps[0].r.app.payload) = htonl(rate);

		outSocket->Send(fkfPacket);
	}
	else if (localCVS)
	{
		localCVS->SetFrameRate(rate);
	}
	jitterBuffer.SetFrameRate(rate);
}

void RendererVideoSession::SlowRenderingCallback()
{
	DBGTRACE("Video renderer[%d] too slow rendering\n", deviceId);

	if (frameRate == 25)
	{
		SetRemoteFrameRate(5);
		return;
	}

	auto rv = Video::GetValues(resolution);

	if (rv.height > 120)
	{
		Transport::RTCPPacket rtcpPacket;

		rtcpPacket.rtcp_common.pt = Transport::RTCPPacket::RTCP_APP;
		rtcpPacket.rtcp_common.length = 1;
		rtcpPacket.rtcps[0].r.app.ssrc = receiverSSRC;
		rtcpPacket.rtcps[0].r.app.messageType = Transport::RTCPPacket::amtReduceComplexity;
		
		outSocket->Send(rtcpPacket);
	}
	else
	{
		if (deviceNotifyCallback)
		{
            deviceNotifyCallback(name, Client::DeviceNotifyType::OvertimeRendering, Proto::DeviceType::VideoRenderer, deviceId, 0);
		}
	}
}

void RendererVideoSession::SendRCAction(Transport::RTCPPacket::RemoteControlAction rca, int32_t x, int32_t y)
{
	Transport::RTCPPacket rtcpPacket;

	rtcpPacket.rtcp_common.pt = Transport::RTCPPacket::RTCP_APP;
	rtcpPacket.rtcp_common.length = 1;
	rtcpPacket.rtcps[0].r.app.ssrc = receiverSSRC;
	rtcpPacket.rtcps[0].r.app.messageType = Transport::RTCPPacket::amtRemoteControl;
	*reinterpret_cast<uint16_t*>(rtcpPacket.rtcps[0].r.app.payload) = htons(rca);
	*reinterpret_cast<uint16_t*>(rtcpPacket.rtcps[0].r.app.payload + 2) = htons(x);
	*reinterpret_cast<uint16_t*>(rtcpPacket.rtcps[0].r.app.payload + 4) = htons(y);

	outSocket->Send(rtcpPacket);
}

void RendererVideoSession::SendRCAction(Transport::RTCPPacket::RemoteControlAction rca, uint8_t modifier, const char* key, uint8_t key_size)
{
	Transport::RTCPPacket rtcpPacket;

	rtcpPacket.rtcp_common.pt = Transport::RTCPPacket::RTCP_APP;
	rtcpPacket.rtcp_common.length = 1;
	rtcpPacket.rtcps[0].r.app.ssrc = receiverSSRC;
	rtcpPacket.rtcps[0].r.app.messageType = Transport::RTCPPacket::amtRemoteControl;
	*reinterpret_cast<uint16_t*>(rtcpPacket.rtcps[0].r.app.payload) = htons(rca);
	*reinterpret_cast<uint16_t*>(rtcpPacket.rtcps[0].r.app.payload + 2) = htons(modifier);
	*reinterpret_cast<uint32_t*>(rtcpPacket.rtcps[0].r.app.payload + 4) = htonl(*reinterpret_cast<const int32_t*>(key));

	outSocket->Send(rtcpPacket);
}

void RendererVideoSession::MouseMove(int32_t x, int32_t y)
{
	SendRCAction(Transport::RTCPPacket::rcaMove, x, y);
}

void RendererVideoSession::MouseLeftDown(int32_t x, int32_t y)
{
	SendRCAction(Transport::RTCPPacket::rcaLeftDown, x, y);
}

void RendererVideoSession::MouseLeftUp(int32_t x, int32_t y)
{
	SendRCAction(Transport::RTCPPacket::rcaLeftUp, x, y);
}

void RendererVideoSession::MouseCenterDown(int32_t x, int32_t y)
{
	SendRCAction(Transport::RTCPPacket::rcaCenterDown, x, y);
}

void RendererVideoSession::MouseCenterUp(int32_t x, int32_t y)
{
	SendRCAction(Transport::RTCPPacket::rcaCenterUp, x, y);
}

void RendererVideoSession::MouseRightDown(int32_t x, int32_t y)
{
	SendRCAction(Transport::RTCPPacket::rcaRightDown, x, y);
}

void RendererVideoSession::MouseRightUp(int32_t x, int32_t y)
{
	SendRCAction(Transport::RTCPPacket::rcaRightUp, x, y);
}

void RendererVideoSession::MouseRightDblClick(int32_t x, int32_t y)
{
	SendRCAction(Transport::RTCPPacket::rcaRightDblClick, x, y);
}

void RendererVideoSession::MouseLeftDblClick(int32_t x, int32_t y)
{
	SendRCAction(Transport::RTCPPacket::rcaLeftDblClick, x, y);
}

void RendererVideoSession::MouseWheel(int32_t delta)
{
	SendRCAction(Transport::RTCPPacket::rcaWheel, delta, 0);
}

void RendererVideoSession::KeyDown(uint8_t modifier, const char* key, uint8_t key_size)
{
	SendRCAction(Transport::RTCPPacket::rcaKeyDown, modifier, key, key_size);
}

void RendererVideoSession::KeyUp(uint8_t modifier, const char* key, uint8_t key_size)
{
	SendRCAction(Transport::RTCPPacket::rcaKeyUp, modifier, key, key_size);
}

}
