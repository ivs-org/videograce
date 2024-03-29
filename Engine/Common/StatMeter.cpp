/**
 * StatMeter.cpp - Contains the statistics meter impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2024
 */

#include <Common/StatMeter.h>

namespace Common
{

StatMeter::StatMeter(size_t size_)
    : vals(), size(size_)
{
}

void StatMeter::PushVal(int64_t val)
{
    vals.push_front(val);

    if (vals.size() > size) vals.pop_back();
}

void StatMeter::Clear()
{
    vals.clear();
}

int64_t StatMeter::GetMax() const
{
    int64_t max_ = 0;

    for (auto v : vals)
    {
        if (v > max_)
        {
            max_ = v;
        }
    }

    return max_;
}

int64_t StatMeter::GetAvg() const
{
    int64_t summ_ = 0;

    for (auto v : vals)
    {
        summ_ += v;
    }

    return vals.size() != 0 ? static_cast<int64_t>((double)summ_ / vals.size()) : 0;
}

size_t StatMeter::GetFill() const
{
    return vals.size();
}

}
