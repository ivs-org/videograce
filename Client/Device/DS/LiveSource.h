/**
 * LiveSource.h - Contains header of the Input Source DirectShow Filter
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#pragma once

#include <memory>

#include <streams.h>
#include <Video/ColorSpace.h>
#include "IAddFrame.h"

class CLiveSourceStream;

class CLiveSource : public CBaseFilter, public IAMFilterMiscFlags
{
public:
	DECLARE_IUNKNOWN;

	CLiveSource(LPUNKNOWN pUnk, HRESULT* phr);
	virtual ~CLiveSource();

	static CUnknown* WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	int GetPinCount();
	CBasePin* GetPin(int n);

	virtual ULONG STDMETHODCALLTYPE GetMiscFlags( void)
	{
		return AM_FILTER_MISC_FLAGS_IS_SOURCE;
	}

private:
	std::unique_ptr<CLiveSourceStream> m_pOutputPin;
	CCritSec m_critSec;
};

class CLiveSourceStream : public CBaseOutputPin, public ILiveSource, public IAMPushSource
{
public:
	DECLARE_IUNKNOWN;

	CLiveSourceStream(CBaseFilter *pFilter, CCritSec *pLock, HRESULT *phr);
	virtual ~CLiveSourceStream();

	// CBaseOutputPin overrides
	virtual HRESULT GetMediaType(int iPosition, CMediaType* pmt);
	virtual HRESULT CheckMediaType(const CMediaType *pmt);
	virtual HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *ppropInputRequest);

	// ILiveSource members
	virtual STDMETHODIMP AddFrame(BYTE* pBuffer, DWORD size);
	virtual STDMETHODIMP SetVideoResolution(DWORD width, DWORD height);
		
	// IAMPushSource members
	virtual STDMETHODIMP GetPushSourceFlags(ULONG *pFlags);
	virtual STDMETHODIMP SetPushSourceFlags(ULONG Flags);
	virtual STDMETHODIMP SetStreamOffset(REFERENCE_TIME rtOffset);
	virtual STDMETHODIMP GetStreamOffset(REFERENCE_TIME *prtOffset);
	virtual STDMETHODIMP GetMaxStreamOffset(REFERENCE_TIME *prtMaxOffset);
	virtual STDMETHODIMP SetMaxStreamOffset(REFERENCE_TIME rtMaxOffset);
	virtual STDMETHODIMP GetLatency(REFERENCE_TIME *prtLatency);

	virtual STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

private:
	HRESULT GetMediaSample(IMediaSample** ppSample);

	void AddSizeToNeeded(const BYTE *input, BYTE *output);

private:
	DWORD curFrameSize;
	int frameRate;
	REFERENCE_TIME rtFrameRate; 
	REFERENCE_TIME lastFrame;
	DWORD videoWidth, videoHeight;

	DWORD neededWidth;
};
