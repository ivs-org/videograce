/**
 * SpeedTester.h - Contains network speed tester header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#pragma once

#include <atomic>
#include <cstdint>
#include <string>
#include <thread>
#include <functional>

namespace NetTester
{

class SpeedTester
{
public:
	SpeedTester(std::function<void(uint32_t, uint32_t)> readyCallback);
	~SpeedTester();

	void SetParams(const std::string serverAddress, bool useHTTPS);

	void DoTheTest();

	uint32_t GetInputSpeed() const;
	uint32_t GetOutputSpeed() const;

private:
    std::function<void(uint32_t, uint32_t)> readyCallback;

	std::thread thread;

	std::string serverAddress;
	bool useHTTPS;
	uint32_t inputSpeed, outputSpeed;
	
	void TakeInputSpeed();
	void TakeOutputSpeed();
};

}
