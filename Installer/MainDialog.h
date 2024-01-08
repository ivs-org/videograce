/**
 * MainDialog.h - Contains main dialog header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#pragma once

#include <wui/theme/theme.hpp>
#include <wui/theme/theme_selector.hpp>
#include <wui/locale/locale.hpp>
#include <wui/locale/locale_selector.hpp>
#include <wui/window/window.hpp>
#include <wui/control/button.hpp>
#include <wui/control/input.hpp>
#include <wui/control/image.hpp>
#include <wui/control/text.hpp>
#include <wui/control/message.hpp>
#include <wui/control/progress.hpp>
#include <wui/control/panel.hpp>

#include "resource.h"

#include <wui/config/config.hpp>
#include <Common/Base64.h>
#include <Common/Process.h>

#include <Transport/HTTP/HttpClient.h>
#include <Transport/URI/URI.h>

#include <mt/timer.h>

#include <memory>
#include <thread>
#include <locale>
#include <codecvt>

#include "ServerAddressDialog.h"

namespace Installer
{

class MainDialog
{
public:
    MainDialog();
    ~MainDialog();

    void Run();

private:
    std::shared_ptr<wui::window> window;

    std::shared_ptr<wui::i_theme> bigTitleTheme, progressTextTheme;
    
    std::shared_ptr<wui::panel> progressPanel;
    std::shared_ptr<wui::text> progressText;
    std::shared_ptr<wui::progress> progressBar;

    std::shared_ptr<wui::image> meetImage;

    std::shared_ptr<wui::text> bigTitleText;
    std::shared_ptr<wui::text> versionText;
    std::shared_ptr<wui::text> informationText;

    std::shared_ptr<wui::button> actionButton;

    std::shared_ptr<wui::text> pathText;
    std::shared_ptr<wui::input> pathInput;
    std::shared_ptr<wui::button> pathButton;

    std::shared_ptr<wui::message> messageBox;
    ServerAddressDialog serverAddressDialog;

    mt::timer loadChangeTimer;

    enum class state
    {
        preamble,
        working,
        breaked,
        completed
    };
    state state_;

    enum class my_event : int32_t
    {
        connect_error,
        download_needed,
        load_changed,
        download_completed,
        runned_app_closed,
        app_installed,
        completed
    };

    static const int32_t WND_WIDTH = 400, WND_HEIGHT = 600;

    /// Connection data
    std::string baseURL;
    std::string downloadedExe;

    std::atomic<bool> newServerAddrSetted, serverAddrDialogShowed;

    std::string installPath, appPath;

    /// Interface flow
    void UpdateCustomThemes();
    void ControlCallback(wui::window_control control, std::string &tooltip_text, bool &);

    void ReceiveMyEvents(const wui::event &ev);
    
    void SelectPath();
    void ActionClick();

    void ShowError(std::string_view error);
    void SetProgress(int32_t value);

    void SetCloudParams();

    void SetParams(std::string_view address,
        std::string_view login = "",
        std::string_view password = "",
        std::string_view conf_tag = "",
        std::string_view user_name = "",
        uint16_t secure = 0, uint16_t port = 0); 

    /// Installation flow
    bool IsUninstaller();
    bool GetServerAddress();
    void CheckExists();
    void DownloadApp();
    void ServerConnectError();
    void CloseRunnedApp();
    void InstallApp();
    void InstallUninstaller();
    void Completed();
    void RunInstalled();

    void Uninstall();

    void onLoadChangeTimer();
};

}
