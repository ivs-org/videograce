/**
 * ShortSleep.cpp - Contains short sleep method impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2019
 */

#include <Common/ShortSleep.h>

#ifdef _WIN32
#include <Windows.h>

static NTSTATUS(__stdcall *NtDelayExecution)(BOOL Alertable, PLARGE_INTEGER DelayInterval) = (NTSTATUS(__stdcall*)(BOOL, PLARGE_INTEGER)) GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtDelayExecution");

#else
#include <unistd.h>
#endif

namespace Common
{

void ShortSleep(uint32_t microseconds)
{
#ifdef _WIN32
	LARGE_INTEGER interval;
	interval.QuadPart = -1 * static_cast<int32_t>(microseconds) * 10;
	NtDelayExecution(true, &interval);
#else
	usleep(microseconds);
#endif
}

}
