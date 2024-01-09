/**
 * Logger.cpp - Contains common logger's initiaion methods impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2024
 */

#include <Common/Logger.h>
#include <Version.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

#include <boost/nowide/convert.hpp>

#include <wui/system/tools.hpp>
#include <wui/system/path_tools.hpp>
#include <wui/config/config.hpp>

#include <Common/FSHelpers.h>

#include <iostream>

namespace Common
{

#ifdef _WIN32
std::string GetLogFileName(std::string_view appName)
{
    std::string logFileName = "\\" SYSTEM_NAME + std::string(appName) + ".log";

    wchar_t pathBuf[MAX_PATH] = { 0 };

    GetModuleFileName(NULL, pathBuf, MAX_PATH);
    auto path = Common::DirNameOf(boost::nowide::narrow(pathBuf)) + "\\";

    if (Common::CheckAllowFileWrite(path))
    {
        return path + logFileName;
    }
    else
    {
        GetTempPath(MAX_PATH, pathBuf);
        if (Common::CheckAllowFileWrite(boost::nowide::narrow(pathBuf)))
        {
            return boost::nowide::narrow(pathBuf) + logFileName;
        }
    }
    return "";
}
#else

std::string GetLogFileName(std::string_view appName)
{
    return wui::real_path("~/." + std::string(appName) + "/" + std::string(appName) + ".log");
}
#endif

void CreateLogger(std::string_view fileName)
{
    if (fileName.empty())
    {
        std::cerr << "Error creating logger, log path is unreachable" << std::endl;
        return;
    }

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
#ifdef _WIN32
        sinks.push_back(std::make_shared<spdlog::sinks::msvc_sink_mt>());
#endif

        auto sysLog = std::make_shared<spdlog::logger>("System", begin(sinks), end(sinks));
        auto errLog = std::make_shared<spdlog::logger>("Error", begin(sinks), end(sinks));

        int logLevel = wui::config::get_int("User", "LogLevel", 5);

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

}
