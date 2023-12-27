/**
 * SpeedTester.xpp - Contains network speed tester impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016, 2023
 */

#include <NetTester/SpeedTester.h>

#include <Proto/CommandType.h>
#include <Proto/CmdConnectRequest.h>
#include <Proto/CmdConnectResponse.h>
#include <Proto/CmdLoadBlobs.h>
#include <Proto/CmdDeliveryBlobs.h>
#include <Proto/CmdPing.h>
#include <Proto/CmdDisconnect.h>

#include <Version.h>

namespace NetTester
{

SpeedTester::SpeedTester(std::shared_ptr<wui::i_locale> locale_,
	std::function<void(uint32_t, uint32_t)> readyCallback_,
	std::function<void(std::string_view, int32_t)> progressCallback_)
	: locale(locale_),
	readyCallback(readyCallback_),
	progressCallback(progressCallback_),
	timeMeter(),
	webSocket(std::bind(&SpeedTester::OnWebSocket, this, std::placeholders::_1, std::placeholders::_2)),
	thread(),
	serverAddress(), useHTTPS(false),
	login(), passwd(),
	inputSpeed(0), outputSpeed(0),
	mode_(mode::input),
	iteration(0),
	runned(false),
	sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{
}

SpeedTester::~SpeedTester()
{
	Stop();
}

void SpeedTester::SetParams(std::string_view serverAddress_, bool useHTTPS_, std::string_view login_, std::string_view passwd_)
{
	serverAddress = serverAddress_;
	useHTTPS = useHTTPS_;
	login = login_;
	passwd = passwd_;
}

void SpeedTester::DoTheTest()
{
	if (!serverAddress.empty())
	{
		Stop();

		runned = true;

		thread = std::thread([this]() {
			Connect();

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
	/*auto baseURL = (useHTTPS ? std::string("https://") : std::string("http://")) + serverAddress;

	progressCallback(locale->get("net_test", "connecting"), 0);
	std::this_thread::yield();

	sysLog->trace("SpeedTester :: TakeOutputSpeed :: Perform connecting to url: {0}", baseURL);

	Transport::HTTPClient httpClient([&](int32_t c, const char *m){
		errLog->error("SpeedTester :: TakeOutputSpeed ERROR(code: {0}, msg: {1})", c, m);
	});
	httpClient.Connect(baseURL);

	sysLog->trace("SpeedTester :: TakeOutputSpeed :: Connection established");

	progressCallback(locale->get("net_test", "connected"), 0);
	std::this_thread::yield();

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

		std::this_thread::yield();
	}
	
    outputSpeed = static_cast<uint32_t>(avgSpeed / COUNT);

	sysLog->trace("SpeedTester :: TakeOutputSpeed :: result: {0} kbps", outputSpeed);

	httpClient.Disconnect();

	sysLog->trace("SpeedTester :: TakeOutputSpeed :: ended");*/
}

void SpeedTester::TakeInputSpeed()
{
	mode_ = mode::input;
	
	if (webSocket.IsConnected())
	{
		webSocket.Send(Proto::LOAD_BLOBS::Command(std::vector<std::string_view>{ 
			"00000000-0000-0000-0000-000000000001" }).Serialize()); /// Service speed test blob guid
	}

	/*	avgSpeed += static_cast<double>((dummy.size() * 8) / (static_cast<double>(timeMeter.Measure()) / 1000));

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

		std::this_thread::yield();
	}

	inputSpeed = static_cast<uint32_t>(avgSpeed / COUNT);

	sysLog->trace("SpeedTester :: TakeInputSpeed :: result: {0} kbps", inputSpeed);

	httpClient.Disconnect();

	sysLog->trace("SpeedTester :: TakeInputSpeed :: ended");*/
}

void SpeedTester::Connect()
{
	progressCallback(locale->get("net_test", "connecting"), 0);
	webSocket.Connect((useHTTPS ? std::string("https://") : std::string("http://")) + serverAddress);
}

void SpeedTester::Logon()
{
	if (webSocket.IsConnected())
	{
		Proto::CONNECT_REQUEST::Command cmd;
		cmd.client_version = CLIENT_VERSION;
#ifdef WIN32
		cmd.system = "Windows";
#else
#ifdef __APPLE__
		cmd.system = "MacOS";
#else
		cmd.system = "Linux";
#endif
#endif
		cmd.login = login;
		cmd.password = passwd;

		webSocket.Send(cmd.Serialize());
	}
}

void SpeedTester::OnWebSocket(Transport::WSMethod method, std::string_view message)
{
	switch (method)
    {
        case Transport::WSMethod::Open:
            sysLog->info("SpeedTester :: Connection to server established");
			
			progressCallback(locale->get("net_test", "connected"), 0);
			std::this_thread::yield();

			Logon();
        break;
        case Transport::WSMethod::Message:
		{
			auto commandType = Proto::GetCommandType(message);
            switch (commandType)
            {
                case Proto::CommandType::ConnectResponse:
                {
                    Proto::CONNECT_RESPONSE::Command cmd;
                    cmd.Parse(message);

                    if (cmd.result == Proto::CONNECT_RESPONSE::Result::OK)
                    {
						timeMeter.Reset();
						webSocket.Send(Proto::LOAD_BLOBS::Command(std::vector<std::string_view>{
							"00000000-0000-0000-0000-000000000001" }).Serialize()); /// Service speed test blob guid
                    }
                }
                break;
				case Proto::CommandType::DeliveryBlobs:
				break;
			}
		}
		break;
		case Transport::WSMethod::Close:
			sysLog->info("SpeedTester :: WebSocket closed (message: \"{0}\")", message);
		break;
		case Transport::WSMethod::Error:
			errLog->critical("SpeedTester :: WebSocket error (message: \"{0}\")", message);
		break;
	}
}

}
