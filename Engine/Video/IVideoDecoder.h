/**
 * IVideoDecoder.h - Contains video decoder interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <Video/Resolution.h>
#include <Video/ColorSpace.h>
#include <Video/CodecType.h>
#include <Video/IPacketLossCallback.h>

namespace Transport
{
	class ISocket;
}

namespace Video
{
	class IDecoder
	{
	public:
		virtual void SetReceiver(Transport::ISocket *receiver) = 0;
		virtual void SetCallback(IPacketLossCallback *callback) = 0;
		
		virtual bool SetResolution(Resolution resolution) = 0;

		virtual void Start(CodecType type, Video::ColorSpace outputType) = 0;
		virtual void Stop() = 0;
		virtual bool IsStarted() = 0;
				
		virtual ~IDecoder() {}
	};
}
