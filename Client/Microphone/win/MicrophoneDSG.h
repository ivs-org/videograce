/**
 * MicrophoneDSG.h - Contains interface of microphone capture direct show graph
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <Device/DS/DSCommon.h>
#include <Device/DS/SampleGrabber.h>

#include <Audio/Resampler.h>

struct IFilterGraph2;
typedef CComQIPtr<IAMStreamConfig, &IID_IAMStreamConfig> StreamConfigMicroPtr;
typedef CComQIPtr<IAMAudioInputMixer, &IID_IAMAudioInputMixer> MicroMixerPtr;

namespace MicrophoneNS
{

HRESULT BuildGraph(IFilterGraph2 *pGraph,
    std::string_view deviceName,
    int gain,
    int frequency,
    bool strero,
    void *inst,
    MANAGEDCALLBACKPROC outputCallback,
    MicroMixerPtr &mixer);

}
