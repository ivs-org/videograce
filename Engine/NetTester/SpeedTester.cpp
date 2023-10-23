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

SpeedTester::SpeedTester(std::function<void(uint32_t, uint32_t)> readyCallback_)
	: readyCallback(readyCallback_),
	thread(),
	serverAddress(), useHTTPS(false),
	inputSpeed(0), outputSpeed(0)
{
}

SpeedTester::~SpeedTester()
{
	if (thread.joinable()) thread.join();
}

void SpeedTester::SetParams(const std::string serverAddress_, bool useHTTPS_)
{
	serverAddress = serverAddress_;
	useHTTPS = useHTTPS_;
}

void SpeedTester::DoTheTest()
{
	if (!serverAddress.empty())
	{
		if (thread.joinable()) thread.join();

		thread = std::thread([this]() {
			TakeInputSpeed();
			TakeOutputSpeed();

            readyCallback(inputSpeed, outputSpeed);
		});
	}
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

	Transport::HTTPClient httpClient;
	httpClient.Connect(baseURL);

	enum { SIZE = 1024 * 100 };
	std::string dummy;
	dummy.resize(SIZE);

	Common::TimeMeter timeMeter;
	timeMeter.Reset();

	httpClient.Request("/nettest/output", "POST", dummy);

	const double recvTime = (double)timeMeter.Measure() / 1000;
	double speed = static_cast<double>(SIZE) * 8 / recvTime;
	
    outputSpeed = static_cast<uint32_t>(speed);

	httpClient.Disconnect();
}

void SpeedTester::TakeInputSpeed()
{
	auto baseURL = (useHTTPS ? std::string("https://") : std::string("http://")) + serverAddress;

	Transport::HTTPClient httpClient;
	httpClient.Connect(baseURL);

	Common::TimeMeter timeMeter;
	timeMeter.Reset();

	auto dummy = httpClient.Request("/nettest/input", "GET");

	const double recvTime = (double)timeMeter.Measure() / 1000;
	const double size = static_cast<double>(dummy.size());
	double speed = size * 8 / recvTime;
	inputSpeed = static_cast<uint32_t>(speed);

	httpClient.Disconnect();
}

}
