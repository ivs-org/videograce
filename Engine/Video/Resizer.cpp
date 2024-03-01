/**
 * Resizer.cpp - Resizing RGB32 frames impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2024
 */

#include <Video/Resizer.h>
#include <Transport/RTP/RTPPacket.h>

#include <memory>

namespace Video
{

Resizer::Resizer()
	: receiver(nullptr),
	runned(false),
	resolution(),
	width(0), height(0),
	mirror(false),
	dstSz{ 0, 0 }, prevSz{ 0, 0 },
	mirrorBuffer(), scaleBuffer(), scaleInitBuffer(), scaleSpecBuffer(), scaleWorkBuffer()
{
}

Resizer::~Resizer()
{
	Stop();
}

void Resizer::Start()
{
	if (!runned)
	{
		CreateBuffers();

		runned = true;
	}
}

void Resizer::Stop()
{
	runned = false;

	DestroyBuffers();
}

void Resizer::SetSize(int32_t width_, int32_t height_)
{
	width = width_;
	height = height_;
}

void Resizer::SetMirror(bool yes)
{
	mirror = yes;
}

void Resizer::SetReceiver(Transport::ISocket *receiver_)
{
	receiver = receiver_;
}

void Resizer::Send(const Transport::IPacket &packet_, const Transport::Address *)
{
	if (!runned)
	{
		return;
	}

	auto rv = Video::GetValues(resolution);

	if (rv.width == 0 || rv.height == 0 || width < 10 || height < 10)
	{
		return;
	}

	auto& packet = *static_cast<const Transport::RTPPacket*>(&packet_);

	if (packet.payloadSize == 0)
	{
		return;
	}

	const uint8_t* data = packet.payload;
	const IppiSize srcSz = { rv.width, rv.height };

	if (mirror)
	{
		ippiMirror_8u_C4R(data, rv.width * 4, mirrorBuffer.get(), rv.width * 4, srcSz, ippAxsVertical);
		data = mirrorBuffer.get();
	}

	auto scale = static_cast<double>(width) / rv.width;

	dstSz = { width, static_cast<int>((static_cast<double>(rv.height) * scale)) };

	if (dstSz.width > 3840 || dstSz.height > 2160)
	{
		return;
	}

	ResizeImage_C4R(data, srcSz, rv.width * 4, scaleBuffer.get(), dstSz, dstSz.width * 4);

	prevSz = dstSz;

	Transport::RTPPacket rtp = packet;
	rtp.payload = scaleBuffer.get();
	rtp.payloadSize = width * height * 4;
	receiver->Send(rtp);
}

void Resizer::CreateBuffers()
{
	auto rv = Video::GetValues(resolution);

	try
	{
		mirrorBuffer = std::unique_ptr<uint8_t[]>(new uint8_t[rv.width * rv.height * 4]);
		scaleBuffer = std::unique_ptr<uint8_t[]>(new uint8_t[3840 * 2160 * 4]);
		scaleInitBuffer = std::unique_ptr<uint8_t[]>(new uint8_t[rv.width * rv.height * 4]);
		scaleSpecBuffer = std::unique_ptr<uint8_t[]>(new uint8_t[rv.width * rv.height * 4]);
		scaleWorkBuffer = std::unique_ptr<uint8_t[]>(new uint8_t[3840 * 2160 * 4]);
	}
	catch (std::bad_alloc&)
	{
		// todo
	}

	prevSz = { 0, 0 };
}

void Resizer::DestroyBuffers()
{
	mirrorBuffer.reset(nullptr);
	scaleBuffer.reset(nullptr);
	scaleInitBuffer.reset(nullptr);
	scaleSpecBuffer.reset(nullptr);
	scaleWorkBuffer.reset(nullptr);
}

void Resizer::SetResolution(Video::Resolution resolution_)
{
	if (runned)
	{
		runned = false;

		DestroyBuffers();
		resolution = resolution_;
		CreateBuffers();

		runned = true;
	}
	else
	{
		resolution = resolution_;
	}
}

void Resizer::ResizeImage_C4R(const Ipp8u* pSrc, IppiSize srcSize, Ipp32s srcStep, Ipp8u* pDst, IppiSize dstSize, Ipp32s dstStep)
{
	if (prevSz.width != dstSize.width || prevSz.height != dstSize.height)
	{
		auto status = ippiResizeLanczosInit_8u(srcSize, dstSize, 3, (IppiResizeSpec_32f*)scaleSpecBuffer.get(), scaleInitBuffer.get());
		if (status != ippStsNoErr)
		{
			//errLog->critical("ippiResizeLanczosInit_8u error: {0:d}", status);
			return;
		}
	}

	ippiResizeLanczos_8u_C4R(pSrc, srcStep, pDst, dstStep, { 0, 0 }, dstSize, ippBorderRepl, 0, (IppiResizeSpec_32f*)scaleSpecBuffer.get(), scaleWorkBuffer.get());
}

}
