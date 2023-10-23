/**
 * CameraDSG.h - Contains interface of camera capture direct show graph
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <Device/DS/DSCommon.h>
#include <Device/DS/SampleGrabber.h>

#include <Video/Resolution.h>
#include <Video/ColorSpace.h>

struct IFilterGraph2;
typedef CComQIPtr<IAMStreamConfig, &IID_IAMStreamConfig> StreamConfigPtr;
typedef CComQIPtr<IKsPropertySet, &IID_IKsPropertySet> KsPropertySetPtr;
typedef CComQIPtr<IAMCameraControl, &IID_IAMCameraControl> AMCameraControlPtr;

namespace Camera
{

HRESULT BuildGraph(IFilterGraph2 *pGraph,
	const std::string &cameraName,
	Video::Resolution resolution,
	Video::ColorSpace colorSpace,
	void *inst,
	MANAGEDCALLBACKPROC outputCallback,
	StreamConfigPtr &streamConfig,
	KsPropertySetPtr &ksPropertySet,
	AMCameraControlPtr &cameraControl);

}

