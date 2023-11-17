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
#include <string_view>
#include <functional>

#include <wui/locale/i_locale.hpp>

namespace NetTester
{

class SpeedTester
{
public:
	SpeedTester(std::shared_ptr<wui::i_locale> locale,
		std::function<void(uint32_t, uint32_t)> readyCallback,
		std::function<void(std::string_view, int32_t) > progressCallback);
	~SpeedTester();

	void SetParams(std::string_view serverAddress, bool useHTTPS);

	void DoTheTest();

	uint32_t GetInputSpeed() const;
	uint32_t GetOutputSpeed() const;

private:
	std::shared_ptr<wui::i_locale> locale;
    std::function<void(uint32_t, uint32_t)> readyCallback;
	std::function<void(std::string_view, int32_t)> progressCallback;

	std::thread thread;

	std::string serverAddress;
	bool useHTTPS;
	uint32_t inputSpeed, outputSpeed;

	std::atomic<bool> runned;
	
	void TakeInputSpeed();
	void TakeOutputSpeed();
};

}
