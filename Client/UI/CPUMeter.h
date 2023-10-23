/**
 * CPUMeter.h - Contains CPU usage meter header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021 - 2022
 */

#pragma once

#include <cstdint>
#include <thread>

namespace Client
{
class CPUMeter
{
public:
    CPUMeter();
    ~CPUMeter();
    
    int32_t GetAVGUsage() const;

private:
    bool runned;

    int32_t avgUsage, iter;
    std::thread thread;
};

}
