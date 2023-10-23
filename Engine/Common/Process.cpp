/**
 * Process.cpp - Contains implements of process helpers
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2019
 */

#include <Common/Process.h>

#ifdef _WIN32

#include <windows.h>

namespace Common
{

bool IsForegroundProcess()
{
	HWND hwnd = GetForegroundWindow();
	if (hwnd == NULL) return false;

	DWORD foregroundPid;
	if (GetWindowThreadProcessId(hwnd, &foregroundPid) == 0) return false;

	return (foregroundPid == GetCurrentProcessId());
}

bool Is64BitSystem()
{
#if _WIN64
	return true;

#elif _WIN32
	typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

	//IsWow64Process is not available on all supported versions of Windows.
	//Use GetModuleHandle to get a handle to the DLL that contains the function
	//and GetProcAddress to get a pointer to the function if available.
	
	LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)
		GetProcAddress(GetModuleHandle(L"kernel32"), "IsWow64Process");

	BOOL isWow64 = FALSE;
	if (fnIsWow64Process && fnIsWow64Process(GetCurrentProcess(), &isWow64))
	{
		if (isWow64)
		{
			return true;
		}
	}
	
	return false;
#endif
}

}

#else

namespace Common
{

bool IsForegroundProcess()
{
	// ToDo!

	return true;
}

bool Is64BitSystem()
{
#ifdef __APPLE__
	return true;
#else
#if defined(__LP64__) || defined(_LP64)
	return true;
#else
	return false;
#endif
#endif
}

}

#endif
