/**
* Camera.cpp - Contains camera's impl
*
* Author: Anton (ud) Golovkov, udattsk@gmail.com
* Copyright (C), Infinity Video Soft LLC, 2014
*/

#include <Camera/win/CameraImpl.h>

#include <Transport/RTP/RTPPacket.h>

#include <Common/Common.h>
#include <Common/ShortSleep.h>

#include <Ks.h>
#include <Ksmedia.h>

#include <assert.h>
#include <ippcc.h>
#include <ippi.h>

#include <boost/nowide/convert.hpp>

namespace Camera
{

CameraImpl::CameraImpl(Common::TimeMeter &timeMeter_, Transport::ISocket &receiver_)
	: timeMeter(timeMeter_),
    receiver(receiver_),
    deviceNotifyCallback(),
	bufferMutex(),
	captureBuffer(), outputBuffer(), tmpBuffer(),
	packetDuration(40000),
	dataLength(0),
	processTime(0),
	overTimeCount(0),
	streamConfig(),
	ksPropertySet(),
	cameraControl(),
	mediaControl(),
	name(),
	deviceId(0),
	resolution(Video::rVGA),
	colorSpace(Video::ColorSpace::I420),
	runned(false),
	restarting(false),
	captureThread(), sendThread(),
	sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{
}

CameraImpl::~CameraImpl()
{
	Stop();
}

void CameraImpl::Move(MoveAxis axis, MoveType type, int value)
{
	if (!ksPropertySet)
	{
		return;
	}

	KSPROPERTY_CAMERACONTROL_S control;
	ZeroMemory(&control, sizeof(KSPROPERTY_CAMERACONTROL_S));

	control.Value = value;
	control.Flags = type == MoveType::Relative ? KSPROPERTY_CAMERACONTROL_FLAGS_RELATIVE : KSPROPERTY_CAMERACONTROL_FLAGS_ABSOLUTE;

	DWORD dwPropId = KSPROPERTY_CAMERACONTROL_TILT_RELATIVE;

	if (type == MoveType::Relative)
	{
		if (axis == MoveAxis::Vertical)
		{
			dwPropId = KSPROPERTY_CAMERACONTROL_TILT_RELATIVE;
		}
		else
		{
			dwPropId = KSPROPERTY_CAMERACONTROL_PAN_RELATIVE;
		}
	}
	else
	{
		if (axis == MoveAxis::Vertical)
		{
			dwPropId = KSPROPERTY_CAMERACONTROL_TILT;
		}
		else
		{
			dwPropId = KSPROPERTY_CAMERACONTROL_PAN;
		}
	}

	HRESULT ret = ksPropertySet->Set(PROPSETID_VIDCAP_CAMERACONTROL, dwPropId, &control, sizeof(control), &control, sizeof(control));
	if (!SUCCEEDED(ret))
	{
		errLog->warn("Error in camera ksPropertySet->Set(...), hresult: {0:x}", (int)ret);
		return;
	}

	if (type == MoveType::Relative)
	{
		Sleep(axis == MoveAxis::Vertical ? 100 : 20);

		control.Value = 0;
		
		ret = ksPropertySet->Set(PROPSETID_VIDCAP_CAMERACONTROL, dwPropId, &control, sizeof(control), &control, sizeof(control));
		if (!SUCCEEDED(ret))
		{
			errLog->warn("Error in camera second ksPropertySet->Set(...), hresult: {0:x}", (int)ret);
		}
	}
}

int CameraImpl::GetCurrentZoom()
{
	long oldZoom = 0;
	CameraControlFlags oldFlags = CameraControl_Flags_Manual;
	cameraControl->Get(CameraControl_Zoom, &oldZoom, (long*)&oldFlags);
	return oldZoom;
}

void CameraImpl::Zoom(int value)
{
	if (!cameraControl)
	{
		return;
	}

	long min_, max_, step, def;
	CameraControlFlags flag;
	cameraControl->GetRange(CameraControl_Zoom, &min_, &max_, &step, &def, (long*)&flag);

	int oldZoom = GetCurrentZoom();
	int newZoom = def;
	if (value > 0)
		newZoom = oldZoom + 10;
	else if (value < 0)
		newZoom = oldZoom - 10;

	newZoom = max(min_, newZoom);
	newZoom = min(max_, newZoom);
	cameraControl->Set(CameraControl_Zoom, newZoom, CameraControl_Flags_Manual);
}

void CameraImpl::SetName(std::string_view name_)
{
	name = name_;
	if (runned)
	{
		Stop();
		Start(colorSpace);
	}
}

void CameraImpl::SetDeviceId(uint32_t id)
{
	deviceId = id;
}

void CameraImpl::Start(Video::ColorSpace colorSpace_)
{
	if (!runned)
	{
		colorSpace = colorSpace_;

		Video::ResolutionValues rv = Video::GetValues(resolution);

		size_t bufferSize = 0;
		switch (colorSpace)
		{
			case Video::ColorSpace::I420:
				bufferSize = (size_t)(rv.width * rv.height * 1.5);
			break;
			case Video::ColorSpace::YUY2: case Video::ColorSpace::UYVU:
				bufferSize = rv.width * rv.height * 2;
			break;
			case Video::ColorSpace::RGB24: case Video::ColorSpace::MJPG:
				bufferSize = rv.width * rv.height * 3;
			break;
			case Video::ColorSpace::RGB32:
				bufferSize = rv.width * rv.height * 4;
			break;
			default:
			break;
		}
		try
		{
			captureBuffer = std::unique_ptr<uint8_t[]>(new uint8_t[bufferSize]);
			outputBuffer = std::unique_ptr<uint8_t[]>(new uint8_t[bufferSize]);
			
			if (colorSpace == Video::ColorSpace::RGB24 || colorSpace == Video::ColorSpace::RGB32 || colorSpace == Video::ColorSpace::MJPG)
			{
				tmpBuffer = std::unique_ptr<uint8_t[]>(new uint8_t[bufferSize]);
			}
		}
		catch (std::bad_alloc &)
		{
			if (deviceNotifyCallback)
			{
                deviceNotifyCallback(name, Client::DeviceNotifyType::MemoryError, Proto::DeviceType::Camera, deviceId, 0);
			}
			return;
		}

		dataLength = 0;

		overTimeCount = 0;

		processTime = 0;
		
		runned = true;
		captureThread = std::thread(&CameraImpl::Capture, this);
		sendThread = std::thread(&CameraImpl::send, this);
	}
}

void CameraImpl::Stop()
{
    runned = false;

	if (mediaControl)
	{
		mediaControl->Stop();
	}

	if (captureThread.joinable()) captureThread.join();
	if (sendThread.joinable()) sendThread.join();

	tmpBuffer.reset(nullptr);
	outputBuffer.reset(nullptr);
	captureBuffer.reset(nullptr);
}

HRESULT CameraImpl::SetCameraParams(const Video::ResolutionValues &rv)
{
	if (streamConfig)
	{
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
		//pCameraFormat.dwBitRate = 221184000;
		pCameraFormat.bmiHeader.biSize = 40;
		pCameraFormat.bmiHeader.biWidth = rv.width;
		pCameraFormat.bmiHeader.biHeight = rv.height;
		pCameraFormat.bmiHeader.biPlanes = 1;
		switch (colorSpace)
		{
			case Video::ColorSpace::I420:
				pCameraFormat.bmiHeader.biBitCount = 12;
			break;
			case Video::ColorSpace::YUY2: case Video::ColorSpace::UYVU:
				pCameraFormat.bmiHeader.biBitCount = 16;
			break;
			case Video::ColorSpace::RGB24: case Video::ColorSpace::MJPG:
				pCameraFormat.bmiHeader.biBitCount = 24;
			break;
			case Video::ColorSpace::RGB32:
				pCameraFormat.bmiHeader.biBitCount = 32;
			break;
		}
		pCamera_pmt.lSampleSize = 0; // (ULONG)(pCameraFormat.bmiHeader.biWidth * pCameraFormat.bmiHeader.biHeight * (isYUY2 ? 2 : 1.5));
		pCameraFormat.bmiHeader.biSizeImage = pCamera_pmt.lSampleSize;
		pCamera_pmt.pbFormat = (BYTE*)&pCameraFormat;

		HRESULT hr = streamConfig->SetFormat(&pCamera_pmt);
		if (!SUCCEEDED(hr))
		{
			if (deviceNotifyCallback && runned)
			{
                deviceNotifyCallback(name, Client::DeviceNotifyType::CameraError, Proto::DeviceType::Camera, deviceId, hr);
			}
		}
		CHECK_HR(hr, "Can't set camera format");
	}
	return S_OK;
}

bool CameraImpl::SetResolution(Video::Resolution resolution_)
{
	if (runned)
	{
		Stop();
		resolution = resolution_;
		Start(colorSpace);
	}
	else
	{
		resolution = resolution_;
	}

	return true;
}

void CameraImpl::SetFrameRate(uint32_t rate)
{
	packetDuration = (1000 / rate) * 1000;
}

void CameraImpl::SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback_)
{
    deviceNotifyCallback = deviceNotifyCallback_;
}

HRESULT CameraImpl::Capture()
{
	CoInitialize(NULL);
	CComPtr<IFilterGraph2> graph;
	graph.CoCreateInstance(CLSID_FilterGraph);

	HRESULT hr = BuildGraph(graph, name, resolution, colorSpace, this, &CameraImpl::OutputCallback, streamConfig, ksPropertySet, cameraControl);
	if (SUCCEEDED(hr))
	{
		mediaControl = CComQIPtr<IMediaControl, &IID_IMediaControl>(graph);
		hr = mediaControl->Run();
		if (SUCCEEDED(hr))
		{
            sysLog->info("Camera {0} successfully started", name);
		}
        else
        {
            if (deviceNotifyCallback && runned)
            {
                deviceNotifyCallback(name, Client::DeviceNotifyType::CameraError, Proto::DeviceType::Camera, deviceId, hr);
            }
            runned = false;
            errLog->critical("Camera {0} start error {1}", name, (uint32_t)hr);
        }

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
				if ((ev == EC_COMPLETE || ev == EC_USERABORT) && !restarting)
				{
					sysLog->warn("Video Capture graph done!\n");
					
					mediaEvent->FreeEventParams(ev, p1, p2);
					runned = false;
				}
				else if (ev == EC_ERRORABORT)
				{
					errLog->error("Video Capture graph error occured: HRESULT {0:x}\n", (uint32_t)p1);
					
					mediaControl->Stop();
					if (deviceNotifyCallback && runned)
					{
                        deviceNotifyCallback(name, Client::DeviceNotifyType::CameraError, Proto::DeviceType::Camera, deviceId, static_cast<uint32_t>(p1));
					}
					
					mediaEvent->FreeEventParams(ev, p1, p2);
					runned = false;
				}
				mediaEvent->FreeEventParams(ev, p1, p2);
			}
		}
	}
	else
	{
		if (deviceNotifyCallback && runned)
		{
            deviceNotifyCallback(name, Client::DeviceNotifyType::CameraError, Proto::DeviceType::Camera, deviceId, hr);
		}
	}

	streamConfig.Release();
	ksPropertySet.Release();
	cameraControl.Release();
	mediaControl.Release();

	graph.Release();

	CoUninitialize();

	return 0;
}

void CameraImpl::ConvertFromYUY2(unsigned char* data_, int *len_, CameraImpl &instance)
{
	const Video::ResolutionValues rv = Video::GetValues(instance.resolution);

	const IppiSize  sz = { rv.width, rv.height };
	Ipp8u*          dst[3] = { instance.captureBuffer.get(), instance.captureBuffer.get() + (rv.width * rv.height) + ((rv.width * rv.height) / 4), instance.captureBuffer.get() + (rv.width * rv.height) };
	int             dstStep[3] = { rv.width, rv.width / 2, rv.width / 2 };
	ippiYCbCr422ToYCrCb420_8u_C2P3R(data_, rv.width * 2, dst, dstStep, sz);

	*len_ = (rv.width * rv.height) + ((rv.width * rv.height) / 2);
}

void CameraImpl::ConvertFromUYVU(unsigned char* data_, int *len_, CameraImpl &instance)
{
	const Video::ResolutionValues rv = Video::GetValues(instance.resolution);

	const IppiSize  sz = { rv.width, rv.height };
	Ipp8u*          dst[3] = { instance.captureBuffer.get(), instance.captureBuffer.get() + (rv.width * rv.height) + ((rv.width * rv.height) / 4), instance.captureBuffer.get() + (rv.width * rv.height) };
	int             dstStep[3] = { rv.width, rv.width / 2, rv.width / 2 };
	ippiCbYCr422ToYCrCb420_8u_C2P3R(data_, rv.width * 2, dst, dstStep, sz);

	*len_ = (rv.width * rv.height) + ((rv.width * rv.height) / 2);
}

void CameraImpl::ConvertFromRGB24(unsigned char* data_, int *len_, CameraImpl &instance)
{
	const Video::ResolutionValues rv = Video::GetValues(instance.resolution);

	const IppiSize  sz = { rv.width, rv.height };
	Ipp8u*          dst[3] = { instance.captureBuffer.get(), instance.captureBuffer.get() + (rv.width * rv.height), instance.captureBuffer.get() + (rv.width * rv.height) + ((rv.width * rv.height)/4) };
	int             dstStep[3] = { rv.width, rv.width / 2, rv.width / 2 };
	
	ippiMirror_8u_C3R(data_, rv.width * 3, instance.tmpBuffer.get(), rv.width * 3, sz, ippAxsHorizontal);
	ippiBGRToYCbCr420_8u_C3P3R(instance.tmpBuffer.get(), rv.width * 3, dst, dstStep, sz);
		
	*len_ = (rv.width * rv.height) + ((rv.width * rv.height)/2);
}

void CameraImpl::ConvertFromRGB32(unsigned char* data_, int *len_, CameraImpl &instance)
{
	const Video::ResolutionValues rv = Video::GetValues(instance.resolution);

	const IppiSize  sz = { rv.width, rv.height };
	Ipp8u*          dst[3] = { instance.captureBuffer.get(), instance.captureBuffer.get() + (rv.width * rv.height), instance.captureBuffer.get() + (rv.width * rv.height) + ((rv.width * rv.height) / 4) };
	int             dstStep[3] = { rv.width, rv.width / 2, rv.width / 2 };

	ippiMirror_8u_C3R(data_, rv.width * 3, instance.tmpBuffer.get(), rv.width * 3, sz, ippAxsHorizontal);
	ippiBGRToYCbCr420_8u_AC4P3R(instance.tmpBuffer.get(), rv.width * 4, dst, dstStep, sz);

	*len_ = (rv.width * rv.height) + ((rv.width * rv.height) / 2);
}

void CALLBACK CameraImpl::OutputCallback(uint8_t* data, int len, void *instance_)
{
    CameraImpl &instance = *static_cast<CameraImpl*>(instance_);

	std::lock_guard<std::mutex> lock(instance.bufferMutex);
	
	switch (instance.colorSpace)
	{
		case Video::ColorSpace::I420:
			memcpy(instance.captureBuffer.get(), data, len);
		break;
		case Video::ColorSpace::YUY2:
			ConvertFromYUY2(data, &len, instance);
		break;
		case Video::ColorSpace::UYVU:
			ConvertFromUYVU(data, &len, instance);
		break;
		case Video::ColorSpace::RGB24: case Video::ColorSpace::MJPG:
			ConvertFromRGB24(data, &len, instance);
		break;
		case Video::ColorSpace::RGB32:
			ConvertFromRGB32(data, &len, instance);
		break;
	}

	instance.dataLength = len;
}

void CameraImpl::send()
{
	using namespace std::chrono;

	while (runned)
	{
		auto start = timeMeter.Measure();

		uint32_t length = 0;
		{
			std::lock_guard<std::mutex> lock(bufferMutex);

			if (dataLength == 0)
			{
				continue;
			}

			length = dataLength;
			memcpy(outputBuffer.get(), captureBuffer.get(), length);
		}

		Transport::RTPPacket packet;

		packet.rtpHeader.ts = (uint32_t)(start / 1000);
		packet.payload = outputBuffer.get();
		packet.payloadSize = length;

		receiver.Send(packet);

		processTime = timeMeter.Measure() - start;

		if (processTime > packetDuration + 5000)
		{
			++overTimeCount;
		}
		else if (overTimeCount > 0)
		{
			--overTimeCount;
		}
		if (overTimeCount == 100)
		{
			overTimeCount = 10000; // prevent duplicates
			if (deviceNotifyCallback && runned)
			{
                deviceNotifyCallback(name, Client::DeviceNotifyType::OvertimeCoding, Proto::DeviceType::Camera, deviceId, 0);
			}
		}

		if (packetDuration > processTime) Common::ShortSleep(packetDuration - processTime - 500);
	}
}

}
