/**
 * IRecorder.h - Contains recorder's interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014-2024
 */

#pragma once

#include <Common/Types.h>
#include <Video/Resolution.h>
#include <string_view>

namespace Video
{
class IPacketLossCallback;
}

namespace Recorder
{
	class IRecorder
	{
	public:
		virtual void Start(std::string_view fileName, bool mp3Mode) = 0;
		virtual void Stop() = 0;

		virtual void AddVideo(ssrc_t ssrc, int64_t clientId, int32_t priority, Video::Resolution resolution, Video::IPacketLossCallback *packetLossCallback) = 0;
		virtual void ChangeVideoResolution(ssrc_t ssrc, Video::Resolution resolution) = 0;
		virtual void DeleteVideo(ssrc_t ssrc) = 0;

		virtual void AddAudio(ssrc_t ssrc, int64_t clientId) = 0;
		virtual void SetSampleFreq(int32_t sampleFreq) = 0;
		virtual void DeleteAudio(ssrc_t ssrc) = 0;

        virtual void SpeakerChanged(int64_t clientId) = 0;

	protected:
		~IRecorder() {}
	};
}
