/**
 * ShClnt.cpp - Contains entry point to shell client
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#include <iostream>

#include <Controller/Controller.h>
#include <Processor/Processor.h>
#include <Processor/MemberList.h>
#include <Player/Player.h>

#include <Storage/Storage.h>

#include <Common/FSHelpers.h>

#include <wui/config/config.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <boost/nowide/convert.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio.hpp>

void CreateLogger(std::string_view login)
{
#ifndef _WIN32
	const std::string fileName = "./ShClnt-" + std::string(login) + ".log";
#else
	wchar_t moduleFileName[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, moduleFileName, MAX_PATH);
	const std::string fileName = Common::DirNameOf(boost::nowide::narrow(moduleFileName)) + "\\ShClnt-" + std::string(login) + ".log";
#endif

    try
    {
        std::vector<spdlog::sink_ptr> sinks;

        sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
#ifdef _WIN32
            boost::nowide::widen(fileName),
#else
            fileName,
#endif
            1024 * 1024 * 5, 3));
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

        auto sysLog = std::make_shared<spdlog::logger>("System", begin(sinks), end(sinks));
        auto errLog = std::make_shared<spdlog::logger>("Error", begin(sinks), end(sinks));

        int logLevel = wui::config::get_int("User", "LogLevel", 0);

        sysLog->set_level(static_cast<spdlog::level::level_enum>(logLevel));
        errLog->set_level(static_cast<spdlog::level::level_enum>(logLevel));

        // globally register the loggers so so the can be accessed using spdlog::get(logger_name)
        spdlog::register_logger(sysLog);
        spdlog::register_logger(errLog);
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        std::cerr << "Log initialization failed: " << ex.what() << std::endl;
    }
}

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

	CreateLogger(login);

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
	
	return 0;
}
