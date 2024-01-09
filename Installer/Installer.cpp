/**
 * Installer.cpp - Contains main()
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#include "MainDialog.h"

#include <wui/framework/framework.hpp>

#include <Version.h>
#include <Common/Logger.h>

#include <spdlog/spdlog.h>

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    Common::CreateLogger(Common::GetLogFileName("Installer"));
    
    wui::framework::init();

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

    Installer::MainDialog mainDialog;
    mainDialog.Run();

    wui::framework::run();

    spdlog::get("System")->info("Application was ended");
    spdlog::drop_all(); // Under VisualStudio, this must be called before main finishes to workaround a known VS issue

    return 0;
}
