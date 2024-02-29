/**
 * RendererAudioSession.h - Contains impl interface of renderer audio session
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014, 2024
 *  
 *                                                  ,-> [Decoder] -> [JitterBuffer] <-> [AudioMixer] <- [AudioRenderer]
 * [NetSocket] -> [Decryptor] -> [RecordSplitter] -<
 *                                                  `-> [RecordBuffer]
 * 
 */

#pragma once

#include <thread>

#include "IRendererAudioSession.h"

#include <Transport/RTPSocket.h>
#include <Transport/SocketSplitter.h>

#include <Transport/WSM/WSMSocket.h>

#include <Common/TimeMeter.h>

#include <Audio/AudioDecoder.h>
#include <Audio/AudioMixer.h>

#include <JitterBuffer/JB.h>

#include <Record/Recorder.h>

#include <Crypto/Decryptor.h>

#include <UI/DeviceNotifies.h>

#include <spdlog/spdlog.h>

namespace RendererSession
{
	class RendererAudioSession : public IRendererAudioSession
	{
	public:
		RendererAudioSession(Common::TimeMeter &timeMeter, Audio::AudioMixer &audioMixer);
		~RendererAudioSession();
		
        void SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback);

		/// derived from IRendererAudioSession
		virtual void SetName(std::string_view name);
		virtual std::string_view GetName();
		virtual void SetMetadata(std::string_view metadata);
		virtual std::string_view GetMetadata();
		virtual void SetMy(bool yes);
		virtual bool GetMy();
		virtual Transport::ISocket* GetDirectReceiver();
		virtual void SetVolume(int vol);
		virtual void SetMute(bool yes);
		virtual bool GetMute();
		virtual void SetDecoderType(Audio::CodecType dt);
		virtual void SetRTPParams(std::string_view recvFromAddr, uint16_t recvFromRTPPort);
		virtual void SetWSMParams(std::string_view addr, std::string_view acessToken, std::string_view wsDestAddr);
		virtual void SetClientId(int64_t clientId);
		virtual int64_t GetClientId() const;
		virtual uint32_t GetDeviceId() const;
		virtual uint32_t GetSSRC() const;
		virtual uint32_t GetAuthorSSRC() const;
		virtual std::string GetSecureKey() const;
		virtual void SetRecorder(Recorder::Recorder* recorder);
		virtual void Start(uint32_t receiverSSRC, uint32_t authorSSRC, uint32_t deviceId, std::string_view secureKey);
		virtual bool Started() const;
		virtual void Stop();

		virtual void Pause();
		virtual void Resume();
	private:
        Client::DeviceNotifyCallback deviceNotifyCallback;
		Audio::AudioMixer &audioMixer;
		Recorder::Recorder *recorder;
		Transport::ISocket* videoRecorder;
		Transport::SocketSplitter recordSplitter;
		Crypto::Decryptor decryptor;
		Audio::Decoder decoder;
        JB::JB jitterBuffer;
		Transport::RTPSocket rtpSocket;
		Transport::WSMSocket wsmSocket;
		Transport::ISocket* outSocket;
		std::thread pinger;

		bool runned, my, mute;

		int32_t volume;

		std::string name;
		std::string metadata;

		Audio::CodecType decoderType;

		uint32_t receiverSSRC, authorSSRC;
		int64_t clientId;
		uint32_t deviceId;
		std::string secureKey;

		uint16_t pingCnt;
		uint32_t lastPacketLoss;

		std::string wsAddr, accessToken, wsDestAddr;
		
		std::shared_ptr<spdlog::logger> sysLog, errLog;

		void EstablishConnection();
	};

	typedef std::shared_ptr<RendererAudioSession> RendererAudioSessionPtr_t;
}
