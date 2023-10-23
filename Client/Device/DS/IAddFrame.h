/**
 * IAddFrame.h - Contains interface of the Input Source DirectShow Filter
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#pragma once

// {D312315A-C36F-4B09-BAF4-B3CE922185C2}
static const GUID IID_ILiveSource =
{ 0xd312315a, 0xc36f, 0x4b09, { 0xba, 0xf4, 0xb3, 0xce, 0x92, 0x21, 0x85, 0xc2 } };

// {F5E6F181-748E-428C-830F-FA1919223DD0}
static const GUID CLSID_CLiveSource =
{ 0xf5e6f181, 0x748e, 0x428c, { 0x83, 0xf, 0xfa, 0x19, 0x19, 0x22, 0x3d, 0xd0 } };

DECLARE_INTERFACE_(ILiveSource, IUnknown)
{
	// Adds pixel data buffer to the video sequence
	STDMETHOD(AddFrame)(BYTE* pBuffer, DWORD size) PURE;
	
	// Set the video resolution
	STDMETHOD(SetVideoResolution)(DWORD width, DWORD height) PURE;
};