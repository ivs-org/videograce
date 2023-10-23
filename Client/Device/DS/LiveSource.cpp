/**
 * LiveSource.cpp - Contains impl of the Input Source DirectShow Filter
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#include "LiveSource.h"

#include <tchar.h>
#include <Version.h>

/// Defines
#define LIVE_FILTER_NAME _T(SYSTEM_NAME) L" Frame Input Source"
#define LIVE_OUTPIN_NAME L"Out"

/// CLiveSource
CLiveSource::CLiveSource(LPUNKNOWN pUnk, HRESULT* phr)
	: CBaseFilter(LIVE_FILTER_NAME, pUnk, &m_critSec, CLSID_CLiveSource),
	m_pOutputPin(),
	m_critSec()
{
	m_pOutputPin = std::unique_ptr<CLiveSourceStream>(new CLiveSourceStream(this, &m_critSec, phr));

	if (phr)
	{
		if (m_pOutputPin.get() == nullptr)
			*phr = E_OUTOFMEMORY;
		else
			*phr = S_OK;
	}  
}

CLiveSource::~CLiveSource()
{
	m_pOutputPin.reset(nullptr);
}

int CLiveSource::GetPinCount()
{
	CAutoLock cAutoLock(&m_critSec);

	return 1;
}

CBasePin* CLiveSource::GetPin(int n)
{
	CAutoLock cAutoLock(&m_critSec);

	return m_pOutputPin.get();
}

CUnknown* WINAPI CLiveSource::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
	CUnknown* pNewFilter = new CLiveSource(pUnk, phr);

	if (phr)
	{
		if (pNewFilter == NULL) 
			*phr = E_OUTOFMEMORY;
		else
			*phr = S_OK;
	}

	return pNewFilter;
}

STDMETHODIMP CLiveSource::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	CheckPointer(ppv, E_POINTER);

	if(riid == IID_ILiveSource) 
	{
		return GetInterface((ILiveSource*) m_pOutputPin.get(), ppv);
	} 
	else if(riid == IID_IAMPushSource)
	{
		return GetInterface((IAMPushSource*) m_pOutputPin.get(), ppv);
	}
	else if(riid == IID_IAMFilterMiscFlags)
	{
		return GetInterface((IAMFilterMiscFlags*) this, ppv);
	}
	else 
	{
		return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
	}
}


/// CLiveSourceStream
CLiveSourceStream::CLiveSourceStream(CBaseFilter *pFilter, CCritSec *pLock, HRESULT *phr)
	: CBaseOutputPin(LIVE_FILTER_NAME, pFilter, pLock, phr, LIVE_OUTPIN_NAME),
	curFrameSize(0),
	frameRate(0),
	rtFrameRate(0),
	lastFrame(0),
	videoWidth(640),
	videoHeight(480),
	neededWidth(videoWidth)
{
}

CLiveSourceStream::~CLiveSourceStream()
{
}

HRESULT CLiveSourceStream::CheckMediaType(const CMediaType* pmt)
{
	CheckPointer(pmt, E_POINTER);

	if (IsEqualGUID(*pmt->Type(), MEDIATYPE_Video))
	{
		VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)pmt->Format();
		if (vih->bmiHeader.biBitCount == 32)
		{
			curFrameSize = videoWidth * videoHeight * 4;

			if (vih->bmiHeader.biWidth != videoWidth)
			{
				neededWidth = vih->bmiHeader.biWidth;
				curFrameSize = neededWidth * videoHeight * 4;
			}
			return S_OK;
		}
	}
	
	return VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CLiveSourceStream::GetMediaType(int iPosition, CMediaType* pmt)
{
	CAutoLock cAutoLock(m_pLock);

	if (iPosition < 0)
	{
		return E_INVALIDARG;
	}

	if (iPosition >= 1)
	{
		return VFW_S_NO_MORE_ITEMS;
	}

	VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)pmt->AllocFormatBuffer(sizeof(VIDEOINFOHEADER));
	if(!vih)
	{
		return E_OUTOFMEMORY;
	}

	vih->AvgTimePerFrame = 333333;
	vih->dwBitErrorRate = 0;
	
	BITMAPINFOHEADER& bmih = vih->bmiHeader;
	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = videoWidth;
	bmih.biHeight = videoHeight;
	bmih.biCompression = BI_RGB;
	bmih.biBitCount = 32;
	bmih.biSizeImage = 0;
	bmih.biPlanes = 1;
	bmih.biClrImportant = 0;
	bmih.biClrUsed = 0;
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;
	vih->dwBitRate = 0;

	RECT &rcSource = vih->rcSource;
	rcSource.left = rcSource.right = rcSource.top = rcSource.bottom = 0;
	RECT &rcTarget = vih->rcTarget;
	rcTarget.left = rcTarget.right = rcTarget.top = rcTarget.bottom = 0;

	pmt->majortype = MEDIATYPE_Video;
	pmt->subtype = MEDIASUBTYPE_RGB32;
	pmt->formattype = FORMAT_VideoInfo;
	pmt->pbFormat = (BYTE*)vih;
	pmt->cbFormat = sizeof(VIDEOINFOHEADER);

	pmt->bFixedSizeSamples = TRUE;
	pmt->bTemporalCompression = FALSE;
	pmt->lSampleSize = bmih.biSizeImage;
	
	pmt->pUnk = 0;
	
	return S_OK;
}

HRESULT CLiveSourceStream::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *ppropInputRequest)
{
	CheckPointer(pAlloc, E_POINTER);
	CheckPointer(ppropInputRequest, E_POINTER);

	CAutoLock cAutoLock(m_pLock);
	HRESULT hr = NOERROR;

	ppropInputRequest->cBuffers = 1;
	ppropInputRequest->cbBuffer = curFrameSize * 10;

	if (ppropInputRequest->cbBuffer == 0)
	{
		return E_FAIL;
	}

	ALLOCATOR_PROPERTIES actual;
	hr = pAlloc->SetProperties(ppropInputRequest, &actual);
	if (FAILED(hr))
	{
		return hr;
	}

	if (actual.cbBuffer < ppropInputRequest->cbBuffer)
	{
		return E_FAIL;
	}

	return S_OK;
}

STDMETHODIMP CLiveSourceStream::AddFrame(BYTE* pBuffer, DWORD size)
{
	if (size == 0)
	{
		return S_OK;
	}

	CAutoLock cAutoLock(m_pLock);
		
	IMediaSample* pSample = NULL;
	BYTE* pData = NULL;

	HRESULT hr = GetMediaSample(&pSample);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = pSample->SetActualDataLength(curFrameSize);
	if (FAILED(hr))
	{
		pSample->Release();
		return hr;
	}

	hr = pSample->GetPointer(&pData);
	if (FAILED(hr))
	{
		pSample->Release();
		return hr;
	}

	if (neededWidth == videoWidth)
	{
		memcpy(pData, pBuffer, size);
	}
	else
	{
		AddSizeToNeeded(pBuffer, pData);
	}

	hr = pSample->SetSyncPoint(TRUE);
	if (FAILED(hr))
	{
		pSample->Release();
		return hr;
	}
	
	hr = this->Deliver(pSample);
	pSample->Release();

	return hr;
}

STDMETHODIMP CLiveSourceStream::SetVideoResolution(DWORD width, DWORD height)
{
	neededWidth = videoWidth = width;
	videoHeight = height;

	curFrameSize = width * height * 4;

	return S_OK;
}

HRESULT CLiveSourceStream::GetMediaSample(IMediaSample** ppSample)
{
	REFERENCE_TIME rtStart = lastFrame;
	lastFrame += rtFrameRate;

	return this->GetDeliveryBuffer(ppSample, &rtStart, &lastFrame, 0);
}

STDMETHODIMP CLiveSourceStream::GetMaxStreamOffset(REFERENCE_TIME *prtMaxOffset)
{
	*prtMaxOffset = 0;
	return S_OK;
} 

STDMETHODIMP CLiveSourceStream::GetPushSourceFlags(ULONG *pFlags)
{ 
	*pFlags = 0;
	return S_OK;
}

STDMETHODIMP CLiveSourceStream::GetStreamOffset(REFERENCE_TIME *prtOffset)
{
	*prtOffset = 0;
	return S_OK;
} 

STDMETHODIMP CLiveSourceStream::SetMaxStreamOffset(REFERENCE_TIME rtMaxOffset)
{
	return E_NOTIMPL;
}

STDMETHODIMP CLiveSourceStream::SetPushSourceFlags(ULONG Flags)
{
	return E_NOTIMPL;
} 

STDMETHODIMP CLiveSourceStream::SetStreamOffset(REFERENCE_TIME rtOffset)
{
	return E_NOTIMPL;
} 

STDMETHODIMP CLiveSourceStream::GetLatency(REFERENCE_TIME *prtLatency)
{ 
	*prtLatency = rtFrameRate;
	return S_OK;
}

STDMETHODIMP CLiveSourceStream::Notify(IBaseFilter * pSender, Quality q)
{
	if (q.Proportion <= 0)
	{
		rtFrameRate = 1000;        
	}
	else
	{
		rtFrameRate = rtFrameRate * 1000 / q.Proportion;
		if (rtFrameRate > 1000)
		{
			rtFrameRate = 1000;    
		}
		else if (rtFrameRate < 30)
		{
			rtFrameRate = 30;      
		}
	}

	if (q.Late > 0)
	{
		rtFrameRate += q.Late;
	}

	return S_OK;
}

void CLiveSourceStream::AddSizeToNeeded(const BYTE *input, BYTE *output)
{
	memset(output, 0, neededWidth * videoHeight * 4);
	
	const int neededLine = neededWidth * 4;
	const int readedLine = videoWidth * 4;

	for (int i = 0; i != videoHeight; ++i)
	{
		memcpy(output, input, readedLine);
		output += neededLine;
		input += readedLine;
	}
}

/// Dummy objects
CFactoryTemplate g_Templates[] = { L"", NULL, NULL, NULL, NULL };
int g_cTemplates = 0;
