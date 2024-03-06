/**
* MicrophoneDMO.cpp - Contains DMO microphone's impl
*
* Author: Anton (ud) Golovkov, udattsk@gmail.com
* Copyright (C), Infinity Video Soft LLC, 2014, 2023
*/

#include <Microphone/win/MicrophoneDMO.h>

#include <Transport/RTP/RTPPacket.h>
#include <Transport/RTP/RTPPayloadType.h>

#include <Common/Common.h>
#include <wui/config/config.hpp>

#include <boost/nowide/convert.hpp>

#include <assert.h>

#include <windows.h>
#include <dmo.h>
#include <Mmsystem.h>
#include <objbase.h>
#include <mediaobj.h>
#include <uuids.h>
#include <propidl.h>
#include <wmcodecdsp.h>

#include <atlbase.h>
#include <ATLComCli.h>
#include <audioclient.h>
#include <MMDeviceApi.h>
#include <AudioEngineEndPoint.h>
#include <DeviceTopology.h>
#include <propkey.h>
#include <strsafe.h>
#include <Functiondiscoverykeys_devpkey.h>

#include "MediaBuf.h"
#include "AecKsBinder.h"

#include <Device/DS/DSCommon.h>

#include <Common/ShortSleep.h>

#define SAFE_ARRAYDELETE(p) {if (p) delete[] (p); (p) = NULL;}
#define SAFE_RELEASE(p) {if (NULL != p) {(p)->Release(); (p) = NULL;}}

class CStaticMediaBuffer : public CBaseMediaBuffer
{
public:
    STDMETHODIMP_(ULONG) AddRef(){ return 2; }
    STDMETHODIMP_(ULONG) Release() { return 1; }
    
    void Init(BYTE *pData, ULONG ulSize, ULONG ulData)
    {
        m_pData = pData;
        m_ulSize = ulSize;
        m_ulData = ulData;
    }
};

namespace MicrophoneNS
{

MicrophoneDMO::MicrophoneDMO(Common::TimeMeter &timeMeter_, Transport::ISocket &receiver_)
	: timeMeter(timeMeter_),
    receiver(receiver_),
    deviceNotifyCallback(),
    deviceName(),
	deviceId(0),
    ssrc(0), seq(0),
    dmoDeviceID(),
    gain(wui::config::get_int("CaptureDevices", "MicrophoneGain", 80)),
	mute(false),
	runned(false),
	restarting(false),
	thread(),
	sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{
}

MicrophoneDMO::~MicrophoneDMO()
{
	Stop();
}

void MicrophoneDMO::SetDeviceName(std::string_view name)
{
	deviceName = name;
	if (runned)
	{
		Stop();
		Start(ssrc);
	}
}

void MicrophoneDMO::SetDeviceId(uint32_t id)
{
	deviceId = id;
}

void MicrophoneDMO::Start(ssrc_t ssrc_)
{
    if (!runned)
	{
        ssrc = ssrc_;
        seq = 0;

		runned = true;
		thread = std::thread(&MicrophoneDMO::Run, this);
	}
}

void MicrophoneDMO::Stop()
{
	runned = false;
    
    dmoDeviceID.clear();

	if (thread.joinable()) thread.join();
}

void MicrophoneDMO::SetGain(uint16_t gain_)
{
    wui::config::set_int("CaptureDevices", "MicrophoneGain", gain_);

	gain = gain_;
	
    if (dmoDeviceID.empty())
    {
        return;
    }

    CComPtr<IMMDeviceEnumerator> deviceEnumerator;
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator);
    if (hrcheck(hr, "IMMDeviceEnumerator error\n")) return;
    
    CComPtr<IMMDevice> mmDevice;
    hr = deviceEnumerator->GetDevice(dmoDeviceID.c_str(), &mmDevice);
    if (hrcheck(hr, "deviceEnumerator->GetDevice error\n")) return;

    CComPtr<IAudioEndpointVolume> endpointVolume;
    hr = mmDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume);
    if (hrcheck(hr, "IAudioEndpointVolume error\n")) return;
    
    float newGain = static_cast<float>(gain);
    newGain /= 100;

    hr = endpointVolume->SetMasterVolumeLevelScalar(newGain, NULL);
    hrcheck(hr, "endpointVolume->SetMasterVolumeLevelScalar error\n\n");    
}

uint16_t MicrophoneDMO::GetGain() const
{
    return gain;
}

void MicrophoneDMO::SetMute(bool yes)
{
	mute = yes;
}

bool MicrophoneDMO::GetMute() const
{
	return mute;
}

void MicrophoneDMO::SetSampleFreq(int32_t)
{
    /// Not support, DMO can play only 16000 (8000; 11025; 16000; or 22050)
}

int32_t MicrophoneDMO::GetSampleFreq() const
{
    return 16000;
}

void MicrophoneDMO::SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback_)
{
    deviceNotifyCallback = deviceNotifyCallback_;
}

HRESULT MicrophoneDMO::Run()
{
    CoInitializeEx(NULL, 0);

    HRESULT hr = S_OK;

    CComPtr<IMediaObject> pDMO = NULL;
    CComPtr <IPropertyStore> pPS = NULL;

    // DMO initialization
    hr = CoCreateInstance(CLSID_CWMAudioAEC, NULL, CLSCTX_INPROC_SERVER, IID_IMediaObject, (void**)&pDMO);
    CHECK_HR(hr, "Can't create DMO\n");
    hr = pDMO->QueryInterface(IID_IPropertyStore, (void**)&pPS);
    CHECK_HR(hr, "Can't get property store\n");

    CStaticMediaBuffer outputBuffer;
    DMO_OUTPUT_DATA_BUFFER OutputBufferStruct = { 0 };
    OutputBufferStruct.pBuffer = &outputBuffer;
    DMO_MEDIA_TYPE mt = { 0 };

    ULONG cbProduced = 0;
    DWORD dwStatus;

    int iMicDevIdx = 0; // microphone device index
    UINT uCapDevCount = 0;
    hr = GetCaptureDeviceNum(uCapDevCount);
    CHECK_HR(hr, "GetCaptureDeviceNum failed\n");

    AUDIO_DEVICE_INFO *pCaptureDeviceInfo = new AUDIO_DEVICE_INFO[uCapDevCount];
    hr = EnumCaptureDevice(uCapDevCount, pCaptureDeviceInfo);
    CHECK_HR(hr, "EnumCaptureDevice failed\n");

    for (int i = 0; i < (int)uCapDevCount; i++)
    {
        if (std::wstring(pCaptureDeviceInfo[i].szDeviceName).find(boost::nowide::widen(deviceName)) != std::wstring::npos)
        {
            dmoDeviceID = pCaptureDeviceInfo[i].szDeviceID;
            iMicDevIdx = i;
            ATLTRACE(L"microphone %i - %s\n", i, deviceName.c_str());
            break;
        }
    }

    SetGain(gain);

    DWORD cOutputBufLen = 0;
    BYTE *pbOutputBuffer = NULL;

    WAVEFORMATEX wfxOut = { WAVE_FORMAT_PCM, 1, 16000, 16000 * 2, 2, 16, 0 };

    PROPVARIANT pvSysMode;
    PropVariantInit(&pvSysMode);
    pvSysMode.vt = VT_I4;
    pvSysMode.lVal = SINGLE_CHANNEL_NSAGC;
    hr = pPS->SetValue(MFPKEY_WMAAECMA_SYSTEM_MODE, pvSysMode);
    CHECK_HR(hr, "Can't Set SYSTEM_MODE\n");
    PropVariantClear(&pvSysMode);

    PROPVARIANT pvDeviceId;
    PropVariantInit(&pvDeviceId);
    pvDeviceId.vt = VT_I4;
    pvDeviceId.lVal = (unsigned long)(0 << 16) + (unsigned long)(0x0000ffff & iMicDevIdx);
    hr = pPS->SetValue(MFPKEY_WMAAECMA_DEVICE_INDEXES, pvDeviceId);
    CHECK_HR(hr, "Can't Set devices ids\n");
    hr = pPS->GetValue(MFPKEY_WMAAECMA_DEVICE_INDEXES, &pvDeviceId);
    CHECK_HR(hr, "Can't Get devices ids\n");
    PropVariantClear(&pvDeviceId);

    // Turn on feature modes
    PROPVARIANT pvFeatrModeOn;
    PropVariantInit(&pvFeatrModeOn);
    pvFeatrModeOn.vt = VT_BOOL;
    pvFeatrModeOn.boolVal = VARIANT_TRUE;
    hr = pPS->SetValue(MFPKEY_WMAAECMA_FEATURE_MODE, pvFeatrModeOn);
    CHECK_HR(hr, "Can't Start FEATURE_MODE\n");
    PropVariantClear(&pvFeatrModeOn);

    // Set output frame size (20 ms)
    PROPVARIANT pvFrameSize;
    PropVariantInit(&pvFrameSize);
    pvFrameSize.vt = VT_I4;
    pvFrameSize.lVal = 320;
    hr = pPS->SetValue(MFPKEY_WMAAECMA_FEATR_FRAME_SIZE, pvFrameSize);
    PropVariantClear(&pvFrameSize);

    // Turn off noise suppression
    PROPVARIANT pvNoiseSup;
    PropVariantInit(&pvNoiseSup);
    pvNoiseSup.vt = VT_I4;
    pvNoiseSup.lVal = 0;
    hr = pPS->SetValue(MFPKEY_WMAAECMA_FEATR_NS, pvNoiseSup);
    CHECK_HR(hr, "MFPKEY_WMAAECMA_FEATR_NS error\n");
    PropVariantClear(&pvNoiseSup);

    // Turn on AGC
    PROPVARIANT pvAGC;
    PropVariantInit(&pvAGC);
    pvAGC.vt = VT_BOOL;
    pvAGC.boolVal = wui::config::get_int("CaptureDevices", "MicrophoneNativeAGC", 1) == 1 ? VARIANT_TRUE : VARIANT_FALSE;
    hr = pPS->SetValue(MFPKEY_WMAAECMA_FEATR_AGC, pvAGC);
    CHECK_HR(hr, "MFPKEY_WMAAECMA_FEATR_AGC error\n");
    PropVariantClear(&pvAGC);

    // Set DMO output format
    hr = MoInitMediaType(&mt, sizeof(WAVEFORMATEX));
    CHECK_HR(hr, "MoInitMediaType failed\n");

    mt.majortype = MEDIATYPE_Audio;
    mt.subtype = MEDIASUBTYPE_PCM;
    mt.lSampleSize = 0;
    mt.bFixedSizeSamples = TRUE;
    mt.bTemporalCompression = FALSE;
    mt.formattype = FORMAT_WaveFormatEx;
    memcpy(mt.pbFormat, &wfxOut, sizeof(WAVEFORMATEX));

    hr = pDMO->SetOutputType(0, &mt, 0);
    CHECK_HR(hr, "SetOutputType failed\n");
    MoFreeMediaType(&mt);

    // allocate output buffer
    cOutputBufLen = wfxOut.nSamplesPerSec * wfxOut.nBlockAlign;
    pbOutputBuffer = new BYTE[cOutputBufLen];

    const uint64_t PACKET_DURATION = 40000;

    using namespace std::chrono;

    while (runned)
    {
        auto start = high_resolution_clock::now();

        auto sendTime = timeMeter.Measure();

        do
        {
            outputBuffer.Init((byte*)pbOutputBuffer, cOutputBufLen, 0);
            OutputBufferStruct.dwStatus = 0;
            hr = pDMO->ProcessOutput(0, 1, &OutputBufferStruct, &dwStatus);

            CHECK_HR (hr, "ProcessOutput failed\n");

            if (hr == S_FALSE)
            {
                cbProduced = 0;
            }
            else
            {
                hr = outputBuffer.GetBufferAndLength(NULL, &cbProduced);
                if (hr != S_OK)
                {
                    ATLTRACE("GetBufferAndLength failed %d\n", hr);
                }
            }

            if (cbProduced != 0)
            {
                Transport::RTPPacket packet;
                packet.rtpHeader.ts = static_cast<uint32_t>(timeMeter.Measure());
                packet.rtpHeader.pt = static_cast<uint32_t>(Transport::RTPPayloadType::ptPCM);
                packet.rtpHeader.ssrc = ssrc;
                packet.rtpHeader.seq = ++seq;
                packet.payload = pbOutputBuffer;
                packet.payloadSize = cbProduced;

                receiver.Send(packet);
            }
        }
        while (OutputBufferStruct.dwStatus & DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE);

        int64_t captureDuration = duration_cast<microseconds>(high_resolution_clock::now() - start).count();

        if (PACKET_DURATION > captureDuration) Common::ShortSleep(PACKET_DURATION - captureDuration - 500);
    }

    SAFE_ARRAYDELETE(pbOutputBuffer);
    SAFE_ARRAYDELETE(pCaptureDeviceInfo);

    CoUninitialize();

    return hr;
}

}
