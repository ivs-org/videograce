/**
 * DSCommon.h - Contains header of common direct show helpers
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <atlbase.h>
#include <dshow.h>

#include <cstdint>
#include <string>
#include <vector>

#include <Device/Device.h>

DEFINE_GUID(MEDIASUBTYPE_I420, 0x30323449, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

BOOL hrcheck(HRESULT hr, char* errtext);

CComPtr<IPin> GetPin(IBaseFilter *pFilter, LPCOLESTR pinname);
CComPtr<IPin> GetFirstOutPin(IBaseFilter *pFilter);
CComPtr<IPin> GetSecondOutPin(IBaseFilter *pFilter);
CComPtr<IPin> GetFirstInPin(IBaseFilter *pFilter);
CComPtr<IPin> GetSecondInPin(IBaseFilter *pFilter);

#define CHECK_HR(hr, msg) if (hrcheck(hr, msg)) return hr;

typedef void(*CaptureCallback)(uint8_t* data, int len, void *instance);

CComPtr<IBaseFilter> CreateFilterByName(std::string_view name, const GUID &category);

HRESULT GetCategoryFilters(const GUID &category, std::vector<Client::Device> &devices);
