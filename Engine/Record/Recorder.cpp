/**
 * Recorder.cpp - Contains media recorder impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014, 2019, 2024
 */

#include <cstdint>
#include <cmath>

#include <ippcc.h>
#include <ippi.h>

#include <wui/config/config.hpp>

#include <Common/Common.h>

#include <Transport/RTP/RTPPacket.h>
#include <Transport/RTP/RTPPayloadType.h>

#include <Record/Recorder.h>

#include <Version.h>

namespace Recorder
{

Recorder::Recorder()
	: runned(false),
	mp3Mode(false),
	writerMutex(),
	writer(),
	muxerSegment(),
	vidTrack(0), audTrack(0),
	ts(0),
	videosRWLock(), videos(),
	fakeVideoChannel{ 0, 0, -1, Video::GetResolution(Video::ResolutionValues(1280, 720)), this },
	hasKeyFrame(false),
	currentVideoChannel(fakeVideoChannel),
	audioEncoder(), audioMixer(),
	jBufs(),
	fakeVideoSource(new uint8_t[static_cast<size_t>(1280 * 720 * 1.5)]),
	fakeVideoEncoder(),
	sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{
	audioEncoder.SetReceiver(this);
	fakeVideoEncoder.SetReceiver(this);
	fakeVideoEncoder.SetResolution(fakeVideoChannel.resolution);
	memset(fakeVideoSource.get(), 128, static_cast<size_t>(1280 * 720 * 1.5));
}

Recorder::~Recorder()
{
	Stop();
}

void Recorder::Start(std::string_view name, bool mp3Mode_)
{
	if (!runned)
	{
		std::lock_guard<std::recursive_mutex> lock(writerMutex);

		mp3Mode = mp3Mode_;

		if (mp3Mode)
		{
			mp3Writer.Start(name);

			audioMixer.Start();

			runned = true;

			return sysLog->info("Recorder start in mp3 only mode, writing file: {0}", name);
		}

		audioMixer.Start();

		ts = 0;
		hasKeyFrame = false;

		bool ok = false;

		if (writer == nullptr)
		{
			writer = std::unique_ptr<mkvmuxer::MkvWriter>(new mkvmuxer::MkvWriter());
			ok = writer->Open(name.data());
			if (!ok)
			{
				return errLog->error("Recorder start error opening file {0}", name);
			}
		}

		if (muxerSegment)
		{
			muxerSegment.reset(nullptr);
		}
		muxerSegment = std::unique_ptr<mkvmuxer::Segment>(new mkvmuxer::Segment());

		ok = muxerSegment->Init(writer.get());
		if (!ok)
		{
			return errLog->error("Recorder start error muxerSegment->Init", name);
		}

		muxerSegment->set_mode(mkvmuxer::Segment::kFile);
		muxerSegment->OutputCues(true);

		mkvmuxer::Cues* const cues = muxerSegment->GetCues();
		cues->set_output_block_number(true);

		// Set SegmentInfo element attributes
		auto *info = muxerSegment->GetSegmentInfo();
		info->set_timecode_scale(1000000);
		info->set_writing_app(SYSTEM_NAME " Client");

		// Creating video channel
		auto rv = Video::GetValues(currentVideoChannel.resolution);
		vidTrack = muxerSegment->AddVideoTrack(rv.width, rv.height, 0);
		if (!vidTrack)
		{
			return errLog->error("Recorder start error muxerSegment->AddVideoTrack :: (name: {0})", name);
		}

		mkvmuxer::VideoTrack* const video =
			static_cast<mkvmuxer::VideoTrack*>(
				muxerSegment->GetTrackByNumber(vidTrack));

		if (!video)
		{
			return errLog->error("Recorder start error muxerSegment->GetTrackByNumber(vidTrack: {0}, name: {1})", vidTrack, name);
		}
		
		video->set_frame_rate(25); /// 40 ms - 25 frames per second

		muxerSegment->CuesTrack(vidTrack);

		// Creating audio channel
		audTrack = muxerSegment->AddAudioTrack(48000, 1, 0);
		if (!audTrack)
		{
			return errLog->error("Recorder start error muxerSegment->AddAudioTrack", name);
		}

		mkvmuxer::AudioTrack* const audio =
			static_cast<mkvmuxer::AudioTrack*>(
				muxerSegment->GetTrackByNumber(audTrack));

		if (!audio)
		{
			return errLog->error("Recorder start error muxerSegment->GetTrackByNumber(audTrack)", name);
		}

		audio->set_codec_id(mkvmuxer::Tracks::kOpusCodecId);

		muxerSegment->CuesTrack(audTrack);

		// Start the audio encoder and mixer
		audioEncoder.Start(Audio::CodecType::Opus);
		audioMixer.Start();

		// Start the fake video encoder
		fakeVideoEncoder.Start(Video::CodecType::VP8, 0);

		runned = true;

		sysLog->info("Recorder started in normal mode, writing file: {0}", name);
	}
}

void Recorder::Stop()
{
	if (runned)
	{
		runned = false;

		if (mp3Mode)
		{
			mp3Writer.Stop();
			return sysLog->info("Recorder ended (mp3 mode)");
		}
		
		audioEncoder.Stop();

		std::lock_guard<std::recursive_mutex> lock(writerMutex);

		muxerSegment->set_duration(std::round(ts / 1000000));

		muxerSegment->Finalize();
		muxerSegment.reset(nullptr);

		writer.reset(nullptr);

		sysLog->info("Recorder ended (normal mode)");
	}
}

void Recorder::AddVideo(ssrc_t ssrc, int64_t clientId, int32_t priority, Video::Resolution resolution, Video::IPacketLossCallback *packetLossCallback)
{
	if (mp3Mode)
	{
		return;
	}

	mt::scoped_rw_lock lock(&videosRWLock, true);

	if (videos.find(ssrc) == videos.end())
	{
		auto newChannel = VideoChannel{ ssrc, clientId, priority, resolution, packetLossCallback };
		videos.insert(std::pair<uint32_t, VideoChannel>(ssrc, newChannel));

		sysLog->info("Recorder::AddVideo :: (ssrc: {0}, client_id: {1}, priority: {2}, resolution: {3})", ssrc, clientId, priority, (int)resolution);

		std::lock_guard<std::recursive_mutex> lock(writerMutex);
		if ((currentVideoChannel.clientId == clientId &&
			currentVideoChannel.priority < priority) ||
			currentVideoChannel.ssrc == 0)
		{
			currentVideoChannel = newChannel;
			hasKeyFrame = false;

			sysLog->trace("Recorder::AddVideo :: Current video channel :: (ssrc: {0}, client_id: {1}, priority: {2}, resolution: {3})", ssrc, clientId, priority, (int)resolution);
		}
	}
}

void Recorder::ChangeVideoResolution(ssrc_t ssrc, Video::Resolution resolution)
{
	if (mp3Mode)
	{
		return;
	}

    mt::scoped_rw_lock lock(&videosRWLock, true);
	
	auto it = videos.find(ssrc);
	if (it != videos.end())
	{
		it->second.resolution = resolution;

		std::lock_guard<std::recursive_mutex> lock(writerMutex);
		if (it->second.ssrc == currentVideoChannel.ssrc)
		{
			currentVideoChannel.resolution = resolution;
		}
	}
}

void Recorder::DeleteVideo(ssrc_t ssrc)
{
	if (mp3Mode)
	{
		return;
	}

	int64_t clientId = 0;

	{
        mt::scoped_rw_lock lock(&videosRWLock, true);

		auto it = videos.find(ssrc);
		if (it != videos.end())
		{
			std::lock_guard<std::recursive_mutex> lock(writerMutex);
			if (it->second.ssrc == currentVideoChannel.ssrc)
			{
				clientId = currentVideoChannel.clientId;
			}
			videos.erase(it);

			sysLog->info("Recorder::DeleteVideo :: (ssrc: {0}, clientId: {1})", ssrc, clientId);
		}
	}

	SpeakerChanged(clientId);
}

void Recorder::AddAudio(ssrc_t ssrc, int64_t clientId)
{
	//audioMixer.AddInput(ssrc, clientId);
}

void Recorder::DeleteAudio(ssrc_t ssrc)
{
	audioMixer.DeleteInput(ssrc);
}

bool IsKeyFrame(const uint8_t *data)
{
	struct parsed_header
	{
		char key_frame;
		int version;
		char show_frame;
		int first_part_size;
	};

	unsigned int tmp;
	const unsigned char* frame = data;
	parsed_header hdr;
	tmp = (frame[2] << 16) | (frame[1] << 8) | frame[0];
	hdr.key_frame = !(tmp & 0x1); /* inverse logic */
	hdr.version = (tmp >> 1) & 0x7;
	hdr.show_frame = (tmp >> 4) & 0x1;
	hdr.first_part_size = (tmp >> 5) & 0x7FFFF;

	return hdr.key_frame != 0;
}

void Recorder::Send(const Transport::IPacket &packet_, const Transport::Address *)
{
	if (!runned)
	{
		return;
	}

	const auto &packet = *static_cast<const Transport::RTPPacket*>(&packet_);
	if (packet.payloadSize == 0)
	{
		return;
	}

	switch (static_cast<Transport::RTPPayloadType>(packet.rtpHeader.pt))
	{
		case Transport::RTPPayloadType::ptPCM: // Receiving audio from clients
			//audioMixer->Send(packet_);
		break;
		case Transport::RTPPayloadType::ptOpus: // Mixed from mixer and encoded by local encoder
		{
			std::lock_guard<std::recursive_mutex> lock(writerMutex);

			if (currentVideoChannel.ssrc == fakeVideoChannel.ssrc)
			{
				GenerateFakeVideo();
			}

			bool ok = muxerSegment->AddFrame(packet.payload,
				packet.payloadSize,
				audTrack,
				ts,
				false);

			ts += 10000000;

			if (!ok)
			{
				return errLog->error("Recorder::Send audio error in muxerSegment->AddFrame({0})", audTrack);
			}
		}
		break;
		case Transport::RTPPayloadType::ptVP8: // Receive the video
		{
			if (mp3Mode)
			{
				return;
			}

			std::lock_guard<std::recursive_mutex> lock(writerMutex);

			if (packet.rtpHeader.ssrc != currentVideoChannel.ssrc) // Drop not current video
			{
				//sysLog->debug("Recorder::Send packet.rtpHeader.ssrc({0}) != currentVideoChannel->ssrc({1})", packet.rtpHeader.ssrc, currentVideoChannel.ssrc);
				return;
			}

			auto isKey = IsKeyFrame(packet.payload);
			if (!hasKeyFrame && !isKey)
			{
				currentVideoChannel.packetLossCallback->ForceKeyFrame(packet.rtpHeader.seq);

				return sysLog->debug("Recorder::Send request the key frame force", vidTrack);
			}

			hasKeyFrame = true;
			
			bool ok = muxerSegment->AddFrame(packet.payload,
				packet.payloadSize,
				vidTrack,
				ts,
				isKey);
			
			if (!ok)
			{
				return errLog->error("Recorder::Send error in video muxerSegment->AddFrame({0})", vidTrack);
			}
		}
		break;
		default:
		break;
	}
}

void Recorder::SpeakerChanged(int64_t clientId)
{
	if (mp3Mode)
	{
		return;
	}

	VideoChannel channel = { 0 };
	{
        mt::scoped_rw_lock lock(&videosRWLock, false);
		int32_t channelPriority = -1;
		for (auto &video : videos)
		{
			if (video.second.clientId == clientId && video.second.priority > channelPriority)
			{
				channel = video.second;
				channelPriority = video.second.priority;
			}
		}
	}

	if (channel.ssrc == 0)
	{
		channel = fakeVideoChannel;
		channel.clientId = clientId;

		sysLog->debug("Recorder::SpeakerChanged to fake channel :: (clientId: {0})", clientId);
	}

	std::lock_guard<std::recursive_mutex> lock(writerMutex);

	if (runned)
	{
		mkvmuxer::VideoTrack* const video =
			static_cast<mkvmuxer::VideoTrack*>(
				muxerSegment->GetTrackByNumber(vidTrack));

		if (!video)
		{
			return errLog->error("Recorder::SpeakerChanged error in muxerSegment->GetTrackByNumber({0})", vidTrack);
		}

		auto rv = Video::GetValues(channel.resolution);
		video->set_width(rv.width);
		video->set_height(rv.height);
	}

	currentVideoChannel = channel;
	hasKeyFrame = false;

	sysLog->debug("Recorder::SpeakerChanged to user_id: {0}", clientId);
}

void Recorder::ForceKeyFrame(uint32_t)
{
	fakeVideoEncoder.ForceKeyFrame(0);
}

void Recorder::GenerateFakeVideo()
{
	Transport::RTPPacket packet;
	packet.payload = fakeVideoSource.get();
	packet.payloadSize = static_cast<uint32_t>(1280 * 720 * 1.5);

	fakeVideoEncoder.Send(packet);
}

}
