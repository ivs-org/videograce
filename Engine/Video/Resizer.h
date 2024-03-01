/**
 * Resizer.h - Resizing RGB32 frames
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2024
 */

#pragma once

#include <Transport/ISocket.h>
#include <Video/Resolution.h>

#include <cstddef>
#include <memory>

#include <ippcc.h>
#include <ippi.h>

namespace Video
{

class Resizer : public Transport::ISocket
{
public:
	Resizer();
	~Resizer();

	void Start();
	void Stop();

	void SetResolution(Video::Resolution resolution);
	void SetSize(int32_t width, int32_t height);

	void SetMirror(bool yes);

	void SetReceiver(Transport::ISocket *receiver);

	/// Derived from Transport::ISocket (input method)
	virtual void Send(const Transport::IPacket &packet, const Transport::Address *address = nullptr) final;

private:
	Transport::ISocket *receiver;

	bool runned;

	Video::Resolution resolution;
	int32_t width, height;

	bool mirror;

	IppiSize dstSz, prevSz;
	std::unique_ptr<uint8_t[]> mirrorBuffer,
		scaleBuffer, scaleInitBuffer, scaleSpecBuffer, scaleWorkBuffer;

	void ResizeImage_C4R(const Ipp8u* pSrc, IppiSize srcSize, Ipp32s srcStep, Ipp8u* pDst, IppiSize dstSize, Ipp32s dstStep);

	void CreateBuffers();
	void DestroyBuffers();
};

}
