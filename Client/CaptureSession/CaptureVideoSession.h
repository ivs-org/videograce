/**
 * CaptureVideoSession.h - Contains impl interface of capture video session
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <memory>

#include "ICaptureVideoSession.h"

#include <Transport/RTPSocket.h>
#include <Transport/SocketSplitter.h>

#include <Common/TimeMeter.h>

#include <Camera/ICamera.h>
#include <Video/VideoEncoder.h>
#include <Video/VP8RTPSplitter.h>

#include <Crypto/Encryptor.h>

#include <spdlog/spdlog.h>

namespace CaptureSession
{
	class CaptureVideoSession : public ICaptureVideoSession, public Transport::ISocket
	{
	public:
		CaptureVideoSession(Common::TimeMeter &timeMeter);
		~CaptureVideoSession();
		
        void SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback);

		/// derived from ICaptureVideoSession
		virtual void SetDeviceType(Proto::DeviceType deviceType);
		virtual void SetName(std::string_view name);
		virtual void SetResolution(Video::Resolution resolution);
		virtual void SetBitrate(int bitrate);
		virtual void SetFrameRate(uint32_t rate);
		virtual void SetEncoderType(Video::CodecType et);
		virtual void SetRCActions(bool yes);
		virtual bool IsRCActionsEnabled() const;
		virtual void ForceKeyFrame();
		virtual void SetRTPParams(const char* addr, uint16_t rtpPort);
		virtual std::string_view GetName() const;
		virtual uint32_t GetDeviceId();
		virtual Proto::DeviceType GetDeviceType();
		virtual Video::Resolution GetResolution();
		virtual int GetBitrate();
		virtual void SetLocalReceiver(Transport::ISocket* receiver);
		virtual void Start(uint32_t ssrc, Video::ColorSpace colorSpace, uint32_t deviceId, std::string_view secureKey);
		virtual bool Started() const;
		virtual void Stop();
		virtual void Pause();
		virtual void Resume();
		virtual void Move(Camera::MoveAxis axis, Camera::MoveType type, int value);
		virtual void Zoom(int value);

		/// derived from Transport::ISocket (receive RTCP only)
		virtual void Send(const Transport::IPacket &packet, const Transport::Address *address = nullptr);
	private:
		Transport::RTPSocket rtpSocket;
		Transport::SocketSplitter localReceiverSplitter;
		Video::VP8RTPSplitter vp8RTPSplitter;
		Crypto::Encryptor encryptor;
		Video::Encoder encoder;
		std::unique_ptr<Camera::ICamera> camera;
		Common::TimeMeter &timeMeter;
		Client::DeviceNotifyCallback deviceNotifyCallback;

		bool runned;
		
		uint32_t ssrc, deviceId;

		uint32_t frameRate;

		Proto::DeviceType deviceType;
		std::string name;
		Video::CodecType encoderType;
		Video::Resolution resolution;
		Video::ColorSpace colorSpace;

		bool rcActionsEnabled;
				
		std::shared_ptr<spdlog::logger> sysLog, errLog;

		void ReduceComplexity();
	};

	typedef std::shared_ptr<CaptureVideoSession> CaptureVideoSessionPtr_t;
}
