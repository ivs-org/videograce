/**
 * VideoGraceClient.cpp - Contains input point of application
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014, 2015, 2022
 */

#include <wui/framework/framework.hpp>

#include <wui/theme/theme.hpp>
#include <wui/theme/theme_selector.hpp>

#include <wui/locale/locale.hpp>
#include <wui/locale/locale_selector.hpp>

#include <wui/system/tools.hpp>
#include <wui/system/path_tools.hpp>
#include <wui/config/config.hpp>

#ifdef _WIN32
#include <windows.h>
#include <GdiPlus.h>

#include <dbt.h>
#include <Shobjidl.h>
#else
#include <pwd.h>
#include <sys/types.h>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#endif

#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/nowide/convert.hpp>

#include <Transport/NetworkInit.h>

#include <Common/FSHelpers.h>

#include <iostream>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

#include <UI/MainFrame.h>

#include <Version.h>

#include <resource.h>

#ifdef _WIN32
std::string GetLogFileName()
{
    const char *logFileName = "\\" SYSTEM_NAME "Client.log";

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

#ifdef _WIN32

static NTSTATUS(__stdcall *NtSetTimerResolution)(IN ULONG RequestedResolution, IN BOOLEAN Set, OUT PULONG ActualResolution) = (NTSTATUS(__stdcall*)(ULONG, BOOLEAN, PULONG)) GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtSetTimerResolution");

HANDLE oneRunMutex = 0;

bool IsAlreadyRunning(bool waitForCloseOnRestarting)
{
    for (;;)
    {
        oneRunMutex = CreateMutex(
            NULL,
            FALSE,
            _T(SYSTEM_NAME) L"ClientRunned");

            if (ERROR_ALREADY_EXISTS == GetLastError())
            {
                if (!waitForCloseOnRestarting)
                {
                    HANDLE h = OpenEvent(EVENT_MODIFY_STATE, FALSE, _T(SYSTEM_NAME) L"RunEvent");
                    SetEvent(h);

                    CloseHandle(oneRunMutex);

                    return true;
                }
                else
                {
                    CloseHandle(oneRunMutex);
                    ReleaseMutex(oneRunMutex);
                    Sleep(200);
                }
            }
            else
            {
                break;
            }
    }

    return false;
}

void ConnectToConference(const std::wstring &cmdLine)
{
    std::vector<std::wstring> strs;
    boost::split(strs, cmdLine, boost::is_any_of(L"/"));
    if (strs.size() == 4)
    {
        wui::config::set_string("User", "ConferenceTag", boost::nowide::narrow(strs[3]));
    }
}

#elif __linux__

bool IsAlreadyRunning(bool waitForCloseOnRestarting)
{
	for (;;)
    {
        try
        {
        	boost::interprocess::shared_memory_object smo(boost::interprocess::create_only, SYSTEM_NAME "ClientRunned", boost::interprocess::read_write);
            smo.truncate(1);
        	break;
        }
        catch(...)
        {
        	// executable is already running
            if (!waitForCloseOnRestarting)
            {
                boost::interprocess::shared_memory_object smo(boost::interprocess::open_only, SYSTEM_NAME "ClientRunned", boost::interprocess::read_write);
            	boost::interprocess::mapped_region region(smo, boost::interprocess::read_write);
                std::memset(region.get_address(), 1, region.get_size());

                std::cerr << "Warning: App is already runned. If the application did not quit correctly before that, run " << SYSTEM_NAME << "Client /reset" << std::endl;
                return true;
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        }
    }
    return false;
}

void ConnectToConference(const std::string &cmdLine)
{
    std::vector<std::string> strs;
    boost::split(strs, cmdLine, boost::is_any_of("/"));
    if (strs.size() == 4)
    {
        wui::config::set_string("User", "ConferenceTag", strs[3]);
    }
}

#endif

#ifdef _WIN32
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    wui::config::use_registry("Software\\IVS\\" SYSTEM_NAME "-Client", HKEY_CURRENT_USER);

    if (std::wstring(lpCmdLine).find(L"/regproto") != std::wstring::npos) // finded like: /regproto
    {
        Client::MainFrame::RegisterVGProtocol();
        return 0;
    }
    else if (std::wstring(lpCmdLine).find(_T(BROWSER_PROTO) L"://conference") != std::wstring::npos) // finded like: vg://conference/tag
    {
        ConnectToConference(lpCmdLine);
    }

    bool autorun = std::wstring(lpCmdLine).find(L"/autorun") != std::wstring::npos;

    if (IsAlreadyRunning(std::wstring(lpCmdLine).find(L"/restart") != std::wstring::npos))
    {
        return 0;
    }

    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);

    ULONG actualResolution;
    NtSetTimerResolution(1, true, &actualResolution);

    std::string resoursesPath;
#else
int main(int argc, char *argv[])
{
	auto ok = wui::config::use_ini_file("~/." CLIENT_USER_FOLDER "/" CLIENT_USER_FOLDER ".conf");
    if (!ok)
    {
        std::cerr << wui::config::get_error().str() << std::endl;
        return -1;
    }

    if (argc > 1 && std::string(argv[1]).find(BROWSER_PROTO "://conference") != std::string::npos) // finded like: vg://conference/tag
    {
        ConnectToConference(argv[1]);
    }

    if (argc > 1 && std::string(argv[1]).find("/reset") != std::string::npos)
    {
    	boost::interprocess::shared_memory_object::remove(SYSTEM_NAME "ClientRunned");
    }

    if (IsAlreadyRunning(argc > 1 ? std::string(argv[1]).find("/restart") != std::string::npos : false))
    {
        return 0;
    }

    if (setlocale(LC_ALL, "") == NULL)
    {
        std::cerr << "warning: could not set default locale" << std::endl;
    }

    bool autorun = argc > 1 ? (std::string(argv[1]).find("/autorun") != std::string::npos) : false;

    auto resoursesPath = wui::config::get_string("Application", "Resources", wui::real_path("~/." CLIENT_USER_FOLDER "/res"));
#endif

    wui::framework::init();

    CreateLogger();

    wui::error err;

    wui::set_app_locales({
        { wui::locale_type::eng, "English", resoursesPath + "/en_locale.json", TXT_LOCALE_EN },
        { wui::locale_type::rus, "Русский", resoursesPath + "/ru_locale.json", TXT_LOCALE_RU },
        { wui::locale_type::kaz, "Қазақ",   resoursesPath + "/kk_locale.json", TXT_LOCALE_KK },
    });

    auto current_locale = static_cast<wui::locale_type>(wui::config::get_int("User", "Locale", 
        static_cast<int32_t>(wui::get_default_system_locale())));
    wui::set_default_locale(wui::locale_type::rus);
    wui::set_current_app_locale(current_locale);
    wui::set_locale_from_type(current_locale, err);
    if (!err.is_ok())
    {
        spdlog::get("Error")->critical("Can't open locale file {0}", err.str());
#ifndef _WIN32
        boost::interprocess::shared_memory_object::remove(SYSTEM_NAME "ClientRunned");
#endif
        return -1;
    }

    wui::set_app_themes({
        { "dark",  resoursesPath + "/dark.json",  TXT_DARK_THEME  },
        { "light", resoursesPath + "/light.json", TXT_LIGHT_THEME }
    });

    auto current_theme = wui::config::get_string("User", "Theme", "dark");
    wui::set_current_app_theme(current_theme);
    wui::set_default_theme_from_name(current_theme, err);
    if (!err.is_ok())
    {
        spdlog::get("Error")->critical("Can't open theme file {0}", err.str());
#ifndef _WIN32
        boost::interprocess::shared_memory_object::remove(SYSTEM_NAME "ClientRunned");
#endif
        return -1;
    }

    Transport::NetworkInit networkInit;

    Client::MainFrame mainFrame;
    mainFrame.Run(autorun);

#ifdef _WIN32
    DEV_BROADCAST_DEVICEINTERFACE notificationFilter;
    ZeroMemory(&notificationFilter, sizeof(notificationFilter));
    notificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    notificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

    HDEVNOTIFY hDeviceNotify = RegisterDeviceNotification(
        mainFrame.context().hwnd,
        &notificationFilter,
        DEVICE_NOTIFY_WINDOW_HANDLE | DEVICE_NOTIFY_ALL_INTERFACE_CLASSES
    );

    wui::framework::run();

    if (hDeviceNotify)
    {
        UnregisterDeviceNotification(hDeviceNotify);
    }

    spdlog::drop_all(); // Under VisualStudio, this must be called before main finishes to workaround a known VS issue

    if (oneRunMutex)
    {
        ReleaseMutex(oneRunMutex);
    }

#elif __linux__
    
    wui::framework::run();

    boost::interprocess::shared_memory_object::remove(SYSTEM_NAME "ClientRunned");
#endif

    return 0;
}
