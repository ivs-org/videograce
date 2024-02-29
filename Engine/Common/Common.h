/**
 * Common.h - Contains Common header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014, 2016
 */

#pragma once

#include <math.h>
#include <string>
#include <locale>
#include <cstdint>

#ifndef _WIN32

#ifdef _DEBUG
#define DBGTRACE printf
#else
#define DBGTRACE(x, ...)
#endif

#ifdef SUBTLE_TRACE
inline void subtle_trace(std::string_view s, int64_t i)
{
	printf("%s, %d", s.data, i);
}
#else
inline void subtle_trace(std::string_view, int64_t) {}
#endif

#else

#include <tchar.h>
#include <atltrace.h>
#define DBGTRACE ATLTRACE

#ifdef SUBTLE_TRACE
inline void subtle_trace(std::string_view s, int64_t i)
{
	OutputDebugStringA(s.data());
	OutputDebugStringA(std::to_string(i).c_str());
	OutputDebugStringA("\n");
}
#else
inline void subtle_trace(std::string_view, int64_t) {}
#endif

#endif
