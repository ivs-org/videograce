/**
 * Player.h - Contains header of shell client's media player
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#pragma once

#include <cstdint>
#include <thread>
#include <deque>
#include <memory>

#include <mkvparser/mkvparser.h>
#include <mkvparser/mkvreader.h>

#include <Transport/RTPSocket.h>
#include <Transport/RTP/OwnedRTPPacket.h>
#include <Video/VideoEncoder.h>
#include <Video/VP8RTPSplitter.h>
#include <Video/Resolution.h>

#include <Common/TimeMeter.h>

#include <Crypto/Encryptor.h>

#include <spdlog/spdlog.h>

namespace Player
{

enum class PlayMode
{
	undefined,

	mkv_av,
	rgb24_show,
	mp3_play
};

class Player : public Transport::ISocket
{
	bool runned;
	bool cameraStarted, microphoneStarted;
	std::thread worker;

	std::string fileName;

    Common::TimeMeter timeMeter;

	mkvparser::MkvReader reader;

	Transport::RTPSocket cameraSocket, microphoneSocket;

	Video::Encoder videoEncoder;
	Video::VP8RTPSplitter videoSplitter;

    Crypto::Encryptor cameraEncryptor, microphoneEncryptor;

	PlayMode playMode;

    uint32_t cameraSSRC, cameraFrameIndex,
        microphoneSSRC, microphoneFrameIndex;
    
    uint64_t videoSendTime, audioSendTime;

	std::unique_ptr<uint8_t[]> outputBuffer, tmpBuffer;

	std::shared_ptr<spdlog::logger> sysLog, errLog;
public:
	Player();
	~Player();

    uint32_t cameraId, microphoneId;

    void SetFileName(const std::string &fileName);

	Video::Resolution GetResolution();

	void SetCameraRTPParams(const char* addr, uint16_t rtpPort);
	void SetMicrophoneRTPParams(const char* addr, uint16_t rtpPort);

	void Start();
	void Stop();

	void StartCamera(uint32_t ssrc, const std::string &secureKey);
	void StopCamera();

	void StartMicrophone(uint32_t ssrc, const std::string &secureKey);
	void StopMicrophone();

	bool IsCameraStarted();
	bool IsMicrophoneStarted();

private:
	Video::Resolution GetResolutionMKV();
	Video::Resolution GetResolutionRGB24();
	Video::Resolution GetResolutionMP3();

	void PlayMKV();
	void ShowRGB24();
	void PlayMP3();

    void SendAudio(const Transport::OwnedRTPPacket &packet);
    void SendVideo(const Transport::OwnedRTPPacket &packet);

	void ConvertFromRGB24(unsigned char* data, size_t *len);
	void ConvertFromRGB32(unsigned char* data, size_t *len);

	/// derived from Transport::ISocket (receive RTCP only)
	virtual void Send(const Transport::IPacket &packet, const Transport::Address *address = nullptr);
};

}