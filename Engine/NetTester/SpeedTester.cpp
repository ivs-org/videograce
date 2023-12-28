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
	return static_cast<uint32_t>(inputSpeed);
}

uint32_t SpeedTester::GetOutputSpeed() const
{
	return static_cast<uint32_t>(outputSpeed);
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

void SpeedTester::Logout()
{
	sysLog->info("SpeedTester :: Logout");

	if (webSocket.IsConnected())
	{
		webSocket.Send(Proto::DISCONNECT::Command().Serialize());
	}

	webSocket.Disconnect();

	sysLog->info("SpeedTester :: Logout finisned");
}

void SpeedTester::RequestInputBlob()
{
	timeMeter.Reset();

	webSocket.Send(Proto::LOAD_BLOBS::Command(std::vector<std::string>{
		"00000000-0000-0000-0000-000000000001" }).Serialize()); /// Service speed test blob guid
}

void SpeedTester::SendOutputBlob()
{
	const auto SIZE = 1024 * 100;
	std::string dummy;
	dummy.resize(SIZE);

	timeMeter.Reset();
	webSocket.Send(Proto::DELIVERY_BLOBS::Command(std::vector<Proto::Blob>{
		Proto::Blob(-1,
			-1,
			"00000000-0000-0000-0000-000000000001",
			Proto::BlobAction::SpeedTest,
			Proto::BlobStatus::Undefined,
			dummy,
			"",
			"") }).Serialize());
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
						sysLog->trace("SpeedTester :: Connected");

						mode_ = mode::input;
						inputSpeed = 0;
						outputSpeed = 0;
						iteration = 0;

						RequestInputBlob();
                    }
                }
                break;
				case Proto::CommandType::DeliveryBlobs:
					switch (mode_)
					{
						case mode::input:
						{
							inputSpeed += (static_cast<double>(message.size() * 8) / (static_cast<double>(timeMeter.Measure()) / 1000));

							++iteration;

							auto currentSpeed = static_cast<uint32_t>(inputSpeed / iteration);

							sysLog->trace("SpeedTester :: TakeInputSpeed :: iteration {0} result : {1} kbps", iteration, currentSpeed);

							progressCallback(locale->get("net_test", "in_speed_testing") + ": " +
								std::to_string(currentSpeed) + " " +
								locale->get("net_test", "kbps"),
								iteration * (100 / ITERATIONS_COUNT));

							if (iteration != ITERATIONS_COUNT)
							{
								RequestInputBlob();
							}
							else
							{
								sysLog->trace("SpeedTester :: TakeInputSpeed :: result: {0} kbps", inputSpeed);

								mode_ = mode::output;
								outputSpeed = 0;
								iteration = 0;

								SendOutputBlob();
							}
						}
						break;
						case mode::output:
						{
							outputSpeed += (static_cast<double>(message.size() * 8) / (static_cast<double>(timeMeter.Measure()) / 1000));

							++iteration;

							auto currentSpeed = static_cast<uint32_t>(outputSpeed / iteration);

							sysLog->trace("SpeedTester :: TakeOutputSpeed :: iteration {0} result : {1} kbps", iteration, currentSpeed);

							progressCallback(locale->get("net_test", "out_speed_testing") + ": " +
								std::to_string(currentSpeed) + " " +
								locale->get("net_test", "kbps"),
								iteration * (100 / ITERATIONS_COUNT));

							if (iteration != ITERATIONS_COUNT)
							{
								SendOutputBlob();
							}
							else
							{
								sysLog->trace("SpeedTester :: TakeOutputSpeed :: result: {0} kbps", outputSpeed);
								
								std::thread([this]() { Logout(); }).detach();
							}
						}
						break;
					}
				break;
				case Proto::CommandType::Ping:
					webSocket.Send(Proto::PING::Command().Serialize());
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
