/**
 * SpeedTester.xpp - Contains network speed tester impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#include <Transport/HTTP/HttpClient.h>
#include <Common/Common.h>
#include <Common/TimeMeter.h>

#include <NetTester/SpeedTester.h>

namespace NetTester
{

SpeedTester::SpeedTester(std::shared_ptr<wui::i_locale> locale_,
	std::function<void(uint32_t, uint32_t)> readyCallback_,
	std::function<void(std::string_view, int32_t)> progressCallback_)
	: locale(locale_),
	readyCallback(readyCallback_),
	progressCallback(progressCallback_),
	thread(),
	serverAddress(), useHTTPS(false),
	inputSpeed(0), outputSpeed(0),
	runned(false),
	sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{
}

SpeedTester::~SpeedTester()
{
	Stop();
}

void SpeedTester::SetParams(std::string_view serverAddress_, bool useHTTPS_)
{
	serverAddress = serverAddress_;
	useHTTPS = useHTTPS_;
}

void SpeedTester::DoTheTest()
{
	if (!serverAddress.empty())
	{
		Stop();

		runned = true;

		thread = std::thread([this]() {
			if (runned)	TakeInputSpeed();
			if (runned) TakeOutputSpeed();

			if (runned) readyCallback(inputSpeed, outputSpeed);
		});
	}
}

void SpeedTester::Stop()
{
	sysLog->trace("SpeedTester :: Perform ending");
	
	runned = false;
	if (thread.joinable()) thread.join();
	
	sysLog->trace("SpeedTester :: Ended");
}

uint32_t SpeedTester::GetInputSpeed() const
{
	return inputSpeed;
}

uint32_t SpeedTester::GetOutputSpeed() const
{
	return outputSpeed;
}

void SpeedTester::TakeOutputSpeed()
{
	auto baseURL = (useHTTPS ? std::string("https://") : std::string("http://")) + serverAddress;

	progressCallback(locale->get("net_test", "connecting"), 0);

	sysLog->trace("SpeedTester :: TakeOutputSpeed :: Perform connecting to url: {0}", baseURL);

	Transport::HTTPClient httpClient([&](int32_t c, const char *m){
		errLog->error("SpeedTester :: TakeOutputSpeed ERROR(code: {0}, msg: {1})", c, m);
	});
	httpClient.Connect(baseURL);

	sysLog->trace("SpeedTester :: TakeOutputSpeed :: Connection established");

	progressCallback(locale->get("net_test", "connected"), 0);

	const auto SIZE = 1024 * 100;
	const auto COUNT = 5;

	std::string dummy;
	dummy.resize(SIZE);

	double avgSpeed = 0;

	Common::TimeMeter timeMeter;
	for (auto i = 1; i != COUNT + 1; ++i)
	{
		sysLog->trace("SpeedTester :: TakeOutputSpeed :: Perform iteration {0}", i);

		timeMeter.Reset();

		httpClient.Request("/nettest/output", "POST", dummy);
		
		avgSpeed += static_cast<double>((SIZE * 8) / (static_cast<double>(timeMeter.Measure()) / 1000));

		auto currentSpeed = static_cast<uint32_t>(avgSpeed / i);

		sysLog->trace("SpeedTester :: TakeOutputSpeed :: iteration {0} result : {1} kbps", i, currentSpeed);

		progressCallback(locale->get("net_test", "out_speed_testing") + ": " + 
			std::to_string(currentSpeed) + " " + 
			locale->get("net_test", "kbps"),
			i * (100 / COUNT));

		if (!runned)
		{
			sysLog->trace("SpeedTester :: TakeOutputSpeed :: iteration {0} stoped immidiately", i);
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
	
    outputSpeed = static_cast<uint32_t>(avgSpeed / COUNT);

	sysLog->trace("SpeedTester :: TakeOutputSpeed :: result: {0} kbps", outputSpeed);

	httpClient.Disconnect();

	sysLog->trace("SpeedTester :: TakeOutputSpeed :: ended");
}

void SpeedTester::TakeInputSpeed()
{
	auto baseURL = (useHTTPS ? std::string("https://") : std::string("http://")) + serverAddress;

	sysLog->trace("SpeedTester :: TakeInputSpeed :: Perform connecting to url: {0}", baseURL);

	progressCallback(locale->get("net_test", "connecting"), 0);

	Transport::HTTPClient httpClient([&](int32_t c, const char *m){
		errLog->error("SpeedTester :: TakeInputSpeed ERROR(code: {0}, msg: {1})", c, m);
	});
	httpClient.Connect(baseURL);

	sysLog->trace("SpeedTester :: TakeInputSpeed :: Connection established");

	progressCallback(locale->get("net_test", "connected"), 0);

	const auto COUNT = 5;

	std::string dummy;
	
	double avgSpeed = 0;

	Common::TimeMeter timeMeter;
	for (auto i = 1; i != COUNT + 1; ++i)
	{
		sysLog->trace("SpeedTester :: TakeInputSpeed :: Perform iteration {0}", i);

		timeMeter.Reset();

		dummy = httpClient.Request("/nettest/input", "GET");

		avgSpeed += static_cast<double>((dummy.size() * 8) / (static_cast<double>(timeMeter.Measure()) / 1000));

		auto currentSpeed = static_cast<uint32_t>(avgSpeed / i);

		sysLog->trace("SpeedTester :: TakeInputSpeed :: iteration {0} result : {1} kbps", i, currentSpeed);

		progressCallback(locale->get("net_test", "in_speed_testing") + ": " +
			std::to_string(currentSpeed) + " " +
			locale->get("net_test", "kbps"),
			i * (100 / COUNT));

		if (!runned)
		{
			sysLog->trace("SpeedTester :: TakeInputSpeed :: iteration {0} stoped immidiately", i);
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}

	inputSpeed = static_cast<uint32_t>(avgSpeed / COUNT);

	sysLog->trace("SpeedTester :: TakeInputSpeed :: result: {0} kbps", inputSpeed);

	httpClient.Disconnect();

	sysLog->trace("SpeedTester :: TakeInputSpeed :: ended");
}

}
