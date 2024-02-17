/**
 * WindowsVersion.h : Defines the windows version helpers
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#ifdef WIN32

#pragma once

#include <windows.h>

namespace Common
{

static bool IsWindowsVistaOrGreater()
{
    OSVERSIONINFOEX osvi = { 0 };
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osvi.dwMajorVersion = 6;
    
    DWORDLONG dwlConditionMask = 0;
    int op = VER_GREATER_EQUAL;

    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMAJOR, op);
    VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMINOR, op);

    return VerifyVersionInfo(
        &osvi,
        VER_MAJORVERSION | VER_MINORVERSION |
        VER_SERVICEPACKMAJOR | VER_SERVICEPACKMINOR,
        dwlConditionMask);
}

static bool IsWindows10OrGreater()
{
    int(__stdcall * RtlGetVersion)(PRTL_OSVERSIONINFOW lpVersionInformation) = nullptr;
    HMODULE ntdll = GetModuleHandle(L"ntdll.dll");
    if (ntdll == NULL)
        return false;
    *reinterpret_cast<FARPROC*>(&RtlGetVersion) = GetProcAddress(ntdll, "RtlGetVersion");
    if (RtlGetVersion == nullptr)
        return false;

    OSVERSIONINFOEX versionInfo{ sizeof(OSVERSIONINFOEX) };
    if (RtlGetVersion(reinterpret_cast<LPOSVERSIONINFO>(&versionInfo)) < 0)
        return false;

    if (versionInfo.dwMajorVersion >= HIBYTE(_WIN32_WINNT_WIN10))
        return true;

    return false;
}

}

#endif
