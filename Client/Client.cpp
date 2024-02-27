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
#include <wui/config/config_impl_reg.hpp>

#ifdef _WIN32
#include <windows.h>

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
#include <Common/Logger.h>
#include <Common/FSHelpers.h>

#include <iostream>

#include <spdlog/spdlog.h>

#include <UI/MainFrame.h>

#include <Version.h>

#include <resource.h>

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

void RegisterVGProtocol()
{
    wchar_t pathW[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, pathW, MAX_PATH);
    std::string path(boost::nowide::narrow(pathW));

    wui::config::config_impl_reg cir("SOFTWARE\\Classes\\" BROWSER_PROTO, HKEY_CURRENT_USER);

    cir.set_string("", "", "URL:" SYSTEM_NAME " Conferencing");
    cir.set_string("", "URL Protocol", "");

    cir.set_string("DefaultIcon", "", Common::FileNameOf(path) + ",1");

    wui::config::config_impl_reg cir1("SOFTWARE\\Classes\\" BROWSER_PROTO "\\shell\\open", HKEY_CURRENT_USER);
    cir1.set_string("command", "", "\"" + path + "\" %1");
}

void CheckVGProtocol()
{
    wchar_t pathW[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, pathW, MAX_PATH);
    std::string currentPath(boost::nowide::narrow(pathW));

    wui::config::config_impl_reg cir("SOFTWARE\\Classes\\" BROWSER_PROTO "\\shell\\open", HKEY_CURRENT_USER);

    if (cir.get_string("command", "", "") != "\"" + currentPath + "\" %1")
    {
        RegisterVGProtocol();
    }
}

void SetMinTimerResolution()
{
    const auto TARGET_RESOLUTION = 1; // 1 - millisecond target resolution

    TIMECAPS tc;

    auto getCapsResult = timeGetDevCaps(&tc, sizeof(TIMECAPS));
    if (getCapsResult != TIMERR_NOERROR)
    {
        return spdlog::get("Error")->critical("SetMinTimerResolution :: timeGetDevCaps error: {0}", getCapsResult);
    }

    spdlog::get("System")->debug("SetMinTimerResolution :: Resolutions: (min: {0}, max: {1})", tc.wPeriodMin, tc.wPeriodMax);

    UINT wTimerRes = min(max(tc.wPeriodMin, TARGET_RESOLUTION), tc.wPeriodMax);
    auto beginPeriodResult = timeBeginPeriod(wTimerRes);
    if (getCapsResult != TIMERR_NOERROR)
    {
        spdlog::get("Error")->critical("SetMinTimerResolution :: timeBeginPeriod error: {0}", beginPeriodResult);
    }

    spdlog::get("System")->info("SetMinTimerResolution :: Set timer resolution to: {0}", wTimerRes);
}

#elif __linux__

std::string exec(std::string_view cmd)
{
    std::array<char, 4096> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.data(), "r"), pclose);
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    std::cout << result << std::endl;
    return result;
}

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

void ConnectToConference(std::string_view cmdLine)
{
    std::vector<std::string> strs;
    boost::split(strs, cmdLine, boost::is_any_of("/"));
    if (strs.size() == 4)
    {
        wui::config::set_string("User", "ConferenceTag", strs[3]);
    }
}

void CheckVGProtocol()
{
}

void RegisterVGProtocol()
{
}

void SetMinTimerResolution()
{
    // todo!!
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

    if (std::wstring(lpCmdLine).find(_T(BROWSER_PROTO) L"://conference") != std::wstring::npos) // finded like: vg://conference/tag
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
        auto strPids = exec("pgrep -f " SYSTEM_NAME "Client");
        std::vector<std::string> pids;
        boost::split(pids, strPids, boost::is_any_of("\n"));
        if (!strPids.empty() && pids.size() > 3)
        {
            return 0;
        }
        else
        {
            boost::interprocess::shared_memory_object::remove(SYSTEM_NAME "ClientRunned");
        }
    }

    if (setlocale(LC_ALL, "") == NULL)
    {
        std::cerr << "warning: could not set default locale" << std::endl;
    }

    bool autorun = argc > 1 ? (std::string(argv[1]).find("/autorun") != std::string::npos) : false;

    auto resoursesPath = wui::config::get_string("Application", "Resources", wui::real_path("~/." CLIENT_USER_FOLDER "/res"));
#endif

    wui::framework::init();

    Common::CreateLogger(
#ifdef _WIN32
        Common::GetLogFileName("Client")
#else
        Common::GetLogFileName(CLIENT_USER_FOLDER)
#endif
    );

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

    SetMinTimerResolution();
    Transport::NetworkInit networkInit;

    CheckVGProtocol();

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

    spdlog::get("System")->info("Application was ended");

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
