/**
 * CameraDSG.cpp - Contains impl of camera capture direct show graph
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#include "CameraDSG.h"

#include <Device/Device.h>

#include <atltrace.h>
#include <initguid.h>

#include <boost/nowide/convert.hpp>

namespace Camera
{

DEFINE_GUID(CLSID_VideoCaptureSources,
	0x860BB310, 0x5D01, 0x11D0, 0xBD, 0x3B, 0x00, 0xA0, 0xC9, 0x11, 0xCE, 0x86);

extern const IID IID_ISampleGrabberCam = { 0x50e1b8a6, 0x90b4, 0x4866, { 0xa2, 0xb1, 0xf, 0x0, 0x17, 0x7d, 0xb5, 0x97 } };

static const CLSID CLSID_NullRendererCam = { 0xC1F400A4, 0x3F08, 0x11d3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };

std::string GetVendorName(std::string_view devName)
{
	std::string out(devName);

	auto pos = out.find_first_of(" ");
	if (pos != std::string::npos)
	{
		out.erase(pos, std::string::npos);
	}
	return out;
}

HRESULT BuildGraph(IFilterGraph2 *pGraph,
    std::string_view cameraName,
	Video::Resolution resolution,
	Video::ColorSpace colorSpace,
	void *inst,
	MANAGEDCALLBACKPROC outputCallback,
	StreamConfigPtr &streamConfig,
	KsPropertySetPtr &ksPropertySet,
	AMCameraControlPtr &cameraControl)
{
	HRESULT hr = S_OK;

	/// Creates block

	// Graph builder
	CComPtr<ICaptureGraphBuilder2> pBuilder;
	hr = pBuilder.CoCreateInstance(CLSID_CaptureGraphBuilder2); CHECK_HR(hr, "Can't create Capture Graph Builder");
	hr = pBuilder->SetFiltergraph(pGraph);
	CHECK_HR(hr, "Can't SetFiltergraph");

	// Add Null Renderer
	CComPtr<IBaseFilter> pNullRenderer;
	hr = pNullRenderer.CoCreateInstance(CLSID_NullRendererCam); CHECK_HR(hr, "Can't create Null Renderer");
	hr = pGraph->AddFilter(pNullRenderer, L"Null Renderer"); CHECK_HR(hr, "Can't add Null Renderer to graph");
	
	// Add SampleGrabber
	CComPtr<IBaseFilter> pSampleGrabber(new CSampleGrabber(NULL, &hr));
	CHECK_HR(hr, "Can't create the SampleGrabber");
	hr = pGraph->AddFilter(pSampleGrabber, L"SampleGrabber");
	CHECK_HR(hr, "Can't add SampleGrabber");

	Video::ResolutionValues rv = Video::GetValues(resolution);
	
	CComQIPtr<ISampleGrabber, &IID_ISampleGrabberCam> pSampleGrabber_isg(pSampleGrabber);
	hr = pSampleGrabber_isg->SetInstance(inst);
	CHECK_HR(hr, "Can't SetInstance on the SampleGrabber");
	hr = pSampleGrabber_isg->RegisterCallback(outputCallback);
	CHECK_HR(hr, "Can't RegisterCallback on the SampleGrabber");
	hr = pSampleGrabber_isg->SetVideo(rv.width, rv.height, colorSpace);
	CHECK_HR(hr, "Can't SetVideo on the SampleGrabber");

	// Creating MJPEG Decompressor if needed
	CComPtr<IBaseFilter> pMJPEGDecompressor;
	if (colorSpace == Video::ColorSpace::MJPG)
	{
		hr = pMJPEGDecompressor.CoCreateInstance(CLSID_MjpegDec);
		CHECK_HR(hr, "Can't create MJPEG Decompressor");
		hr = pGraph->AddFilter(pMJPEGDecompressor, L"MJPEGDecompressor");
		CHECK_HR(hr, "Can't add MJPEGDecopressor");
	}

	// Create source filter
	CComPtr<IBaseFilter> pSourceFilter = CreateFilterByName(cameraName, CLSID_VideoCaptureSources);
	hr = pGraph->AddFilter(pSourceFilter, L"Source Filter");
	CHECK_HR(hr, "Can't add Source Filter to camera graph");

	CComPtr<IPin> inCamPin = GetFirstInPin(pSourceFilter);
	if (inCamPin != NULL) /// Find the crossbar
	{
		std::string crossbarName;

		std::vector<Client::Device> crossbars;
		hr = GetCategoryFilters(AM_KSCATEGORY_CROSSBAR, crossbars);
		if (SUCCEEDED(hr))
		{
			auto camVendor = GetVendorName(cameraName);
			for (const auto &crossbar : crossbars)
			{
				if (GetVendorName(crossbar.name) == camVendor)
				{
					crossbarName = crossbar.name;
					break;
				}
			}

			if (!crossbarName.empty())
			{
				CComPtr<IBaseFilter> pCrossbarFilter = CreateFilterByName(crossbarName, AM_KSCATEGORY_CROSSBAR);
				hr = pGraph->AddFilter(pCrossbarFilter, L"Crossbar");
				CHECK_HR(hr, "Can't add crossbar to camera graph");

				CComPtr<IPin> outCrossbarPin = GetFirstOutPin(pCrossbarFilter);
				if (outCrossbarPin == NULL)
				{
					ATLTRACE(L"Crossbar %s out pin not found\n", crossbarName.c_str());
					return S_FALSE;
				}

				// Connect Crossbar and Source Filter
				hr = pGraph->ConnectDirect(outCrossbarPin, inCamPin, NULL);
				CHECK_HR(hr, "Can't connect Crossbar and Source Filter");

				CComPtr<IPin> outCrossbarSecondPin = GetSecondOutPin(pCrossbarFilter);
				CComPtr<IPin> inCamSecondPin = GetSecondInPin(pSourceFilter);
				if (outCrossbarSecondPin != NULL && inCamSecondPin != NULL)
				{
					pGraph->ConnectDirect(outCrossbarSecondPin, inCamSecondPin, NULL);
				}
			}
		}
	}

	CComPtr<IPin> outCamPin = GetFirstOutPin(pSourceFilter);
	if (outCamPin == NULL)
	{
		ATLTRACE(L"Camera %s out pit not found\n", cameraName);
		return S_FALSE;
	}

	AM_MEDIA_TYPE pCamera_pmt;
	ZeroMemory(&pCamera_pmt, sizeof(AM_MEDIA_TYPE));
	pCamera_pmt.majortype = MEDIATYPE_Video;
	switch (colorSpace)
	{
		case Video::ColorSpace::I420:
			pCamera_pmt.subtype = MEDIASUBTYPE_I420;
		break;
		case Video::ColorSpace::YUY2:
			pCamera_pmt.subtype = MEDIASUBTYPE_YUY2;
		break;
		case Video::ColorSpace::UYVU:
			pCamera_pmt.subtype = MEDIASUBTYPE_UYVY;
		break;
		case Video::ColorSpace::RGB24:
			pCamera_pmt.subtype = MEDIASUBTYPE_RGB24;
		break;
		case Video::ColorSpace::RGB32:
			pCamera_pmt.subtype = MEDIASUBTYPE_RGB32;
		break;
		case Video::ColorSpace::MJPG:
			pCamera_pmt.subtype = MEDIASUBTYPE_MJPG;
		break;
	}
	pCamera_pmt.formattype = FORMAT_VideoInfo;
	pCamera_pmt.bFixedSizeSamples = TRUE;
	pCamera_pmt.cbFormat = 88;
	pCamera_pmt.bTemporalCompression = FALSE;

	VIDEOINFOHEADER pCameraFormat;
	ZeroMemory(&pCameraFormat, sizeof(VIDEOINFOHEADER));
	pCameraFormat.AvgTimePerFrame = 333333;
	pCameraFormat.bmiHeader.biSize = 40;
	pCameraFormat.bmiHeader.biWidth = rv.width;
	pCameraFormat.bmiHeader.biHeight = rv.height;
	pCameraFormat.bmiHeader.biPlanes = 3;
	switch (colorSpace)
	{
		case Video::ColorSpace::I420:
			pCameraFormat.bmiHeader.biBitCount = 12;
			pCameraFormat.bmiHeader.biCompression = 808596553;
		break;
		case Video::ColorSpace::YUY2:
			pCameraFormat.bmiHeader.biBitCount = 16;
			pCameraFormat.bmiHeader.biCompression = 844715353;
		break;
		case Video::ColorSpace::UYVU:
			pCameraFormat.bmiHeader.biBitCount = 16;
			pCameraFormat.bmiHeader.biCompression = 1498831189;
		break;
		case Video::ColorSpace::RGB24:
			pCameraFormat.bmiHeader.biBitCount = 24;
			pCameraFormat.bmiHeader.biCompression = 0;
		break;
		case Video::ColorSpace::MJPG:
			pCameraFormat.bmiHeader.biBitCount = 24;
			pCameraFormat.bmiHeader.biCompression = 1196444237;
		break;
		case Video::ColorSpace::RGB32:
			pCameraFormat.bmiHeader.biBitCount = 32;
			pCameraFormat.bmiHeader.biCompression = 0;
		break;
	}
    pCamera_pmt.lSampleSize = static_cast<ULONG>(pCameraFormat.bmiHeader.biWidth * pCameraFormat.bmiHeader.biHeight * (static_cast<double>(pCameraFormat.bmiHeader.biBitCount) / 8));
	pCameraFormat.bmiHeader.biSizeImage = pCamera_pmt.lSampleSize;
	pCamera_pmt.pbFormat = (BYTE*)&pCameraFormat;

	StreamConfigPtr pSource_fmt(outCamPin);
	if (!pSource_fmt)
	{
		return S_FALSE;
	}

	streamConfig = pSource_fmt;
	hr = pSource_fmt->SetFormat(&pCamera_pmt);
	CHECK_HR(hr, "Can't set camera format");

	KsPropertySetPtr pKsPropertySet(pSourceFilter);
	ksPropertySet = pKsPropertySet;

	AMCameraControlPtr pCameraControl(pSourceFilter);
	cameraControl = pCameraControl;

	/// Connects block
	if (colorSpace != Video::ColorSpace::MJPG)
	{
		// Connect Source Filter and SampleGrabber
		hr = pGraph->ConnectDirect(outCamPin, GetPin(pSampleGrabber, L"Input"), NULL);
		CHECK_HR(hr, "Can't connect Source Filter and SampleGrabber");

		// Connect SampleGrabber and Null Renderer
		hr = pGraph->ConnectDirect(GetPin(pSampleGrabber, L"Output"), GetPin(pNullRenderer, L"In"), NULL);
		CHECK_HR(hr, "Can't connect SampleGrabber and Null Renderer");
	}
	else
	{
		// Connect Source Filter and MJPEG Decompressor
		hr = pGraph->ConnectDirect(outCamPin, GetPin(pMJPEGDecompressor, L"XForm In"), NULL);
		CHECK_HR(hr, "Can't connect Source Filter and MJPEG Decompressor");

		// Connect MJPEG Decompressor and SampleGrabber
		hr = pGraph->ConnectDirect(GetPin(pMJPEGDecompressor, L"XForm Out"), GetPin(pSampleGrabber, L"Input"), NULL);
		CHECK_HR(hr, "Can't connect MJPEG Decompressor and SampleGrabber");

		// Connect SampleGrabber and Null Renderer
		hr = pGraph->ConnectDirect(GetPin(pSampleGrabber, L"Output"), GetPin(pNullRenderer, L"In"), NULL);
		CHECK_HR(hr, "Can't connect SampleGrabber and Null Renderer");
	}
		
	return S_OK;
}

}
