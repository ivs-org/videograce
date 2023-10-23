/**
 * SampleGrabber.cpp - Contains impl of the Sample Grabber DirectShow Filter
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#include <ddraw.h>
#include "SampleGrabber.h"
#include <initguid.h>

#define FILTERNAME L"SampleGrabberFilter"

CUnknown *WINAPI CSampleGrabber::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
	HRESULT hr;
	if (!phr)
		phr = &hr;
	CSampleGrabber *pNewObject = new CSampleGrabber(punk, phr);
	if (pNewObject == NULL)
		*phr = E_OUTOFMEMORY;
	return pNewObject;
}

CSampleGrabber::CSampleGrabber(IUnknown * pOuter, HRESULT * phr) 
	: CTransInPlaceFilter( FILTERNAME, (IUnknown*) pOuter, CLSID_SampleGrabber, phr),
	inst(NULL),
	callback(NULL),
	isVideo(true),
	width(640), height(480),
	freq(48000), nChannels(2)
{ 
}

CSampleGrabber::~CSampleGrabber()
{
}

// IUnknown
HRESULT CSampleGrabber::NonDelegatingQueryInterface(const IID &riid, void **ppv)
{
	if ( riid == IID_ISampleGrabber )
	{
		return GetInterface(static_cast<ISampleGrabber*>(this), ppv);
	}
	else
		return CTransInPlaceFilter::NonDelegatingQueryInterface(riid, ppv);
}

// CTransInPlaceFilter
HRESULT CSampleGrabber::CheckInputType(const CMediaType *pmt)
{
	if (isVideo)
	{
		if (IsEqualGUID(*pmt->Type(), MEDIATYPE_Video))
		{
			WORD bitCount = 12;
			switch (colorSpace)
			{
				case Video::ColorSpace::I420:
					bitCount = 12;
				break;
				case Video::ColorSpace::YUY2: case Video::ColorSpace::UYVU:
					bitCount = 16;
				break;
				case Video::ColorSpace::RGB24: case Video::ColorSpace::MJPG:
					bitCount = 24;
				break;
				case Video::ColorSpace::RGB32:
					bitCount = 32;
				break;
			}

			VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)pmt->Format();
			if (vih->bmiHeader.biBitCount == bitCount &&
				vih->bmiHeader.biWidth == width &&
				vih->bmiHeader.biHeight == height)
			{
				return S_OK;
			}
		}
	}
	else
	{
		if (IsEqualGUID(*pmt->Type(), MEDIATYPE_Audio))
		{
			WAVEFORMATEX* wfx = (WAVEFORMATEX*)pmt->Format();
			if (wfx->wBitsPerSample == 16 &&
				wfx->nSamplesPerSec == freq &&
				wfx->nChannels == nChannels)
			{
				return S_OK;
			}
		}
	}
	return VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CSampleGrabber::SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt)
{
	return S_OK;
}

HRESULT CSampleGrabber::Transform(IMediaSample *pMediaSample)
{
	long size = 0;
	BYTE *pData;

	if (!pMediaSample)
		return E_FAIL;

	// Get pointer to the video buffer data
	if (FAILED(pMediaSample->GetPointer(&pData))) 
		return E_FAIL;
	size = pMediaSample->GetSize();

	// invoke managed delegate
	if (callback)
		callback(pData, size, inst);

	return S_OK;
}

HRESULT CSampleGrabber::DecideBufferSize(IMemAllocator *pAlloc,ALLOCATOR_PROPERTIES *pProperties)
{
	if (m_pInput->IsConnected() == FALSE)
	{
		return E_UNEXPECTED;
	}

	ASSERT(pAlloc);
	ASSERT(pProperties);
	HRESULT hr = NOERROR;

	pProperties->cBuffers = 1;
	pProperties->cbBuffer = m_pInput->CurrentMediaType().GetSampleSize();
	//ASSERT(pProperties->cbBuffer);

	// Ask the allocator to reserve us some sample memory, NOTE the function
	// can succeed (that is return NOERROR) but still not have allocated the
	// memory that we requested, so we must check we got whatever we wanted
	ALLOCATOR_PROPERTIES actual;
	hr = pAlloc->SetProperties(pProperties, &actual);
	if (FAILED(hr))
	{
		return hr;
	}

	ASSERT(actual.cBuffers >= 1);

	if (pProperties->cBuffers > actual.cBuffers ||
		pProperties->cbBuffer > actual.cbBuffer)
	{
		return E_FAIL;
	}
	return NOERROR;
}

HRESULT CSampleGrabber::GetMediaType(int iPosition, CMediaType *pMediaType)
{
	// Is the input pin connected
	if (m_pInput->IsConnected() == FALSE)
	{
		return E_UNEXPECTED;
	}

	// This should never happen
	if (iPosition < 0)
	{
		return E_INVALIDARG;
	}

	// Do we have more items to offer
	if (iPosition > 0)
	{
		return VFW_S_NO_MORE_ITEMS;
	}

	*pMediaType = m_pInput->CurrentMediaType();
	return NOERROR;
}

// ISampleGrabber
STDMETHODIMP CSampleGrabber::SetInstance(void* inst_)
{
	inst = inst_;
	return S_OK;
}

STDMETHODIMP CSampleGrabber::RegisterCallback(MANAGEDCALLBACKPROC mdelegate) 
{
	// Set pointer to managed delegate
	callback = mdelegate;
	return S_OK;
}

STDMETHODIMP CSampleGrabber::SetVideo(DWORD width_, DWORD height_, Video::ColorSpace colorSpace_)
{
	isVideo = true;
	width = width_;
	height = height_;
	colorSpace = colorSpace_;

	return S_OK;
}

STDMETHODIMP CSampleGrabber::SetAudio(DWORD freq_, DWORD nChannels_)
{
	isVideo = false;
	freq = freq_;
	nChannels = nChannels_;

	return S_OK;
}
