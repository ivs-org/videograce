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

#include <Common/TimeMeter.h>
#include <Transport/WebSocket/WebSocket.h>

#include <spdlog/spdlog.h>

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
	void Stop();

	uint32_t GetInputSpeed() const;
	uint32_t GetOutputSpeed() const;

private:
	std::shared_ptr<wui::i_locale> locale;
    std::function<void(uint32_t, uint32_t)> readyCallback;
	std::function<void(std::string_view, int32_t)> progressCallback;

	Common::TimeMeter timeMeter;

	Transport::WebSocket webSocket;

	std::thread thread;

	std::string serverAddress;
	bool useHTTPS;
	
	double inputSpeed, outputSpeed;

	enum class mode
	{
		input, output
	} mode_;
	
	const int32_t ITERATIONS_COUNT = 5;
	int32_t iteration;

	std::atomic_bool runned;

	std::shared_ptr<spdlog::logger> sysLog, errLog;
	
	void TakeInputSpeed();
	void TakeOutputSpeed();

	void Connect();

	void OnWebSocket(Transport::WSMethod method, std::string_view message);
};

}
