/**
 * ShClnt.cpp - Contains entry point to shell client
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#include <Controller/Controller.h>
#include <Processor/Processor.h>
#include <Processor/MemberList.h>
#include <Player/Player.h>

#include <Storage/Storage.h>

#include <Common/Logger.h>

#include <wui/config/config.hpp>

#include <spdlog/spdlog.h>

#include <boost/asio/signal_set.hpp>
#include <boost/asio.hpp>

#include <iostream>

int main(int argc, char* argv[])
{
#ifndef _DEBUG
	if (argc < 5)
	{
		std::cout << "Usage: ShClnt server address login password conference file name" << std::endl;
		return 0;
	}

	std::string address = argv[1];
	std::string login = argv[2];
	std::string password = argv[3];
	std::string conference = argv[4];
	std::string fileName = argv[5];
#else
	std::string address = "::1:8778",
		login = "test1",
		password = "1",
		conference = "default",
		fileName = "d:\\play.mkv";

#endif

	Common::CreateLogger("ShClnt-" + login);

    Storage::Storage storage;
    Processor::MemberList memberList;
    Controller::Controller controller(storage, memberList);

    Player::Player player;
    player.SetFileName(fileName);

	Processor::Processor processor(controller, player);
	processor.SetParams(address, login, password, conference);

    controller.SetEventHandler(std::bind(&Processor::Processor::ReceiveEvent, &processor, std::placeholders::_1));

	processor.Start();

	bool end = false;

	// Capture SIGINT and SIGTERM to perform a clean shutdown
	boost::asio::io_service io;

	boost::asio::signal_set signals(io, SIGINT, SIGTERM);
	signals.async_wait([&](boost::system::error_code const&, int)
	{
		spdlog::get("System")->info("SIGINT received, finishing the application's work");
		end = true;
	});

	std::thread t([&](){ io.run(); });

	while (processor.isRunned() && !end)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	processor.Stop();

	io.stop();
	t.join();

	spdlog::get("System")->info("Application was ended");
	spdlog::drop_all(); // Under VisualStudio, this must be called before main finishes to workaround a known VS issue
	
	return 0;
}
