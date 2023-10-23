/**
 * RendererVideoSession.h - Contains impl interface of renderer video session
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <memory>
#include <thread>

#include "IRendererVideoSession.h"

#include <Transport/RTPSocket.h>
#include <Transport/SocketSplitter.h>

#include <Video/VideoDecoder.h>
#include <Video/VP8RTPCollector.h>

#include <VideoRenderer/VideoRenderer.h>

#include <JitterBuffer/JB.h>

#include <Record/Recorder.h>

#include <Crypto/Decryptor.h>

#include <CaptureSession/CaptureVideoSession.h>

#include <spdlog/spdlog.h>

namespace Client
{
	class IDeviceNotifies;
}

namespace RendererSession
{

class RendererVideoSession : public IRendererVideoSession, public Video::IPacketLossCallback
{
public:
	RendererVideoSession(Common::TimeMeter &timeMeter);
	~RendererVideoSession();

	void SetLocalCVS(CaptureSession::CaptureVideoSessionPtr_t localCVS);
	void SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback);

	/// derived from IRendererVideoSession
    virtual std::shared_ptr<wui::i_control> GetControl();

	virtual void SetName(const std::string &name);
	virtual const std::string &GetName();
	virtual void SetDeviceType(Proto::DeviceType deviceType);
	virtual Proto::DeviceType GetDeviceType();
	virtual void SetMy(bool yes);
	virtual bool GetMy();
	virtual Transport::ISocket* GetDirectReceiver();
	virtual void SetResolution(Video::Resolution resolution);
	virtual Video::Resolution GetResolution();
	virtual void SetFrameRate(uint32_t rate);
	virtual void SetMirrorVideo(bool yes);
	virtual bool GetVideoMirrored() const;
	virtual void SetDecoderType(Video::CodecType dt);
	virtual void SetRTPParams(const char* recvFromAddr, uint16_t recvFromRTPPort);
	virtual void SetRecorder(Recorder::Recorder* recorder);
	virtual void SetSpeak(bool speak);
	virtual void SetOrder(uint32_t order);
	virtual void SetClientId(int64_t clientId);
	virtual int64_t GetClientId() const;
	virtual uint32_t GetOrder() const;
	virtual uint32_t GetDeviceId() const;
	virtual uint32_t GetSSRC() const;
	virtual uint32_t GetAuthorSSRC() const;
	virtual std::string GetSecureKey() const;
	virtual void Start(uint32_t receiverSSRC, uint32_t authorSSRC, uint32_t deviceId, const std::string &secureKey);
	virtual bool Started() const;
	virtual void Stop();
	virtual void Pause();
	virtual void Resume();

	/// derived from Video::IPacketLossCallback
	virtual void ForceKeyFrame(uint32_t lastRecvSeq);

	/// Receive from JitterBuffer
	void SlowRenderingCallback();

private:	
	std::shared_ptr<VideoRenderer::VideoRenderer> renderer;
	Recorder::Recorder *recorder;
	Transport::SocketSplitter recordSplitter;
	JB::JB jitterBuffer;
	Video::Decoder decoder;
	Video::VP8RTPCollector vp8RTPCollector;
	Crypto::Decryptor decryptor;
	Transport::RTPSocket rtpSocket;
	std::thread pinger;
	CaptureSession::CaptureVideoSessionPtr_t localCVS;
    Client::DeviceNotifyCallback deviceNotifyCallback;

	bool runned, my;
	uint32_t receiverSSRC, authorSSRC; 
	int64_t clientId;
	uint32_t deviceId;
	std::string secureKey;
	std::string name;
	Video::CodecType decoderType;
	uint32_t frameRate;
	uint32_t order;

	Video::Resolution resolution;

	uint16_t pingCnt;

	std::shared_ptr<spdlog::logger> sysLog, errLog;
	
	void EstablishConnection();
 
	void StartRemote();
	void StopRemote();

	void SetRemoteFrameRate(uint32_t rate);
};

typedef std::shared_ptr<RendererVideoSession> RendererVideoSessionPtr_t;
}
