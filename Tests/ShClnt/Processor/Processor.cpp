/**
 * Processor.cpp - Contains impl of shell client's processor
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#include <Processor/Processor.h>

namespace Processor
{

Processor::Processor(Controller::Controller &controller_, Player::Player &player_)
	: controller(controller_),
    player(player_),

	address(),
	login(),
	password(),
	conference(),

	runned(false),
	online(false),
    secured(false),

	sem(),
	queueMutex(),
	eventQueue(),
	worker(),

	sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{
}

void Processor::ReceiveEvent(const Controller::Event &e)
{
	std::lock_guard<std::mutex> lock(queueMutex);
	eventQueue.push(e);
	sem.notify();
}

bool Processor::GetEventFromQueue(Controller::Event &event_)
{
	std::lock_guard<std::mutex> lock(queueMutex);
	if (!eventQueue.empty())
	{
		event_ = eventQueue.front();
		eventQueue.pop();
		return true;
	}
	return false;
}

void Processor::Process()
{
	Controller::Event e;
	while (GetEventFromQueue(e))
	{
		switch (e.type)
		{
			/// Connection
			case Controller::Event::Type::UpdateRequired:
				sysLog->info("Update required");
				runned = false;
			break;
			case Controller::Event::Type::LogonSuccess:
			{
				sysLog->info("Logon success, client: {0}", controller.GetMyClientName());

				online = true;

				ConnectToConference();
			}
			break;
			case Controller::Event::Type::Disconnected:
				if (online)
				{
					errLog->info("Disconnected");
					online = false;
				}
				
				//controller.Connect(address, secured);
			break;
            case Controller::Event::Type::ServerChanged:
            {
                errLog->error("Server changed to {0}, secure: {0:d}", e.data, e.iData);

                address = e.data;
                secured = static_cast<bool>(e.iData);
            }
            break;
			case Controller::Event::Type::ServerOutdated:
				errLog->error("Server Outdated");
			break;
			case Controller::Event::Type::AuthNeeded:
			{
				sysLog->info("Auth needed");

				if (e.iData == 0)
				{
					errLog->error("Server URL unspecified");
				}
				else if (e.iData == 1)
				{
					errLog->error("Invalid credentials");
				}
				else if (e.iData == 2)
				{
					errLog->error("Crendentials unspecified");
				}
			}
			break;

			case Controller::Event::Type::ConferenceConnected:
				sysLog->info("Conference connected");

				player.Start();

				StartCamera();
				StartMicrophone();
			break;
			case Controller::Event::Type::DisconnectedFromConference:
			{
				sysLog->info("Disconnected from conference");

				StopCamera();
				StopMicrophone();
			}
			break;
			case Controller::Event::Type::ConferenceNotExists:
				errLog->warn("Conference not exists");
			break;
			case Controller::Event::Type::ConferenceNotAllowed:
				errLog->warn("Conference not allowed");
			break;
			case Controller::Event::Type::ConferenceLicenseFull:
				errLog->warn("Conference not connected, because license is full");
			break;

			case Controller::Event::Type::DoDisconnectFromConference: case Controller::Event::Type::ActionDisconnectFromConference:
				sysLog->info("Do Disconnect from conference");
				DisconnectFromConference();
			break;

			case Controller::Event::Type::CameraCreated:
			{
				sysLog->info("Camera created");

				const Controller::DeviceValues &device = e.deviceValues;

				player.cameraId = device.deviceId;

				controller.ConnectCapturer(device);
			}
			break;
			case Controller::Event::Type::CameraStarted:
			{
				sysLog->info("Camera started");

				const auto &device = e.deviceValues;

				player.SetCameraRTPParams(device.addr.c_str(), device.port);
				player.StartCamera(device.authorSSRC, device.secureKey);
			}
			break;
			case Controller::Event::Type::MicrophoneCreated:
			{
				sysLog->info("Microphone created");

				const Controller::DeviceValues &device = e.deviceValues;

				player.microphoneId = device.deviceId;

				controller.ConnectCapturer(device);
			}
			break;
			case Controller::Event::Type::MicrophoneStarted:
			{
				sysLog->info("Microphone started");

				const auto &device = e.deviceValues;

				player.SetMicrophoneRTPParams(device.addr.c_str(), device.port);
				player.StartMicrophone(device.authorSSRC, device.secureKey);

				//controller.MicrophoneActive(player.microphoneId, Proto::MICROPHONE_ACTIVE::ActiveType::Speak);
			}
			break;

			case Controller::Event::Type::ActionTurnSpeaker:
				controller.SendTurnSpeaker();
			break;
			case Controller::Event::Type::ActionTurnCamera:
				if (player.IsCameraStarted())
				{
					player.StopCamera();
					controller.DeleteCapturer(player.cameraId);
				}
				else
				{
					StartCamera();
				}
			break;
			case Controller::Event::Type::ActionTurnMicrophone:
				if (player.IsMicrophoneStarted())
				{
					player.StopMicrophone();
					controller.DeleteCapturer(player.microphoneId);
				}
				else
				{
					StartMicrophone();
				}
			break;
            case Controller::Event::Type::DeviceConnect:
                //controller.MicrophoneActive(player.microphoneId, Proto::MICROPHONE_ACTIVE::atSpeak);
            break;
		}
	}
}

void Processor::ConnectToConference()
{
	Proto::Conference conf;
	conf.tag = conference;
	controller.ConnectToConference(conf, true, true, false);
}

void Processor::DisconnectFromConference()
{
	player.Stop();

	std::vector<Controller::DeviceValues> capturers;
	
	Controller::DeviceValues capturer;
	capturer.deviceId = player.cameraId;
	capturers.emplace_back(capturer);
	capturer.deviceId = player.microphoneId;
	capturers.emplace_back(capturer);
		
	std::vector<Controller::DeviceValues> renderers;
	
	controller.DisconnectFromConference(capturers, renderers);

	runned = false;
}

void Processor::StartCamera()
{
	const Controller::State currentState = controller.GetState();
	if (currentState == Controller::State::Conferencing)
	{
		/// Add new camera
		Controller::DeviceValues device;
		device.name = "Fake camera";
		device.type = Proto::DeviceType::Camera;
		device.resolution = player.GetResolution();
		controller.CreateCapturer(device);
	}
}

void Processor::StopCamera()
{
	controller.DeleteCapturer(player.cameraId);
}

void Processor::StartMicrophone()
{
	const Controller::State currentState = controller.GetState();
	if (currentState == Controller::State::Conferencing)
	{
		/// Add new camera
		Controller::DeviceValues device;
		device.name = "Fake microphone";
		device.type = Proto::DeviceType::Microphone;
		controller.CreateCapturer(device);
	}
}

void Processor::StopMicrophone()
{
	controller.DeleteCapturer(player.microphoneId);
}

void Processor::SetParams(const std::string &address_,
	const std::string &login_,
	const std::string &password_,
	const std::string &conference_)
{
	address = address_;
	login = login_;
	password = password_;
	conference = conference_;
}

void Processor::Start()
{
	if (runned)
	{
		return;
	}

	runned = true;
	worker = std::thread([this]() {
		while (runned)
		{
			sem.wait();
			Process();
		}
	});

	controller.SetCredentials(login, password);
	controller.Connect(address, secured);
}

void Processor::Stop()
{
	DisconnectFromConference();

	runned = false;
	{
		std::lock_guard<std::mutex> lock(queueMutex);
		eventQueue.push(Controller::Event());
	}
	sem.notify();

	if (worker.joinable())
	{
		worker.join();
	}
}

bool Processor::isRunned() const
{
	return runned;
}

}
