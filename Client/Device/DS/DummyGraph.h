/**
 * DummyGraph.h - Contains dummy (doing nothing) direct show graph
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2019
 */

#pragma once

#include <thread>
#include <atomic>

#include "DSCommon.h"

class DummyGraph
{
public:
	DummyGraph();
	~DummyGraph();

	void Start();
	void Stop();

private:
	std::atomic<bool> runned;
	std::thread thread;

	CComQIPtr<IMediaControl, &IID_IMediaControl> mediaControl;

	HRESULT Run();
};