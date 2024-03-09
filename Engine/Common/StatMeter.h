/**
 * StatMeter.h - Contains the statistics meter header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2024
 */

#pragma once

#include <deque>

namespace Common
{

class StatMeter
{
public:
	StatMeter(size_t size);

	void PushVal(int64_t val);

	void Clear();

	int64_t GetMax() const;
	int64_t GetAvg() const;

	size_t GetFill() const;
private:
	std::deque<int64_t> vals;
	size_t size;
};

}
