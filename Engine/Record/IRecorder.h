/**
 * IRecorder.h - Contains recorder's interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <Video/Resolution.h>

namespace Video
{
class IPacketLossCallback;
}

namespace Recorder
{
	class IRecorder
	{
	public:
		virtual void Start(const char *fileName, bool mp3Mode) = 0;
		virtual void Stop() = 0;

		virtual void AddVideo(uint32_t ssrc, int64_t clientId, int32_t priority, Video::Resolution resolution, Video::IPacketLossCallback *packetLossCallback) = 0;
		virtual void ChangeVideoResolution(uint32_t ssrc, Video::Resolution resolution) = 0;
		virtual void DeleteVideo(uint32_t ssrc) = 0;

		virtual void AddAudio(uint32_t ssrc, int64_t clientId) = 0;
		virtual void DeleteAudio(uint32_t ssrc) = 0;

        virtual void SpeakerChanged(int64_t clientId) = 0;

	protected:
		~IRecorder() {}
	};
}
