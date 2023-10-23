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

#else

#include <tchar.h>
#include <atltrace.h>
#define DBGTRACE ATLTRACE

#endif
