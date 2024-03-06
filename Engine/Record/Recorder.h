/**
 * Recorder.h - Contains media recorder header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014, 2019, 2024
 */

#pragma once

#include <Record/IRecorder.h>

#include <map>
#include <mutex>
#include <memory>
#include <thread>
#include <atomic>

#include <mt/rw_lock.h>

#include <Transport/ISocket.h>

#include <Audio/AudioMixer.h>
#include <Audio/AudioEncoder.h>

#include <Video/VideoEncoder.h>

#include <Video/IPacketLossCallback.h>

#include <JitterBuffer/JB.h>

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
		virtual void Start(std::string_view fileName, bool mp3Mode) final;
		virtual void Stop() final;

		virtual void AddVideo(ssrc_t ssrc, int64_t clientId, int32_t priority, Video::Resolution resolution, Video::IPacketLossCallback *packetLossCallback);
		virtual void ChangeVideoResolution(ssrc_t ssrc, Video::Resolution resolution);
		virtual void DeleteVideo(ssrc_t ssrc);

		virtual void AddAudio(ssrc_t ssrc, int64_t clientId);
		virtual void DeleteAudio(ssrc_t ssrc);

        virtual void SpeakerChanged(int64_t clientId);

		/// Derived from Transport::ISocket
		virtual void Send(const Transport::IPacket &packet, const Transport::Address *address = nullptr) final;
		
		/// Derived from Video::IPacketLossCallback
		virtual void ForceKeyFrame(uint32_t);

		Recorder();
		~Recorder();

	private:
		static constexpr int32_t FRAME_DURATION = 40000;

		std::atomic_bool runned;

		bool mp3Mode;

		Common::TimeMeter timeMeter;

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
		std::map<ssrc_t, VideoChannel> videos;

		VideoChannel fakeVideoChannel;

		bool hasKeyFrame;
		VideoChannel currentVideoChannel;

		MP3Writer mp3Writer;

		Audio::Encoder audioEncoder;
		Audio::AudioMixer audioMixer;

		mt::rw_lock audiosRWLock;
		std::map<ssrc_t, std::shared_ptr<JB::JB>> jBufs;

		std::unique_ptr<uint8_t[]> fakeVideoSource;
		std::unique_ptr<uint8_t[]> buffer0, buffer1, buffer2;
		Video::Encoder fakeVideoEncoder;

		std::thread soundWriter;

		std::shared_ptr<spdlog::logger> sysLog, errLog;

		void GenerateFakeVideo();

		void WriteSound();
	};
}
