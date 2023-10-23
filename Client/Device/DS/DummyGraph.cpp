/**
 * DummyGraph.cpp - Contains dummy (doing nothing) direct show graph impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2019
 */

#include "DummyGraph.h"

#include "LiveSource.h"

#include <initguid.h>

DummyGraph::DummyGraph()
	: runned(false),
	thread(),
	mediaControl()
{
}

DummyGraph::~DummyGraph()
{
	Stop();
}

void DummyGraph::Start()
{
	if (!runned)
	{
		runned = true;
		thread = std::thread(&DummyGraph::Run, this);
	}
}

void DummyGraph::Stop()
{
	if (mediaControl)
	{
		mediaControl->Stop();
	}

	runned = false;
	if (thread.joinable()) thread.join();
}

extern const IID IID_ILiveSourceDummy = { 0xd312315a, 0xc36f, 0x4b09, { 0xba, 0xf4, 0xb3, 0xce, 0x92, 0x21, 0x85, 0xc2 } };

static const CLSID CLSID_NullRendererDummy = { 0xC1F400A4, 0x3F08, 0x11d3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };

HRESULT BuildGraph(IFilterGraph2 *pGraph)
{
	HRESULT hr = S_OK;

	/// Creates block

	// Graph builder
	CComPtr<ICaptureGraphBuilder2> pBuilder;
	hr = pBuilder.CoCreateInstance(CLSID_CaptureGraphBuilder2); CHECK_HR(hr, "Can't create Dummy Graph Builder");
	hr = pBuilder->SetFiltergraph(pGraph);
	CHECK_HR(hr, "Can't SetFiltergraph");

	// Create the Frame Input SourceFilter
	CComPtr<IBaseFilter> pInputSource(new CLiveSource(NULL, &hr));
	CHECK_HR(hr, "Can't create Frame Input Source Filter");
	hr = pGraph->AddFilter(pInputSource, L"Input Source");
	CHECK_HR(hr, "Can't add the Input Source to graph");

	CComQIPtr<ILiveSource, &IID_ILiveSourceDummy> pLiveSource(pInputSource);
	pLiveSource->SetVideoResolution(640, 480);
	
	// Add Null Renderer
	CComPtr<IBaseFilter> pNullRenderer;
	hr = pNullRenderer.CoCreateInstance(CLSID_NullRendererDummy); CHECK_HR(hr, "Can't create Null Renderer");
	hr = pGraph->AddFilter(pNullRenderer, L"Null Renderer"); CHECK_HR(hr, "Can't add Null Renderer to graph");

	/// Connects block
	hr = pGraph->ConnectDirect(GetPin(pInputSource, L"Out"), GetPin(pNullRenderer, L"In"), NULL);
	CHECK_HR(hr, "Can't connect Input Source and Null Renderer");

	return S_OK;
}

HRESULT DummyGraph::Run()
{
	CoInitialize(NULL);
	CComPtr<IFilterGraph2> graph;
	graph.CoCreateInstance(CLSID_FilterGraph);

	HRESULT hr = BuildGraph(graph);
	if (SUCCEEDED(hr))
	{
		mediaControl = CComQIPtr<IMediaControl, &IID_IMediaControl>(graph);
		hr = mediaControl->Run();
		CHECK_HR(hr, "Can't run the Dummy graph");

		CComQIPtr<IMediaEvent, &IID_IMediaEvent> mediaEvent(graph);

		MSG msg;
		while (runned)
		{
			long ev = 0;
			LONG_PTR p1 = 0, p2 = 0;

			Sleep(250);

			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				DispatchMessage(&msg);
			}

			while (mediaEvent->GetEvent(&ev, &p1, &p2, 0) == S_OK)
			{
				if ((ev == EC_COMPLETE || ev == EC_USERABORT))
				{
					ATLTRACE("Dummy graph done!\n");
					
					mediaEvent->FreeEventParams(ev, p1, p2);
					runned = false;
				}
				else if (ev == EC_ERRORABORT)
				{
					ATLTRACE("Dummy graph error occured: HRESULT=%x\n", p1);

					mediaControl->Stop();

					mediaEvent->FreeEventParams(ev, p1, p2);
					runned = false;
				}

				mediaEvent->FreeEventParams(ev, p1, p2);
			}
		}
	}

	mediaControl.Release();

	CoUninitialize();

	return 0;
}
