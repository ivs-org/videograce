/**
 * CaptureVideoSession.cpp - Contains impl of capture video session
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014, 2024
 * 
 *                                                    ,-> [Encoder] -> [vp8RTPSplitter] -> [Encryptor] -> [NetSocket]
 * [Camera] -> [Encoder] -> [LocalReceiverSplitter] -<
 *                                                    '-> [LocalRenderer]
 */

#include <Camera/Camera.h>
#include <Camera/ScreenCapturer.h>

#include "CaptureVideoSession.h"

#include <Transport/RTP/RTCPPacket.h>

namespace CaptureSession
{

CaptureVideoSession::CaptureVideoSession(Common::TimeMeter &timeMeter_)
	: rtpSocket(),
	wsmSocket(),
	localReceiverSplitter(),
	vp8RTPSplitter(),
	encryptor(),
	encoder(),
	camera(),
	timeMeter(timeMeter_),
	deviceNotifyCallback(),
	runned(false),
	ssrc(0), deviceId(0),
	frameRate(25),
	deviceType(Proto::DeviceType::Camera),
	name(),
	encoderType(Video::CodecType::VP8),
	resolution(Video::rUndefined),
	colorSpace(Video::ColorSpace::Unsupported),
	rcActionsEnabled(false),
	wsAddr(), accessToken(), wsDestAddr(),
	sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{
	encoder.SetReceiver(&localReceiverSplitter);
	localReceiverSplitter.SetReceiver0(&vp8RTPSplitter);
	vp8RTPSplitter.SetReceiver(&encryptor);
	encryptor.SetReceiver(&rtpSocket);
	rtpSocket.SetReceiver(nullptr, this);
	wsmSocket.SetReceiver(nullptr, this);
}

CaptureVideoSession::~CaptureVideoSession()
{
	Stop();

	camera.reset(nullptr);
}

void CaptureVideoSession::SetDeviceType(Proto::DeviceType deviceType_)
{
	deviceType = deviceType_;
}

void CaptureVideoSession::SetName(std::string_view name_)
{
	name = name_;
	if (camera)
	{
		camera->SetName(name_);
	}
}

void CaptureVideoSession::SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback_)
{
    deviceNotifyCallback = deviceNotifyCallback_;
	if (camera)
	{
		camera->SetDeviceNotifyCallback(deviceNotifyCallback);
	}
}

void CaptureVideoSession::SetResolution(Video::Resolution resolution_)
{
	resolution = resolution_;

	if (runned)
	{
		camera->Stop();
	}

	encoder.SetResolution(resolution);
	
	if (camera)
	{
		camera->SetResolution(resolution);
	}

	if (runned)
	{
		camera->Start(colorSpace, ssrc);
	}
}

void CaptureVideoSession::SetBitrate(int bitrate)
{
	encoder.SetBitrate(bitrate);
}

void CaptureVideoSession::SetFrameRate(uint32_t rate)
{
	frameRate = rate;
	if (camera)
	{
		camera->SetFrameRate(rate);
	}
}

void CaptureVideoSession::SetEncoderType(Video::CodecType et)
{
	encoderType = et;
	if (runned)
	{
		encoder.Stop();
		encoder.Start(et);
	}
}

void CaptureVideoSession::SetRCActions(bool yes)
{
	rcActionsEnabled = yes;

	if (camera)
	{
		static_cast<Camera::ScreenCapturer*>(camera.get())->SetRCMode(yes);
	}
}

bool CaptureVideoSession::IsRCActionsEnabled() const
{
	return rcActionsEnabled;
}

void CaptureVideoSession::ForceKeyFrame()
{
	encoder.ForceKeyFrame(0);
}

void CaptureVideoSession::SetRTPParams(std::string_view addr, uint16_t port)
{
	wsAddr.clear();
	accessToken.clear();
	wsDestAddr.clear();

	rtpSocket.SetDefaultAddress(addr, port);
}

void CaptureVideoSession::SetWSMParams(std::string_view addr, std::string_view accessToken_, std::string_view wsDestAddr_)
{
	wsAddr = addr;
	accessToken = accessToken_;
	wsDestAddr = wsDestAddr_;
}

std::string_view CaptureVideoSession::GetName() const
{
	return name;
}

uint32_t CaptureVideoSession::GetDeviceId()
{
	return deviceId;
}

Proto::DeviceType CaptureVideoSession::GetDeviceType()
{
	return deviceType;
}

Video::Resolution CaptureVideoSession::GetResolution()
{
	return resolution;
}

int CaptureVideoSession::GetBitrate()
{
	return encoder.GetBitrate();
}

void CaptureVideoSession::SetLocalReceiver(Transport::ISocket* receiver)
{
	localReceiverSplitter.SetReceiver1(receiver);
}

void CaptureVideoSession::Start(uint32_t ssrc_, Video::ColorSpace colorSpace_, uint32_t deviceId_, std::string_view secureKey)
{
	if (runned)
	{
		return;
	}
	runned = true;

	ssrc = ssrc_;
	deviceId = deviceId_;
	colorSpace = colorSpace_;

	camera.reset(nullptr);

	switch (deviceType)
	{
		case Proto::DeviceType::Camera:
			camera = std::unique_ptr<Camera::ICamera>(new Camera::Camera(timeMeter, encoder));
		break;
		case Proto::DeviceType::Demonstration:
			camera = std::unique_ptr<Camera::ICamera>(new Camera::ScreenCapturer(timeMeter, encoder));
			encoder.SetScreenContent(true);
		break;
	}

	camera->SetName(name);
	camera->SetDeviceId(deviceId);
	camera->SetDeviceNotifyCallback(deviceNotifyCallback);
	camera->SetResolution(resolution);
	camera->SetFrameRate(frameRate);

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

	vp8RTPSplitter.Reset();
	
	if (!secureKey.empty())
	{
		encryptor.Start(secureKey);
	}

	encoder.Start(encoderType);
	if (!encoder.IsStarted())
	{
		errLog->info("Can't start camera encoder because no memory, device id: {0:d}, ssrc: {1:d}", deviceId, ssrc);
		if (deviceNotifyCallback)
		{
            deviceNotifyCallback(name, Client::DeviceNotifyType::MemoryError, Proto::DeviceType::Camera, deviceId, 0);
		}
		return;
	}

	camera->Start(colorSpace, ssrc);
	
	sysLog->trace("Started camera session, device id: {0:d}, ssrc: {1:d}", deviceId, ssrc);
}

bool CaptureVideoSession::Started() const
{
	return runned;
}

void CaptureVideoSession::Stop()
{	
	if (!runned)
	{
		return;
	}
	runned = false;

	camera->Stop();
	encoder.Stop();
	encryptor.Stop();
	rtpSocket.Stop();

	sysLog->trace("Stoped camera session, device id: {0:d}, ssrc: {1:d}", deviceId, ssrc);
}

void CaptureVideoSession::Pause()
{
	if (runned)
	{
		camera->Stop();
	}
}

void CaptureVideoSession::Resume()
{
	if (runned)
	{
		camera->Start(colorSpace, ssrc);
	}
}

void CaptureVideoSession::Move(Camera::MoveAxis axis, Camera::MoveType type, int value)
{
	camera->Move(axis, type, value);
}

void CaptureVideoSession::Zoom(int value)
{
	camera->Zoom(value);
}

void CaptureVideoSession::Send(const Transport::IPacket &packet_, const Transport::Address *)
{
	/// Receive RTCP to force key frame and ARC

	const Transport::RTCPPacket &packet = *static_cast<const Transport::RTCPPacket*>(&packet_);
	if (packet.rtcp_common.pt == Transport::RTCPPacket::RTCP_APP)
	{
		switch (packet.rtcps[0].r.app.messageType)
		{
			case Transport::RTCPPacket::amtForceKeyFrame:
			{
				auto lastRecvSeq = ntohl(*reinterpret_cast<const uint32_t*>(packet.rtcps[0].r.app.payload));
				sysLog->trace("Camera[{0}] :: Received force key frame (last_recv_seq: {1})", name, lastRecvSeq);
				encoder.ForceKeyFrame(lastRecvSeq);
			}
			break;
			case Transport::RTCPPacket::amtReduceComplexity:
				ReduceComplexity();
				sysLog->trace("Camera[{0}] :: Received ReduceComplexity", name);
			break;
			case Transport::RTCPPacket::amtSetFrameRate:
			{
				auto fr = ntohl(*reinterpret_cast<const uint32_t*>(packet.rtcps[0].r.app.payload));
				SetFrameRate(fr);
				sysLog->trace("Camera[{0}] :: Received SetFrameRate", name, fr);
			}
			break;
			case Transport::RTCPPacket::amtRemoteControl:
				if (rcActionsEnabled)
				{
					if (static_cast<Transport::RTCPPacket::RemoteControlAction>(ntohs(*reinterpret_cast<const uint16_t*>(packet.rtcps[0].r.app.payload))) == Transport::RTCPPacket::rcaKeyDown ||
						static_cast<Transport::RTCPPacket::RemoteControlAction>(ntohs(*reinterpret_cast<const uint16_t*>(packet.rtcps[0].r.app.payload))) == Transport::RTCPPacket::rcaKeyUp)
					{
						static_cast<Camera::ScreenCapturer*>(camera.get())->MakeKeyboardAction(packet.rtcps[0].r.app.payload);
					}
					else
					{
						static_cast<Camera::ScreenCapturer*>(camera.get())->MakeMouseAction(packet.rtcps[0].r.app.payload);
					}
				}
			break;
			default:
			break;
		}
	}
}

void CaptureVideoSession::ReduceComplexity()
{
	if (deviceNotifyCallback)
	{
        deviceNotifyCallback(name, Client::DeviceNotifyType::OvertimeCoding, Proto::DeviceType::Camera, deviceId, 0);
	}
}

}
