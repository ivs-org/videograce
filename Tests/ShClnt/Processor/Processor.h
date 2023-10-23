/**
 * Processor.h - Contains header of shell client's processor
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#pragma once

#include <Controller/Controller.h>
#include <Player/Player.h>

#include <thread>
#include <queue>

#include <mt/semaphore.h>

#include "spdlog/spdlog.h"

namespace Processor
{

class Processor
{
	Controller::Controller &controller;
	Player::Player &player;

	std::string address,
		login,
		password,
		conference;

	bool runned;
	bool online;
    bool secured;

	mt::semaphore sem;
	std::mutex queueMutex;
	std::queue<Controller::Event> eventQueue;
	std::thread worker;

	std::shared_ptr<spdlog::logger> sysLog, errLog;
public:
	Processor(Controller::Controller &controller, Player::Player &player);
	virtual ~Processor() {}

	void ReceiveEvent(const Controller::Event &event_);

	void SetParams(const std::string &address,
		const std::string &login,
		const std::string &password,
		const std::string &conference);

	void Start();
	void Stop();

	bool isRunned() const;

private:
	bool GetEventFromQueue(Controller::Event &event_);
	void Process();

	void ConnectToConference();
	void DisconnectFromConference();

	void StartCamera();
	void StopCamera();
	void StartMicrophone();
	void StopMicrophone();
};

}
