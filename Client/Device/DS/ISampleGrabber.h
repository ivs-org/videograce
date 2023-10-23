/**
 * ISampleGrabber.h - Contains interface of the Sample Grabber DirectShow Filter
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#pragma once

#include <Video/ColorSpace.h>

typedef void (CALLBACK *MANAGEDCALLBACKPROC)(unsigned char* pdata, int len, void *inst);

// {50E1B8A6-90B4-4866-A2B1-0F00177DB597}
static const GUID IID_ISampleGrabber =
{ 0x50e1b8a6, 0x90b4, 0x4866, { 0xa2, 0xb1, 0xf, 0x0, 0x17, 0x7d, 0xb5, 0x97 } };

// {E1AA7D9A-BBD4-40EB-90CE-75BFE4EAACE9}
static const GUID CLSID_SampleGrabber =
{ 0xe1aa7d9a, 0xbbd4, 0x40eb, { 0x90, 0xce, 0x75, 0xbf, 0xe4, 0xea, 0xac, 0xe9 } };

DECLARE_INTERFACE_(ISampleGrabber, IUnknown)
{
	STDMETHOD (SetInstance)(void* inst) PURE;

	STDMETHOD (RegisterCallback)(MANAGEDCALLBACKPROC callback) PURE;

	STDMETHOD (SetVideo)(DWORD width, DWORD height, Video::ColorSpace colorSpace) PURE;

	STDMETHOD (SetAudio)(DWORD freq, DWORD nChannels) PURE;
};
