/**
 * SampleGrabber.h - Contains header of the Sample Grabber DirectShow Filter
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#pragma once

#include <streams.h>
#include "ISampleGrabber.h"

class CSampleGrabber : public CTransInPlaceFilter, public ISampleGrabber
{
private:
	void* inst;
	MANAGEDCALLBACKPROC callback;
	bool isVideo;
	DWORD width, height;
	Video::ColorSpace colorSpace;
	DWORD freq, nChannels;
public:
	CSampleGrabber(IUnknown * pOuter, HRESULT * phr);
	~CSampleGrabber();
	static CUnknown *WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);
	
	// IUnknown
	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
	
	// CTransInPlaceFilter
	HRESULT CheckInputType(const CMediaType *pmt);
	HRESULT SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt);
	HRESULT Transform(IMediaSample *pMediaSample);
	HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProperties);
	HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
	HRESULT CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut)
	{
		return NOERROR;
	}
	
	// ISampleGrabber
	STDMETHODIMP SetInstance(void* inst);
	STDMETHODIMP RegisterCallback(MANAGEDCALLBACKPROC mdelegate);
	STDMETHODIMP SetVideo(DWORD width, DWORD height, Video::ColorSpace colorSpace);
	STDMETHODIMP SetAudio(DWORD freq, DWORD nChannels);
};
