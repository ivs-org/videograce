/**
 * CPUMeter.cpp - Contains CPU usage meter impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <UI/CPUMeter.h>

#include <cmath>

namespace Client
{

#ifdef _WIN32

#include <windows.h>

ULONGLONG ft2ull(FILETIME &ft)
{
    ULARGE_INTEGER ul;
    ul.HighPart = ft.dwHighDateTime;
    ul.LowPart = ft.dwLowDateTime;
    return ul.QuadPart;
}

int32_t GetCurrentUsage()
{
    FILETIME idle, prev_idle;
    FILETIME kernel, prev_kernel;
    FILETIME user, prev_user;
    GetSystemTimes(&prev_idle, &prev_kernel, &prev_user);
    Sleep(1000);
    GetSystemTimes(&idle, &kernel, &user);
    ULONGLONG sys = (ft2ull(user) - ft2ull(prev_user)) +
        (ft2ull(kernel) - ft2ull(prev_kernel));
    
    int cpu = int((sys - ft2ull(idle) + ft2ull(prev_idle)) * 100.0 / sys);
    prev_idle = idle;
    prev_kernel = kernel;
    prev_user = user;
    return cpu;
}

CPUMeter::CPUMeter()
    : runned(true),
    avgUsage(0), iter(0),
    thread()
{
    thread = std::thread([this]() {
        while (runned)
        {
            ++iter;
            
            if (iter == 11)
            {
                iter = 1;
            }
            
            avgUsage += GetCurrentUsage();
            double tmp = (double)avgUsage / iter;
            avgUsage = (int32_t)std::round(tmp);
        }
    });
}

CPUMeter::~CPUMeter()
{
    runned = false;
    if (thread.joinable()) thread.join();
}

int32_t CPUMeter::GetAVGUsage() const
{
    return avgUsage;
}

#elif __linux__

CPUMeter::CPUMeter()
    : runned(true),
    avgUsage(0), iter(0),
    thread()
{
    /*thread = std::thread([this]() {
        while (runned)
        {
            ++iter;

            if (iter == 11)
            {
                iter = 1;
            }

            avgUsage += GetCurrentUsage();
            double tmp = (double)avgUsage / iter;
            avgUsage = (int32_t)std::round(tmp);
        }
    });*/
}

CPUMeter::~CPUMeter()
{
    runned = false;
    if (thread.joinable()) thread.join();
}

int32_t CPUMeter::GetAVGUsage() const
{
    return avgUsage;
}

#endif

}
