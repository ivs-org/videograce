/**
 * Installer.cpp - Contains main()
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#include "MainDialog.h"

#include <Version.h>

#include <Common/FSHelpers.h>

#include <mutex>

#include <boost/nowide/convert.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

#include <iostream>

#ifdef _WIN32
std::string GetLogFileName()
{
    const char* logFileName = "\\" SYSTEM_NAME "Installer.log";

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

std::string GetLogFileName()
{
    return wui::real_path("~/." CLIENT_USER_FOLDER "/" CLIENT_USER_FOLDER ".log");
}
#endif

void CreateLogger()
{
    auto fileName = GetLogFileName();
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

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    CreateLogger();

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    wui::set_default_theme_from_resource("light", TXT_LIGHT_THEME, "JSONS");

	wui::config::use_registry("Software\\IVS\\" SYSTEM_NAME "-Client", HKEY_CURRENT_USER);

    wui::error err;

    wui::set_app_locales({
        { wui::locale_type::eng, "English", "/en_locale.json", TXT_LOCALE_EN },
        { wui::locale_type::rus, "Русский", "/ru_locale.json", TXT_LOCALE_RU },
        { wui::locale_type::kaz, "Қазақ",   "/kk_locale.json", TXT_LOCALE_KK },
        });

    auto current_locale = static_cast<wui::locale_type>(wui::config::get_int("User", "Locale",
        static_cast<int32_t>(wui::get_default_system_locale())));
    wui::set_default_locale(wui::locale_type::rus);
    wui::set_current_app_locale(current_locale);
    wui::set_locale_from_type(current_locale, err);
    if (!err.is_ok())
    {
        //std::cerr << "Can't open locale file {0}" << err.str() << std::endl;
        return -1;
    }

    {
        Installer::MainDialog mainDialog;
        mainDialog.Run();

        // Main message loop:
        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    Gdiplus::GdiplusShutdown(gdiplusToken);

    spdlog::get("System")->info("Application was ended");

    spdlog::drop_all(); // Under VisualStudio, this must be called before main finishes to workaround a known VS issue

    return 0;
}
