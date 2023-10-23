/**
 * TimeMeter.h - Contains timer meter
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#include <cstdint>

#ifndef _WIN32
#include <time.h>
#else
#include <windows.h>
#endif

namespace Common
{

#ifndef _WIN32
class TimeMeter
{
	struct timespec startTime;
public:
	TimeMeter()
		: startTime()
	{
		Reset();
	}

	void Reset()
	{
		clock_gettime(CLOCK_REALTIME, &startTime);
	}

	uint64_t Measure() /// microseconds
	{
		struct timespec now;
		clock_gettime(CLOCK_REALTIME, &now);

		long long measure = ((now.tv_sec * 1000000) + (now.tv_nsec / 1000)) - ((startTime.tv_sec * 1000000) + (startTime.tv_nsec / 1000));

		return measure;
	}
};
#else
class TimeMeter
{
	LARGE_INTEGER startTime;
	LARGE_INTEGER frequency;
public:
	TimeMeter()
		: startTime(), frequency()
	{
		QueryPerformanceFrequency(&frequency);
		Reset();
	}

	void Reset()
	{
		QueryPerformanceCounter(&startTime);
	}

	uint64_t Measure() /// microseconds
	{
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);

		LARGE_INTEGER measure;
		measure.QuadPart = now.QuadPart - startTime.QuadPart;

		measure.QuadPart *= 1000000;
		measure.QuadPart /= frequency.QuadPart;

		return measure.QuadPart;
	}
};
#endif

}
