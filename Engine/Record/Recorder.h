/**
 * Recorder.h - Contains media recorder header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014, 2019
 */

#pragma once

#include <Record/IRecorder.h>

#include <map>
#include <mutex>
#include <memory>
#include <atomic>

#include <mt/rw_lock.h>

#include <Transport/ISocket.h>

#include <Audio/AudioMixer.h>
#include <Audio/AudioEncoder.h>

#include <Video/VideoEncoder.h>

#include <Video/IPacketLossCallback.h>

#include <Record/MP3Writer.h>

// libwebm muxer includes
#include <mkvmuxer/mkvmuxer.h>
#include <mkvmuxer/mkvwriter.h>
#include <mkvmuxer/mkvmuxerutil.h>

#include <spdlog/spdlog.h>

namespace Recorder
{
	class Recorder : public IRecorder, public Transport::ISocket, public Video::IPacketLossCallback
	{
	public:
		/// Derived from IRecorder
		virtual void Start(const char *fileName, bool mp3Mode) final;
		virtual void Stop() final;

		virtual void AddVideo(uint32_t ssrc, int64_t clientId, int32_t priority, Video::Resolution resolution, Video::IPacketLossCallback *packetLossCallback);
		virtual void ChangeVideoResolution(uint32_t ssrc, Video::Resolution resolution);
		virtual void DeleteVideo(uint32_t ssrc);

		virtual void AddAudio(uint32_t ssrc, int64_t clientId);
		virtual void DeleteAudio(uint32_t ssrc);

        virtual void SpeakerChanged(int64_t clientId);

		/// Derived from Transport::ISocket
		virtual void Send(const Transport::IPacket &packet, const Transport::Address *address = nullptr) final;
		
		/// Derived from Video::IPacketLossCallback
		virtual void ForceKeyFrame(uint32_t);

		Recorder();
		~Recorder();

	private:
		std::atomic<bool> runned;

		bool mp3Mode;

		std::recursive_mutex writerMutex;
				
		std::unique_ptr<mkvmuxer::MkvWriter> writer;
		std::unique_ptr<mkvmuxer::Segment> muxerSegment;

		uint64_t vidTrack, audTrack;
		uint64_t ts;

		struct VideoChannel
		{
			uint32_t ssrc;
			int64_t clientId;
			int32_t priority;
			Video::Resolution resolution;
			Video::IPacketLossCallback *packetLossCallback;
		};

		mt::rw_lock videosRWLock;
		std::map<uint32_t /* ssrc */, VideoChannel> videos;

		VideoChannel fakeVideoChannel;

		bool hasKeyFrame;
		VideoChannel *currentVideoChannel;

		MP3Writer mp3Writer;

		Audio::Encoder audioEncoder;
		Audio::AudioMixer audioMixer;

		std::unique_ptr<uint8_t[]> fakeVideoSource;
		Video::Encoder fakeVideoEncoder;

		std::shared_ptr<spdlog::logger> sysLog, errLog;

		void GenerateFakeVideo();
	};
}
