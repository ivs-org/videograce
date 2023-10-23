/**
 * DSCommon.cpp - Contains impl of common direct show helpers
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include "DSCommon.h"
#include <Streams.h>

#include <Common/Common.h>

#include <algorithm>
#include <functional>

#include <boost/nowide/convert.hpp>

CComPtr<IPin> GetPin(IBaseFilter *pFilter, LPCOLESTR pinname)
{
	CComPtr<IEnumPins> pEnum;
	CComPtr<IPin>	   pPin;
	
	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (hrcheck(hr, "Can't enumerate pins."))
		return NULL;

	while(pEnum->Next(1, &pPin, 0) == S_OK)
	{
		PIN_INFO pinfo;
		pPin->QueryPinInfo(&pinfo);
		BOOL found = !_wcsicmp(pinname, pinfo.achName);
		if (pinfo.pFilter)
			pinfo.pFilter->Release();
		if (found)
			return pPin;
		pPin.Release();
	}
	ATLTRACE ("Pin not found!\n");
	return NULL;
}

CComPtr<IPin> GetFirstOutPin(IBaseFilter *pFilter)
{
	CComPtr<IEnumPins> pEnum;
	CComPtr<IPin>	   pPin;

	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (hrcheck(hr, "Can't enumerate pins."))
		return NULL;

	while (pEnum->Next(1, &pPin, 0) == S_OK)
	{
		PIN_INFO pinfo;
		pPin->QueryPinInfo(&pinfo);
		if (pinfo.dir == PINDIR_OUTPUT)
		{
			if (pinfo.pFilter)
				pinfo.pFilter->Release();
			return pPin;
		}
		if (pinfo.pFilter)
			pinfo.pFilter->Release();
		pPin.Release();
	}

	ATLTRACE("No out pins!\n");
	return NULL;
}

CComPtr<IPin> GetSecondOutPin(IBaseFilter *pFilter)
{
	CComPtr<IEnumPins> pEnum;
	CComPtr<IPin>	   pPin;

	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (hrcheck(hr, "Can't enumerate pins."))
		return NULL;

	int number = 0;

	while (pEnum->Next(1, &pPin, 0) == S_OK)
	{
		PIN_INFO pinfo;
		pPin->QueryPinInfo(&pinfo);
		if (pinfo.dir == PINDIR_OUTPUT)
		{
			++number;

			if (number == 2)
			{
				if (pinfo.pFilter)
					pinfo.pFilter->Release();

				return pPin;
			}
		}
		if (pinfo.pFilter)
			pinfo.pFilter->Release();
		pPin.Release();
	}

	ATLTRACE("No out pins!\n");
	return NULL;
}

CComPtr<IPin> GetFirstInPin(IBaseFilter *pFilter)
{
	CComPtr<IEnumPins> pEnum;
	CComPtr<IPin>	   pPin;

	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (hrcheck(hr, "Can't enumerate pins."))
		return NULL;

	while (pEnum->Next(1, &pPin, 0) == S_OK)
	{
		PIN_INFO pinfo;
		pPin->QueryPinInfo(&pinfo);
		if (pinfo.dir == PINDIR_INPUT)
		{
			if (pinfo.pFilter)
				pinfo.pFilter->Release();
			return pPin;
		}
		if (pinfo.pFilter)
			pinfo.pFilter->Release();
		pPin.Release();
	}

	ATLTRACE("No input pins!\n");
	return NULL;
}

CComPtr<IPin> GetSecondInPin(IBaseFilter *pFilter)
{
	CComPtr<IEnumPins> pEnum;
	CComPtr<IPin>	   pPin;

	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (hrcheck(hr, "Can't enumerate pins."))
		return NULL;

	int number = 0;

	while (pEnum->Next(1, &pPin, 0) == S_OK)
	{
		PIN_INFO pinfo;
		pPin->QueryPinInfo(&pinfo);
		if (pinfo.dir == PINDIR_INPUT)
		{
			++number;

			if (number == 2)
			{
				if (pinfo.pFilter)
					pinfo.pFilter->Release();
				return pPin;
			}
		}
		if (pinfo.pFilter)
			pinfo.pFilter->Release();
		pPin.Release();
	}

	ATLTRACE("No input pins!\n");
	return NULL;
}

BOOL hrcheck(HRESULT hr, char* errtext)
{
	if (hr >= S_OK)
		return FALSE;
	char szErr[MAX_ERROR_TEXT_LEN] = {0};
	DWORD res = AMGetErrorTextA(hr, szErr, MAX_ERROR_TEXT_LEN);
	if (res)
		ATLTRACE ("Error %x: %s\n%s\n", hr, errtext, szErr);
	else
		ATLTRACE ("Error %x: %s\n", hr, errtext);
	
	return TRUE;
}

void ProcessFilterDuplicates(std::vector<Client::Device> &devices)
{
	for (const auto &fv : devices)
	{
		auto currentValue = fv.name;
		int duplicateCount = 0;
		for (auto &fvd : devices)
		{
			if (fvd.name == currentValue)
			{
				++duplicateCount;
				if (duplicateCount > 1)
				{
					fvd.name += " {" + std::to_string(duplicateCount - 1) + "}";
				}
			}
		}
	}
}

HRESULT GetCategoryFilters(const GUID &category, std::vector<Client::Device> &devices)
{
	HRESULT hr = S_OK;
	CComPtr<ICreateDevEnum> pSysDevEnum;
	hr = pSysDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
	CHECK_HR(hr, "Can't create system device enumerator\n");
	
	CComPtr<IEnumMoniker> pEnumCat;
	hr = pSysDevEnum->CreateClassEnumerator(category, &pEnumCat, 0);
	CHECK_HR(hr, "Can't create system device enumerator's moniker\n");
	if (hr == S_FALSE || !pEnumCat)
	{
		return VFW_E_NOT_FOUND;
	}
	
	CComPtr<IMoniker> pMoniker;
	ULONG cFetched = 0;
		
	while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
	{
		CComPtr<IPropertyBag> pPropBag;
		hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag);
		if (SUCCEEDED(hr) && pPropBag)
		{
			CComVariant varName;
			hr = pPropBag->Read(L"FriendlyName", &varName, 0);
			if (SUCCEEDED(hr))
			{
				/// Add the filter
				Client::Device device;
                device.name = boost::nowide::narrow(varName.bstrVal);
				
				/// Get the supported resolutins
				if (category == CLSID_VideoInputDeviceCategory)
				{
					CComPtr<IBaseFilter> pFilter;
					hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&pFilter);
					if (SUCCEEDED(hr) && pFilter)
					{
						CComPtr<IPin> pin = GetFirstOutPin(pFilter);
						if (pin)
						{
							CComPtr<IEnumMediaTypes> mediaTypesEnumerator;
							hr = pin->EnumMediaTypes(&mediaTypesEnumerator);
							if (SUCCEEDED(hr) && mediaTypesEnumerator)
							{
								AM_MEDIA_TYPE *mediaType = NULL;
								while (S_OK == mediaTypesEnumerator->Next(1, &mediaType, NULL))
								{
									if (mediaType)
									{
										VIDEOINFOHEADER *format = (VIDEOINFOHEADER *)mediaType->pbFormat;
										if (format->bmiHeader.biWidth != 0 && format->bmiHeader.biHeight != 0)
										{
											Video::VideoFormat *videoFormat = nullptr;

											Video::Resolution resolution = Video::GetResolution(Video::ResolutionValues((uint16_t)format->bmiHeader.biWidth, (uint16_t)format->bmiHeader.biHeight));
											auto finded = std::find(device.formats.begin(), device.formats.end(), Video::VideoFormat(resolution));
											if (finded != device.formats.end())
											{
												videoFormat = &(*finded);
											}
											else
											{
                                                device.formats.emplace_back(Video::VideoFormat(resolution));
												videoFormat = &device.formats.back();
											}

											Video::ColorSpace colorSpace = Video::ColorSpace::Unsupported;

											if (IsEqualGUID(mediaType->subtype, MEDIASUBTYPE_MJPG))
											{
												colorSpace = Video::ColorSpace::MJPG;
											}
											else if (IsEqualGUID(mediaType->subtype, MEDIASUBTYPE_I420))
											{
												colorSpace = Video::ColorSpace::I420;
											}
											else if (IsEqualGUID(mediaType->subtype, MEDIASUBTYPE_YUY2))
											{
												colorSpace = Video::ColorSpace::YUY2;
											}
											else if (IsEqualGUID(mediaType->subtype, MEDIASUBTYPE_UYVY))
											{
												colorSpace = Video::ColorSpace::UYVU;
											}
											else if (IsEqualGUID(mediaType->subtype, MEDIASUBTYPE_RGB24))
											{
												colorSpace = Video::ColorSpace::RGB24;
											}
											else if (IsEqualGUID(mediaType->subtype, MEDIASUBTYPE_RGB32))
											{
												colorSpace = Video::ColorSpace::RGB32;
											}

											if (colorSpace != Video::ColorSpace::Unsupported &&
												std::find(videoFormat->colorSpaces.begin(), videoFormat->colorSpaces.end(), colorSpace) == videoFormat->colorSpaces.end())
											{
												videoFormat->colorSpaces.emplace_back(colorSpace);
											}
										}

										DeleteMediaType(mediaType);
									}
								}
							}
						}
					}
				}

				if (category != CLSID_VideoInputDeviceCategory ||
					(category == CLSID_VideoInputDeviceCategory && !device.formats.empty()))
				{
					for (auto &format : device.formats)
					{
						std::sort(format.colorSpaces.begin(), format.colorSpaces.end(), [](Video::ColorSpace a, Video::ColorSpace b)
                        {
                            return static_cast<int32_t>(a) < static_cast<int32_t>(b);
                        });
					}
					devices.emplace_back(device);
				}
			}
		}
		pMoniker.Release();
	}
			
	ProcessFilterDuplicates(devices);

	return hr;
}

CComPtr<IBaseFilter> CreateFilterByName(const std::string &name_, const GUID &category)
{
	auto filterName = name_;

	/// Extract duplicate number (if exists) and trim filter name to original
	int duplicateNumber = 0;
	{
		const std::string::size_type startPos = filterName.find("{");
		const std::string::size_type endPos = filterName.find("}");
		if (startPos != std::string::npos && endPos != std::string::npos)
		{
			std::wstring numb;
			for (size_t i = startPos + 1; i != endPos; ++i)
			{
				numb += filterName[i];
			}
			duplicateNumber = atoi(boost::nowide::narrow(numb).c_str());
			filterName.resize(startPos - 1);
		}
	}
	
	HRESULT hr = S_OK;
	CComPtr<ICreateDevEnum> pSysDevEnum;
	hr = pSysDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
	if (hrcheck(hr, "Can't create system device enumerator\n"))
	{
		return NULL;
	}

	CComPtr<IEnumMoniker> pEnumCat;
	hr = pSysDevEnum->CreateClassEnumerator(category, &pEnumCat, 0);
	if (SUCCEEDED(hr) && pEnumCat)
	{
		CComPtr<IMoniker> pMoniker;
		ULONG cFetched;

		int filterNumber = 0;
		while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
		{
			CComPtr<IPropertyBag> pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag);
			if (SUCCEEDED(hr) && pPropBag)
			{
				CComVariant varName;
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if (SUCCEEDED(hr))
				{
					auto dshowDeviceNameStr = boost::nowide::narrow(varName.bstrVal);
					size_t found = dshowDeviceNameStr.find(filterName);
					if (found != std::string::npos)
					{
						if (duplicateNumber != 0)
						{
							if (filterNumber != duplicateNumber)
							{
								pMoniker.Release();
								++filterNumber;
								continue;
							}
						}

						CComPtr<IBaseFilter> pFilter;
						hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&pFilter);
						if (hrcheck(hr, "Can't create the filter\n"))
						{
							return NULL;
						}

						return pFilter;
					}
				}
			}
			pMoniker.Release();
		}
	}
	
	return NULL;
}
