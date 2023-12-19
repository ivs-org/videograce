/**
 * MicrophoneDSG.cpp - Contains impl of microphone capture direct show graph
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#include <Microphone/win/MicrophoneDSG.h>
#include <atltrace.h>
#include <initguid.h>
#include <math.h>

namespace MicrophoneNS
{

DEFINE_GUID(CLSID_AudioCaptureSources,
	0x0033D9A762, 0x90C8, 0x11D0, 0xBD, 0x43, 0x00, 0xA0, 0xC9, 0x11, 0xCE, 0x86);

extern const IID IID_ISampleGrabberMicro = { 0x50e1b8a6, 0x90b4, 0x4866, { 0xa2, 0xb1, 0xf, 0x0, 0x17, 0x7d, 0xb5, 0x97 } };

static const CLSID CLSID_NullRendererMicro = { 0xC1F400A4, 0x3F08, 0x11d3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };

HRESULT BuildGraph(IFilterGraph2 *pGraph,
	std::string_view deviceName,
	int gain,
	int freq,
	bool stereo,
	void *inst,
	MANAGEDCALLBACKPROC outputCallback,
	MicroMixerPtr &mixer)
{
	HRESULT hr = S_OK;

	/// Creates block

	// Add Null Renderer
	CComPtr<IBaseFilter> pNullRenderer;
	hr = pNullRenderer.CoCreateInstance(CLSID_NullRendererMicro); CHECK_HR(hr, "Can't create Null Renderer");
	hr = pGraph->AddFilter(pNullRenderer, L"Null Renderer"); CHECK_HR(hr, "Can't add Null Renderer to graph");

	// Graph builder
	CComPtr<ICaptureGraphBuilder2> pBuilder;
	hr = pBuilder.CoCreateInstance(CLSID_CaptureGraphBuilder2); CHECK_HR(hr, "Can't create Capture Graph Builder");
	hr = pBuilder->SetFiltergraph(pGraph);
	CHECK_HR(hr, "Can't SetFiltergraph");
	
	// Add SampleGrabber
	CComPtr<IBaseFilter> pSampleGrabber(new CSampleGrabber(NULL, &hr));
	CHECK_HR(hr, "Can't create the SampleGrabber");
	hr = pGraph->AddFilter(pSampleGrabber, L"SampleGrabber");
	CHECK_HR(hr, "Can't add SampleGrabber");

	CComQIPtr<ISampleGrabber, &IID_ISampleGrabberMicro> pSampleGrabber_isg(pSampleGrabber);
	hr = pSampleGrabber_isg->SetInstance(inst);
	CHECK_HR(hr, "Can't SetInstance on the SampleGrabber");
	hr = pSampleGrabber_isg->RegisterCallback(outputCallback);
	CHECK_HR(hr, "Can't RegisterCallback on the SampleGrabber");
	hr = pSampleGrabber_isg->SetAudio(freq, stereo ? 2 : 1);
	CHECK_HR(hr, "Can't SetAudio on the SampleGrabber");

	// Create source filter
	CComPtr<IBaseFilter> pSourceFilter = CreateFilterByName(deviceName, CLSID_AudioCaptureSources);
	hr = pGraph->AddFilter(pSourceFilter, L"Source Filter"); CHECK_HR(hr, "Can't add Source Filter to microphone graph");

	CComPtr<IPin> outMicroPin = GetFirstOutPin(pSourceFilter);
	if (outMicroPin == NULL)
	{
		ATLTRACE(L"Microphone %s out pit not found\n", deviceName);
		return S_FALSE;
	}

	AM_MEDIA_TYPE pMicrophone_pmt;
	ZeroMemory(&pMicrophone_pmt, sizeof(AM_MEDIA_TYPE));
	pMicrophone_pmt.majortype = MEDIATYPE_Audio;
	pMicrophone_pmt.subtype = MEDIASUBTYPE_PCM;
	pMicrophone_pmt.formattype = FORMAT_WaveFormatEx;
	pMicrophone_pmt.bFixedSizeSamples = TRUE;
	pMicrophone_pmt.cbFormat = 18;
	pMicrophone_pmt.bTemporalCompression = FALSE;
		
	WAVEFORMATEX microphoneFormat;
	ZeroMemory(&microphoneFormat, sizeof(WAVEFORMATEX));
	microphoneFormat.wFormatTag = 1;
	microphoneFormat.nChannels = stereo ? 2 : 1;
	microphoneFormat.nBlockAlign = microphoneFormat.nChannels * 2;
	microphoneFormat.wBitsPerSample = 16;
	microphoneFormat.nSamplesPerSec = freq;
	microphoneFormat.nAvgBytesPerSec = freq * microphoneFormat.nBlockAlign;
		
	pMicrophone_pmt.lSampleSize = microphoneFormat.nBlockAlign;
	
	pMicrophone_pmt.pbFormat = (BYTE*)&microphoneFormat;

	CComQIPtr<IAMStreamConfig, &IID_IAMStreamConfig> pSource_fmt(outMicroPin);
	hr = pSource_fmt->SetFormat(&pMicrophone_pmt);
	CHECK_HR(hr, "Can't set microphone format");

	CComPtr<IAMBufferNegotiation> pNeg;
	hr = outMicroPin->QueryInterface(IID_IAMBufferNegotiation, (void**)&pNeg);
	CHECK_HR(hr, "QueryInterface IID_IAMBufferNegotiation error");

	ALLOCATOR_PROPERTIES prop = { 0 };
	prop.cbBuffer = (freq / 1000) * (stereo ? 4 : 2) * 20; // 20 ms frame
	prop.cBuffers = 6;                                     // 6 frames
	prop.cbAlign = microphoneFormat.nBlockAlign;

	hr = pNeg->SuggestAllocatorProperties(&prop);
	CHECK_HR(hr, "SuggestAllocatorProperties error");
	
	CComQIPtr<IAMAudioInputMixer, &IID_IAMAudioInputMixer> pAMAudioInputMixer_isg(pSourceFilter);
	pAMAudioInputMixer_isg->put_MixLevel(gain != 0 ? (exp((double)gain / 100) / 2.718281828) : 0.0);

	mixer = pAMAudioInputMixer_isg;

	/// Connects block

	// Connect Source Filter and SampleGrabber
	hr = pGraph->ConnectDirect(outMicroPin, GetPin(pSampleGrabber, L"Input"), NULL);
	CHECK_HR(hr, "Can't connect Source Filter and SampleGrabber");

	// Connect SampleGrabber and Null Renderer
	hr = pGraph->ConnectDirect(GetPin(pSampleGrabber, L"Output"), GetPin(pNullRenderer, L"In"), NULL);
	CHECK_HR(hr, "Can't connect SampleGrabber and Null Renderer");

	return S_OK;
}

}
