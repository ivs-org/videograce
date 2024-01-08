/**
 * MainDialog.cpp - Contains main dialog impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/nowide/convert.hpp>
#include <boost/filesystem.hpp>

#include <wui/config/config_impl_reg.hpp>
#include <wui/common/flag_helpers.hpp>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <MainDialog.h>

#include <Version.h>

#include <Shlobj.h>
#include <Shlwapi.h>
#include <tchar.h>

#include <algorithm>
#include <fstream>

namespace Installer
{

MainDialog::MainDialog()
    : window(new wui::window()),
    bigTitleTheme(wui::make_custom_theme()), progressTextTheme(wui::make_custom_theme()),
    progressPanel(new wui::panel()),
    progressText(new wui::text("", wui::hori_alignment::left, wui::vert_alignment::top, wui::text::tc, progressTextTheme)),
    progressBar(new wui::progress(0, 100, 0)),
    meetImage(new wui::image(IMG_MEET)),
    bigTitleText(new wui::text(wui::locale("installer", "big_title"), wui::hori_alignment::center, wui::vert_alignment::top, wui::text::tc, bigTitleTheme)),
    versionText(new wui::text(std::string("v. ") + SYSTEM_VERSION, wui::hori_alignment::center)),
    informationText(new wui::text(wui::locale("installer", !IsUninstaller() ? "preamble_install" : "preamble_uninstall"), wui::hori_alignment::center)),
    actionButton(new wui::button(wui::locale("installer", "next"), std::bind(&MainDialog::ActionClick, this))),
    pathText(new wui::text(wui::locale("installer", "path"))),
    pathInput(new wui::input()),
    pathButton(new wui::button(wui::locale("installer", "path_button"), std::bind(&MainDialog::SelectPath, this))),
    messageBox(new wui::message(window)),
    serverAddressDialog(),
    loadChangeTimer(std::bind(&MainDialog::onLoadChangeTimer, this)),
    state_(state::preamble),
    baseURL(),
    downloadedExe(),
    newServerAddrSetted(false), serverAddrDialogShowed(false),
    installPath(), appPath()
{
    UpdateCustomThemes();

    const int32_t imageSize = 200;

    wui::rect image_pos = { (WND_WIDTH - imageSize) / 2,
        WND_HEIGHT / 6,
        (WND_WIDTH - imageSize) / 2 + imageSize,
        WND_HEIGHT / 6 + imageSize };

    window->add_control(progressPanel, { 0, 30, WND_WIDTH, 80 });
    window->add_control(progressText, { 10, 55, WND_WIDTH - 10, 70 });
    window->add_control(progressBar, { 0, 80, WND_WIDTH, 90 });

    progressPanel->hide();
    progressText->hide();
    progressBar->hide();
    
    window->add_control(meetImage, image_pos);
    window->add_control(bigTitleText, { 10, image_pos.bottom + 10, WND_WIDTH - 10, image_pos.bottom + 50 });
    window->add_control(versionText, { 10, image_pos.bottom + 50, WND_WIDTH - 10, image_pos.bottom + 70 });
    window->add_control(informationText, { 10, image_pos.bottom + 80, WND_WIDTH - 10, image_pos.bottom + 170 });

    window->add_control(actionButton, { (WND_WIDTH - 110) / 2, image_pos.bottom + 180, (WND_WIDTH - 110) / 2 + 110, image_pos.bottom + 210 });

    window->set_focused(actionButton);

    if (!IsUninstaller())
    {
        window->add_control(pathText, { 10, WND_HEIGHT - 55, WND_WIDTH - 10, WND_HEIGHT - 40 });
        window->add_control(pathInput, { 10, WND_HEIGHT - 35, WND_WIDTH - 50, WND_HEIGHT - 10 });
        window->add_control(pathButton, { WND_WIDTH - 40, WND_HEIGHT - 35, WND_WIDTH - 10, WND_HEIGHT - 10 });
    }

    window->set_control_callback(std::bind(&MainDialog::ControlCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    window->subscribe(std::bind(&MainDialog::ReceiveMyEvents, this, std::placeholders::_1), wui::event_type::internal);

    wchar_t appDataPath[MAX_PATH * sizeof(wchar_t)] = { 0 };
    SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appDataPath);
    pathInput->set_text(boost::nowide::narrow(std::wstring(appDataPath) + L"\\Programs\\" SYSTEM_NAME "Client"));
}

MainDialog::~MainDialog()
{
}

void MainDialog::Run()
{
    window->init(wui::locale("installer", "title"), { -1, -1, WND_WIDTH, WND_HEIGHT },
        wui::flags_map<wui::window_style>(3,
            wui::window_style::dialog,
            wui::window_style::minimize_button,
            wui::window_style::switch_theme_button),
        [this]() {
            PostQuitMessage(IDCANCEL);
    });
}

void MainDialog::UpdateCustomThemes()
{
    bigTitleTheme->load_theme(*wui::get_default_theme());
    bigTitleTheme->set_font(wui::text::tc, wui::text::tv_font, { "Segoe UI", 32 });

    progressTextTheme->load_theme(*wui::get_default_theme());
    progressTextTheme->set_color(wui::text::tc, wui::text::tv_color, wui::theme_color(wui::window::tc, wui::window::tv_background));
    progressTextTheme->set_font(wui::text::tc, wui::text::tv_font, { "Segoe UI", 18 });
}

void MainDialog::ControlCallback(wui::window_control control, std::string &tooltip_text, bool &)
{
    if (control != wui::window_control::theme)
    {
        return;
    }

    auto theme_name = wui::get_default_theme()->get_name();

    if (theme_name == "dark")
    {
        tooltip_text = wui::locale("window", "dark_theme");
        wui::set_default_theme_from_resource("light", TXT_LIGHT_THEME, "JSONS");
    }
    else if (theme_name == "light")
    {
        tooltip_text = wui::locale("window", "light_theme");
        wui::set_default_theme_from_resource("dark", TXT_DARK_THEME, "JSONS");
    }

    UpdateCustomThemes();

    window->update_theme();
}

void MainDialog::ReceiveMyEvents(const wui::event &ev)
{
    if (ev.type == wui::event_type::internal && ev.internal_event_.type == wui::internal_event_type::user_emitted)
    {
        switch (static_cast<my_event>(ev.internal_event_.x))
        {
            case my_event::connect_error:
                ServerConnectError();
            break;
            case my_event::download_needed:
                DownloadApp();
            break;
            case my_event::download_completed:
                SetProgress(90);
                CloseRunnedApp();
            break;
            case my_event::runned_app_closed:
                if (!IsUninstaller())
                {
                    SetProgress(92);
                    InstallApp();
                }
                else
                {
                    SetProgress(50);
                    Uninstall();
                }
            break;
            case my_event::app_installed:
                SetProgress(97);
                InstallUninstaller();
            break;
            case my_event::completed:
                SetProgress(100);
                Completed();
            break;

            case my_event::load_changed:
                SetProgress(ev.internal_event_.y);
            break;
        }
    }
}

void MainDialog::SelectPath()
{
    BROWSEINFO bi = { 0 };
    bi.hwndOwner = window->context().hwnd;
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    bi.lParam = (LPARAM)this;

    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

    if (pidl != 0)
    {
        wchar_t path[MAX_PATH * sizeof(wchar_t)];
        
        SHGetPathFromIDList(pidl, path);

        IMalloc * imalloc = 0;
        if (SUCCEEDED(SHGetMalloc(&imalloc)))
        {
            imalloc->Free(pidl);
            imalloc->Release();
        }

        pathInput->set_text(boost::nowide::narrow(path));
    }
}

void MainDialog::ActionClick()
{
    switch (state_)
    {
        case state::preamble:
            state_ = state::working;

            actionButton->set_caption(wui::locale("button", "cancel"));

            pathText->hide();
            pathInput->hide();
            pathButton->hide();

            progressPanel->show();
            progressText->show();
            progressBar->show();

            if (!IsUninstaller())
            {
                informationText->set_text(wui::locale("installer", "installing"));
                SetProgress(0);

                if (GetServerAddress() && state_ == state::working)
                {
                    SetProgress(80);
                    CheckExists();
                }
            }
            else
            {
                informationText->set_text(wui::locale("installer", "uninstalling"));
                SetProgress(0);
                
                CloseRunnedApp();
            }
        break;
        case state::working:
            state_ = state::breaked;
            window->destroy();
        break;
        case state::breaked:
            // Do nothing
        break;
        case state::completed:
            if (!IsUninstaller())
            {
                RunInstalled();
            }
            window->destroy();
        break;
    }
}

void MainDialog::ShowError(std::string_view error_)
{
    messageBox->show(error_, wui::locale("installer_error", "title"), wui::message_icon::alert, wui::message_button::ok, [this](wui::message_result) { window->destroy(); });
}

void MainDialog::SetProgress(int32_t value)
{
    progressText->set_text(wui::locale("installer", !IsUninstaller() ? "installation_progress" : "uninstallation_progress") + "(" + std::to_string(value) + "%)");
    progressBar->set_value(value);
}

void MainDialog::SetCloudParams()
{
    uint16_t secure = 0;

    std::vector<std::string> vals;
    boost::split(vals, CLOUD_ADDRESS, boost::is_any_of("/"));
    if (vals.size() > 2)
    {
        if (vals[0] == "https:")
        {
            secure = 1;
        }

        SetParams(vals[2], "", "", "", "", secure, 0);
    }
}

void MainDialog::SetParams(std::string_view address,
    std::string_view login,
    std::string_view password,
    std::string_view conf_tag,
    std::string_view user_name,
    uint16_t secure, uint16_t port)
{
    auto server = std::string(address) + (port != 0 ? ":" + std::to_string(port) : "");

    if (!login.empty() || server != wui::config::get_string("Connection", "Address", ""))
    {
        wui::config::set_string("Credentials", "Login", login);
        wui::config::set_string("Credentials", "Password", password);
    }

    if (!conf_tag.empty())
    {
        wui::config::set_string("User", "ConferenceTag", conf_tag);
    }

    if (!user_name.empty())
    {
        wui::config::set_string("User", "Name", user_name);
    }

    wui::config::set_int("Connection", "Secure", secure);
    wui::config::set_string("Connection", "Address", server);

    baseURL = (secure != 0 ? "https://" : "http://") + server;
}

bool MainDialog::IsUninstaller()
{
    wchar_t moduleFileName[4096] = { 0 };
    GetModuleFileNameW(NULL, moduleFileName, 4096);

    return std::wstring(moduleFileName).find(L"uninstall") != std::wstring::npos;
}

bool MainDialog::GetServerAddress()
{
    wchar_t moduleFileName_[4096] = { 0 };
    GetModuleFileNameW(NULL, moduleFileName_, 4096);
    const std::string moduleFileName(boost::nowide::narrow(moduleFileName_));
    //const std::string moduleFileName("Installer-eyJhIjoiMTkyLjE2OC4xLjEwMSIsInMiOjAsInAiOjg3Nzh9.exe");

    size_t b64Pos = moduleFileName.find_last_of("-");
    if (b64Pos == std::string::npos)
    {
        SetCloudParams();
        return true;
    }

    size_t exePos = moduleFileName.find(".exe");
    if (exePos == std::string::npos || b64Pos == exePos)
    {
        SetCloudParams();
        return true;
    }

    const std::string b64 = moduleFileName.substr(b64Pos + 1, exePos - (b64Pos + 1));
    const std::string json = Common::fromBase64(b64);

    std::string address, login, password, conf_tag, user_name;
    uint16_t secure = 0, port = 0;
    try
    {
        //spdlog::get("System")->trace("api::redirect_user :: perform parsing");

        auto j = nlohmann::json::parse(json);

        if (j.count("a") != 0) address = j.at("a").get<std::string>();
        if (j.count("s") != 0) secure = j.at("s").get<uint16_t>();
        if (j.count("p") != 0) port = j.at("p").get<uint16_t>();
        if (j.count("l") != 0) login = j.at("l").get<std::string>();
        if (j.count("w") != 0) password = j.at("w").get<std::string>();
        if (j.count("c") != 0) conf_tag = j.at("c").get<std::string>();
        if (j.count("n") != 0) user_name = j.at("n").get<std::string>();
    }
    catch (nlohmann::json::parse_error& ex)
    {
        //spdlog::get("Error")->critical("api::redirect_user :: error parse json (byte: {0}, what: {1})", ex.byte, ex.what());

        ShowError(wui::locale("installer_error", "parse_error"));
        
        return false;
    }

    SetParams(address, login, password, conf_tag, user_name, secure, port);
    return true;
}

void MainDialog::CheckExists()
{
    auto checkPath = std::wstring(boost::nowide::widen(pathInput->text())) + L"\\" _T(SYSTEM_NAME) L"Client.exe";
    if (!PathFileExistsW(checkPath.c_str()) || IsWrongVersion())
    {
        window->emit_event(static_cast<int32_t>(my_event::download_needed), 0);
    }
    else
    {
        appPath = pathInput->text() + "\\" + SYSTEM_NAME "Client.exe";
        window->emit_event(static_cast<int32_t>(my_event::completed), 0);
    }
}

bool MainDialog::IsWrongVersion()
{
    std::vector<int32_t> wrong_vers = { { 522 } };

    auto szVersionFile = std::wstring(boost::nowide::widen(pathInput->text())) + L"\\" _T(SYSTEM_NAME) L"Client.exe";

    DWORD  verHandle = 0;
    UINT   size = 0;
    LPBYTE lpBuffer = NULL;
    DWORD  verSize = GetFileVersionInfoSize(szVersionFile.c_str(), &verHandle);

    if (verSize != NULL)
    {
        LPSTR verData = new char[verSize];

        if (GetFileVersionInfo(szVersionFile.c_str(), verHandle, verSize, verData))
        {
            if (VerQueryValue(verData, L"\\", (VOID FAR * FAR*) & lpBuffer, &size))
            {
                if (size)
                {
                    VS_FIXEDFILEINFO* verInfo = (VS_FIXEDFILEINFO*)lpBuffer;
                    if (verInfo->dwSignature == 0xfeef04bd)
                    {
                        if (std::find(wrong_vers.begin(), wrong_vers.end(), ((verInfo->dwFileVersionLS >> 0) & 0xffff)) != wrong_vers.end())
                        {
                            return true;
                        }
                    }
                }
            }
        }
        delete[] verData;
    }

    return false;
}

void MainDialog::DownloadApp()
{
    std::thread ([this]() {
    start:
        {
            loadChangeTimer.start();

            Transport::HTTPClient httpClient([this](int32_t code, std::string_view) {
                if (code != 10057)
                {
                    window->emit_event(static_cast<int32_t>(my_event::connect_error), 0);
                }});
            httpClient.Connect(baseURL);

            if (Common::Is64BitSystem())
            {
                downloadedExe = httpClient.Request("/update/x64/" SYSTEM_NAME "Client.exe", "GET");
            }
            else
            {
                downloadedExe = httpClient.Request("/update/" SYSTEM_NAME "Client.exe", "GET");
            }

            loadChangeTimer.stop();

            if (downloadedExe.empty())
            {
                while (!serverAddrDialogShowed && state_ == state::working)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                }

                if (!newServerAddrSetted)
                {
                    return;
                }

                newServerAddrSetted = false;
                serverAddrDialogShowed = false;

                if (state_ == state::working)
                {
                    goto start;
                }
            }

            if (state_ == state::working)
            {
                window->emit_event(static_cast<int32_t>(my_event::download_completed), 0);
            }
        }
    }).detach();
}

void MainDialog::ServerConnectError()
{
    if (serverAddrDialogShowed)
    {
        return;
    }

    char buff[1024] = { 0 };
    snprintf(buff, sizeof(buff), wui::locale("installer_error", "server_not_found").c_str(), baseURL.c_str());
    messageBox->show(buff, wui::locale("installer_error", "title"), wui::message_icon::alert, wui::message_button::yes_no, [this](wui::message_result result) {
        if (result == wui::message_result::yes)
        {
            serverAddressDialog.Run(window, [this](bool ok) {
                if (ok)
                {
                    auto secure = wui::config::get_int("Connection", "Secure", 0);
                    auto server = wui::config::get_string("Connection", "Address", "");

                    baseURL = (secure != 0 ? "https://" : "http://") + server;

                    newServerAddrSetted = true;
                    serverAddrDialogShowed = true;
                }
                else
                {
                    serverAddrDialogShowed = true;
                    window->destroy();
                }
            });
        }
        else
        {
            serverAddrDialogShowed = true;
            window->destroy();
        }
    });
}

void MainDialog::CloseRunnedApp()
{
    std::thread([this]() {
        ShellExecute(NULL, L"open", L"taskkill", L"/f /IM " SYSTEM_NAME "Client.exe ", NULL, SW_HIDE);
        
        std::this_thread::sleep_for(std::chrono::seconds(2));

        window->emit_event(static_cast<int32_t>(my_event::runned_app_closed), 0);
    }).detach();
}

HRESULT CreateLink(LPCWSTR lpszPathObj, LPCWSTR lpszPathLink, LPCWSTR lpszDesc)
{
    ::CoInitialize(NULL);

    HRESULT hres;
    IShellLink* psl;

    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
    if (SUCCEEDED(hres))
    {
        IPersistFile* ppf;

        psl->SetPath(lpszPathObj);
        psl->SetDescription(lpszDesc);

        hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);

        if (SUCCEEDED(hres))
        {
            hres = ppf->Save(lpszPathLink, TRUE);
            ppf->Release();
        }
        psl->Release();
    }

    ::CoUninitialize();

    return hres;
}

void MainDialog::InstallApp()
{
    installPath = pathInput->text();

    boost::filesystem::path p(boost::nowide::widen(installPath));

    std::wstring dir;
    for (auto &part : p)
    {
        dir += part.c_str() + std::wstring(L"\\");
        CreateDirectory(dir.c_str(), NULL);
    }

    std::ofstream exefile;
    exefile.open(boost::nowide::widen(installPath) + L"\\" _T(SYSTEM_NAME) L"Client.exe", std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
    exefile << downloadedExe;
    exefile.close();

    appPath = installPath + "\\" SYSTEM_NAME "Client.exe";

    wchar_t desktopPath[MAX_PATH * sizeof(wchar_t)] = { 0 };
    SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, desktopPath);
    std::wstring shortcutPath = std::wstring(desktopPath) + L"\\" _T(SYSTEM_NAME) L" Client.lnk";

    CreateLink(boost::nowide::widen(appPath).c_str(), shortcutPath.c_str(), L"");

    window->emit_event(static_cast<int32_t>(my_event::app_installed), 0);
}

void MainDialog::InstallUninstaller()
{
    wchar_t moduleFileName[4096] = { 0 };
    GetModuleFileName(NULL, moduleFileName, 4096);

    CopyFile(moduleFileName, std::wstring(boost::nowide::widen(installPath) + L"\\uninstall.exe").c_str(), FALSE);

	wui::config::config_impl_reg cir("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");

    cir.set_string(CLIENT_INSTALLER_GUID, "DisplayName", SYSTEM_NAME " Client");
    cir.set_string(CLIENT_INSTALLER_GUID, "Publisher", COMPANY_NAME);
    cir.set_string(CLIENT_INSTALLER_GUID, "DisplayIcon", appPath);
	cir.set_string(CLIENT_INSTALLER_GUID, "InstallLocation", installPath);
	cir.set_string(CLIENT_INSTALLER_GUID, "QuietUninstallString", "\"" + installPath + "\\uninstall.exe\"");
	cir.set_string(CLIENT_INSTALLER_GUID, "UninstallString", "\"" + installPath + "\\uninstall.exe\"");
	cir.set_string(CLIENT_INSTALLER_GUID, "DisplayVersion", STR(MAJOR_VERSION) "." STR(MINOR_VERSION));
	cir.set_int(CLIENT_INSTALLER_GUID, "VersionMinor", MAJOR_VERSION);
	cir.set_int(CLIENT_INSTALLER_GUID, "VersionMajor", MINOR_VERSION);
	cir.set_int(CLIENT_INSTALLER_GUID, "MajorVersion", MAJOR_VERSION);
	cir.set_int(CLIENT_INSTALLER_GUID, "MinorVersion", MINOR_VERSION);
	cir.set_int(CLIENT_INSTALLER_GUID, "NoModify", 1);
    cir.set_int(CLIENT_INSTALLER_GUID, "NoRepair", 1);

	wui::config::config_impl_reg cir1("SOFTWARE\\Microsoft\\Windows\\CurrentVersion");
	cir1.set_string("Run", SYSTEM_NAME "Client", appPath + " /autorun");

    window->emit_event(static_cast<int32_t>(my_event::completed), 0);
}

void MainDialog::Completed()
{
    state_ = state::completed;
    informationText->set_text(wui::locale("installer", !IsUninstaller() ? "install_completed" : "uninstall_completed"));
    actionButton->set_caption(wui::locale("button", "complete"));
}

void MainDialog::RunInstalled()
{
    STARTUPINFO si = { 0 };
    PROCESS_INFORMATION pi = { 0 };

    si.cb = sizeof(si);

    CreateProcessW(NULL,
        (LPWSTR)boost::nowide::widen(appPath).c_str(),
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi);
}

void MainDialog::Uninstall()
{
    wchar_t moduleFileName[4096] = { 0 };
    GetModuleFileName(NULL, moduleFileName, 4096);

    boost::filesystem::path p(moduleFileName);
    p.remove_filename();

    auto installPath = p.c_str();

    wchar_t appDataPath[MAX_PATH * sizeof(wchar_t)] = { 0 };
    SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appDataPath);

    std::wofstream cmdfile;
    cmdfile.open(std::wstring(appDataPath) + L"\\remove.cmd", std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
    //cmdfile.imbue(std::locale(cmdfile.getloc(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::little_endian>));
    cmdfile << L"@chcp 65001\r\n:loop\r\n" \
        L"if exist \"" << installPath << L"\\uninstall.exe\" (\r\n"\
        L"rmdir /S /Q \"" << installPath << L"\" \r\n"\
        L"goto loop \r\n"\
        L")\r\n"\
        L"del \"" << appDataPath << L"\\remove.cmd\"";
    cmdfile.close();

    ShellExecute(NULL, L"open", std::wstring(std::wstring(appDataPath) + L"\\remove.cmd").c_str(), 0, 0, SW_HIDE);

    wchar_t desktopPath[MAX_PATH * sizeof(wchar_t)] = { 0 };
    SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, desktopPath);
    std::wstring shortcutPath = std::wstring(desktopPath) + L"\\" _T(SYSTEM_NAME) L" Client.lnk";

    DeleteFile(shortcutPath.c_str());

	wui::config::config_impl_reg cir("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
    cir.delete_key(CLIENT_INSTALLER_GUID);

    ShellExecute(NULL, L"runas", L"reg", L"delete HKCR\\vg /f", 0, SW_HIDE);

	wui::config::config_impl_reg cir1("SOFTWARE\\Microsoft\\Windows\\CurrentVersion");
    cir.delete_value("Run", SYSTEM_NAME "Client");

	wui::config::config_impl_reg cir2("vg", HKEY_CLASSES_ROOT);
    cir2.delete_key("");

    window->emit_event(static_cast<int32_t>(my_event::completed), 0);
}

void MainDialog::onLoadChangeTimer()
{
    static int value = 0;
    window->emit_event(static_cast<int32_t>(my_event::load_changed), value);
    ++value;
    if (value >= 100) value = 75;
}

}
