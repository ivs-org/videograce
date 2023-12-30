/**
 * MainFrame.cpp - Contains main frame impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <UI/MainFrame.h>

#include <wui/framework/framework.hpp>

#include <wui/theme/theme.hpp>
#include <wui/theme/theme_selector.hpp>

#include <wui/locale/locale.hpp>
#include <wui/system/tools.hpp>
#include <wui/system/wm_tools.hpp>
#include <wui/system/uri_tools.hpp>
#include <wui/common/flag_helpers.hpp>
#include <wui/config/config.hpp>
#include <wui/config/config_impl_reg.hpp>

#include <Version.h>
#include <Common/Base64.h>
#include <Common/BitHelpers.h>
#include <Common/FSHelpers.h>
#include <Common/Process.h>

#include <License/Grants.h>

#include <Proto/ConferenceGrants.h>

#include <boost/nowide/convert.hpp>
#include <boost/algorithm/string.hpp>

#ifdef _WIN32
#include <Shlobj.h>
#else
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <random>

#include <resource.h>

namespace Client
{

enum class MyEvent : uint32_t
{
    Controller                = 10000,

    ShowBusy                  = 4000,
    SetBusyProgress           = 4001,
    HideBusy                  = 4002,

    SetMainProgress           = 4010,

    CredentialsReady          = 4100,
    NewCredentials            = 4101,

    SpeedTestCompleted        = 4200,
    ConnectivityTestCompleted = 4201,

    RingerEnd                 = 4202,

    Normalize                 = 4050
};

#ifdef _WIN32
std::string GetAppDataPath()
{
    wchar_t appDataPath_[MAX_PATH] = { 0 };
    SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appDataPath_);

    std::wstring appDataPath = appDataPath_;
    appDataPath += L"\\IVS";
    CreateDirectoryW(appDataPath.c_str(), NULL);

    appDataPath += L"\\" _T(SYSTEM_NAME);
    CreateDirectoryW(appDataPath.c_str(), NULL);

    auto srvUsrCode = Common::toBase64(wui::config::get_string("Connection", "Address", "") + wui::config::get_string("Credentials", "Login", ""));
    std::replace(srvUsrCode.begin(), srvUsrCode.end(), '+', 'a');
    std::replace(srvUsrCode.begin(), srvUsrCode.end(), '-', 'b');
    std::replace(srvUsrCode.begin(), srvUsrCode.end(), '/', 'c');
    std::replace(srvUsrCode.begin(), srvUsrCode.end(), '=', 'd');

    appDataPath += L"\\" + boost::nowide::widen(srvUsrCode);
    CreateDirectoryW(appDataPath.c_str(), NULL);

    return boost::nowide::narrow(appDataPath) + "\\";
}

#else

std::string GetAppDataPath()
{
    auto homeDir = getpwuid(getuid())->pw_dir;

    mkdir(std::string(std::string(homeDir) + "/." CLIENT_USER_FOLDER "/").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    auto srvUsrCode = Common::toBase64(wui::config::get_string("Connection", "Address", "") + wui::config::get_string("Credentials", "Login", ""));
    std::replace(srvUsrCode.begin(), srvUsrCode.end(), '+', 'a');
    std::replace(srvUsrCode.begin(), srvUsrCode.end(), '-', 'b');
    std::replace(srvUsrCode.begin(), srvUsrCode.end(), '/', 'c');
    std::replace(srvUsrCode.begin(), srvUsrCode.end(), '=', 'd');

    mkdir(std::string(std::string(homeDir) + "/." CLIENT_USER_FOLDER "/" + srvUsrCode).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    return std::string(std::string(homeDir) + "/." CLIENT_USER_FOLDER "/" + srvUsrCode + "/");
}

#endif

std::string GetRecordName()
{
    time_t t = time(0);
    struct tm now = { 0 };

#ifdef _WIN32
    localtime_s(&now, &t);
#else
    localtime_r(&t, &now);
#endif

    std::string dt = std::to_string(1900 + now.tm_year) + "." + std::to_string(1 + now.tm_mon) + "." +
        std::to_string(now.tm_mday) + "-" + std::to_string(now.tm_hour) + "." +
        std::to_string(now.tm_min) + "." + std::to_string(now.tm_sec);

    std::string path = wui::config::get_string("Record", "Path", "");

#ifdef _WIN32
    if (path.empty())
    {
        wchar_t myDocuments[MAX_PATH] = { 0 };
        HRESULT result = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, myDocuments);
        if (result == S_OK)
        {
            path = boost::nowide::narrow(myDocuments);
        }
    }
    std::string out = path + "\\" + dt + (wui::config::get_int("Record", "MP3Mode", 0) != 0 ? ".mp3" : ".mkv");
#else
    if (path.empty())
    {
        path = getpwuid(getuid())->pw_dir;
    }
    std::string out = path + "/" + dt + (wui::config::get_int("Record", "MP3Mode", 0) != 0 ? ".mp3" : ".mkv");
#endif

    return out;
}

MainFrame::MainFrame()
    : window(new wui::window()),
    messageBox(new wui::message(window)),
    mainMenu(new wui::menu()),
    mainProgress(new wui::progress(0, 100, 0)),
    volumeBox(new VolumeBox(std::bind(&MainFrame::VolumeBoxChangeCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))),
    trayIcon(),

    controllerEventsQueueMutex(), controllerEventsQueue(),

    activateHandler(std::bind(&MainFrame::ActivateCallback, this)),
    
    storage(),
    
    contactList(storage, controller, conferenceDialog, *this),
    memberList(storage, controller),

    controller(storage, memberList),

    timeMeter(),
    
    speedTester(wui::get_locale(),
        std::bind(&MainFrame::SpeedTestCompleted, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&MainFrame::SetMainProgess, this, std::placeholders::_1, std::placeholders::_2)),
    udpTester(wui::get_locale()),

    cameraDevices(),
    microphoneDevices(),
    audioRendererDevices(),

    captureVideoSession(), demonstrationSession(),
    captureAudioSession(),

    recorder(),
    audioRenderer(), resampler(audioRenderer), audioMixer(),
    renderersVideo(), renderersAudio(),

    cpuMeter(),

    ringer(audioMixer, [this](Ringer::RingType rt) { window->emit_event(static_cast<int32_t>(MyEvent::RingerEnd), static_cast<int32_t>(rt)); }),

    miniWindow(window),

    mainToolBar(window, *this),
    listPanel(window, contactList, memberList, *this),
    contentPanel(window, storage, controller, *this),
    renderersBox(window, miniWindow, renderersVideo, controller),
    timerBar(window, timeMeter, std::bind(&MainFrame::DisconnectFromConference, this)),

    busyBox(window),
    busyTitle(), busySubTitle(),

    aboutDialog(window),

    credentialsDialog(window, controller, std::bind(&MainFrame::CredentialsCloseCallback, this)),

#ifdef _WIN32
    dummyGraph(),
#endif

    dialingDialog(window, std::bind(&MainFrame::CancelCall, this)),
    questionDialog(window, std::bind(&MainFrame::QuestionCall, this, std::placeholders::_1)),
    conferenceDialog(window, controller, storage),
    settingsDialog(window,
        controller,
        audioRenderer,
        resampler,
        audioMixer,
        ringer,
        std::bind(&MainFrame::DetermineNetSpeed, this, std::placeholders::_1),
        std::bind(&MainFrame::CheckConnectivity, this, std::placeholders::_1),
        std::bind(&MainFrame::SettingsReadyCallback, this)),

    showConnectivityResult(false),
    actionQuested(false), useWSMedia(false),
    online(false), working(true),
    userForceClose(false),

    callParams(),

    sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{
    window->subscribe(std::bind(&MainFrame::ReceiveEvents, this, std::placeholders::_1),
        wui::flags_map<wui::event_type>(3,
            wui::event_type::internal,
            wui::event_type::system,
            wui::event_type::keyboard));
    window->set_control_callback(std::bind(&MainFrame::WindowControlCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    window->set_min_size(700, 400);

    mainMenu->set_items({
        { 1, wui::menu_item_state::separator, wui::locale("menu", "settings"), "", nullptr, {
            { 11, wui::menu_item_state::normal, wui::locale("settings", "camera"),      "", nullptr, {}, [this](int32_t i) { settingsDialog.Run(SettingsSection::Camera); } },
            { 12, wui::menu_item_state::normal, wui::locale("settings", "microphone"),  "", nullptr, {}, [this](int32_t i) { settingsDialog.Run(SettingsSection::Microphone); } },
            { 13, wui::menu_item_state::normal, wui::locale("settings", "loudspeaker"), "", nullptr, {}, [this](int32_t i) { settingsDialog.Run(SettingsSection::Loudspeaker); } },
            { 14, wui::menu_item_state::normal, wui::locale("settings", "account"),     "", nullptr, {}, [this](int32_t i) { settingsDialog.Run(SettingsSection::Account); } },
            { 15, wui::menu_item_state::normal, wui::locale("settings", "connection"),  "", nullptr, {}, [this](int32_t i) { settingsDialog.Run(SettingsSection::Connection); } },
            { 16, wui::menu_item_state::normal, wui::locale("settings", "preferences"), "", nullptr, {}, [this](int32_t i) { settingsDialog.Run(SettingsSection::Preferences); } },
            { 17, wui::menu_item_state::separator, wui::locale("settings", "record"),   "", nullptr, {}, [this](int32_t i) { settingsDialog.Run(SettingsSection::Record); } }
        }, [](int32_t i) {} },
        { 2, wui::menu_item_state::separator, wui::locale("menu", wui::config::get_int("Record", "Enabled", 0) != 0 ? "disable_record" : "enable_record"), "Ctrl+R", nullptr, {}, [this](int32_t i) { ActionTurnRecord(); } },
        { 3, wui::menu_item_state::normal, wui::locale("menu", "help"), "", nullptr, {}, [this](int32_t i) { ShowHelp(); } },
        { 4, wui::menu_item_state::separator, wui::locale("menu", "about"), "", nullptr, {}, [this](int32_t i) { aboutDialog.Run(controller.GetGrants()); } },
        { 5, wui::menu_item_state::normal, wui::locale("menu", "exit"), "Alt+F4", nullptr, {}, [this](int32_t i) { ConfirmClose(true); } }
    });

    window->add_control(mainMenu, { 0 });

    window->add_control(volumeBox, { 0 });

    controller.SetEventHandler([this](const Controller::Event &ev) {
        //std::lock_guard<std::mutex> lock(controllerEventsQueueMutex);
        controllerEventsQueue.push(ev);

        if (window)
        {
            window->emit_event(static_cast<int32_t>(MyEvent::Controller), 0);
        }
    });

    storage.SubscribeMessagesReceiver(std::bind(&MainFrame::MessagesUpdatedCallback, this, std::placeholders::_1, std::placeholders::_2));

    if (!wui::config::get_string("Connection", "Address", "").empty() && !wui::config::get_string("Credentials", "Login", "").empty())
    {
        storage.Connect(GetAppDataPath() + "local.db");
    }

    audioRenderer.SetErrorHandler(std::bind(&MainFrame::AudioRendererErrorCallback, this, std::placeholders::_1, std::placeholders::_2));
    audioMixer.SetReceiver(&resampler);

    mainProgress->hide();
}

MainFrame::~MainFrame()
{
}

void MainFrame::Run(bool minimized)
{
    sysLog->info("The application started");

#ifdef _WIN32
    ::CoInitialize(NULL);
    dummyGraph.Start();
#endif

    if (minimized)
    {
        window->minimize();
    }

    window->add_control(mainProgress, {});

    window->init(wui::locale("client", "title") + SHORT_VERSION " - " + wui::locale("common", "offline"),
        { -1, -1, wui::config::get_int("MainFrame", "Width", 1200), wui::config::get_int("MainFrame", "Height", 700) },
        wui::flags_map<wui::window_style>(2, wui::window_style::frame, wui::window_style::switch_theme_button),
        [this]() {
        if (controller.GetState() == Controller::State::Conferencing)
        {
            if (!callParams.callingSubscriber.empty())
            {
                CancelCall();
            }

            DisconnectFromConference();
        }

        trayIcon.reset();

        working = false;

        speedTester.Stop();

        sysLog->info("The application perform ending");

#ifdef _WIN32
        ::CoUninitialize();
#endif
        wui::framework::stop();
    });

	if (minimized)
	{
		wui::hide_taskbar_icon(window->context());
	}

    trayIcon = std::shared_ptr<wui::tray_icon>(new wui::tray_icon(window, ICO_INACTIVE, wui::locale("client", "title") + "(" + wui::locale("common", "offline") + ")", std::bind(&MainFrame::TrayIconCallback, this, std::placeholders::_1)));

    InitAudio();

    bool secureConnection = wui::config::get_int("Connection", "Secure", 0) != 0;
    auto serverAddress = wui::config::get_string("Connection", "Address", "");
    auto login = wui::config::get_string("Credentials", "Login", "");
    auto password = wui::config::get_string("Credentials", "Password", "");

    controller.SetCredentials(login, password);
    controller.Connect(serverAddress, secureConnection);
}

wui::system_context &MainFrame::context()
{
    return window->context();
}

/// IControlActions impl
void MainFrame::ActionCall()
{
    if (controller.GetState() == Controller::State::Conferencing && listPanel.GetPanelMode() == ListPanel::PanelMode::MemberList)
    {
        return memberList.Add();
    }
    if (controller.GetState() != Controller::State::Conferencing && controller.GetState() != Controller::State::Ready)
    {
        return messageBox->show(wui::locale("message", "not_connection_for_calling"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok);
    }

    auto selection = contactList.GetSelection();
    switch (selection.type)
    {
        case ContactList::SelectionType::Undefined:
            return messageBox->show(wui::locale("message", "need_select_user_or_conference"),
                wui::locale("message", "title_error"),
                wui::message_icon::alert,
                wui::message_button::ok);
        break;
        case ContactList::SelectionType::Client:
            Call(selection.id);
        break;
        case ContactList::SelectionType::Conference:
            ConnectToConference(selection.id, selection.my);
        break;
    }
}

void MainFrame::ActionConference()
{
    if (!License::Parse(controller.GetGrants()).allowedCreatingConferences)
    {
        return messageBox->show(wui::locale("message", "conference_creating_denied"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok);
    }

    if (controller.GetState() == Controller::State::Conferencing)
    {
        return messageBox->show(wui::locale("message", "conference_creating_not_possible_in_conference"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok);
    }

    conferenceDialog.Run(Proto::Conference(), [this](std::string_view tag) {
        ConnectToConference(tag, true);
    });
}

void MainFrame::ActionHangup()
{
    if (!callParams.callingSubscriber.empty())
    {
        CancelCall();
    }
    if (controller.GetState() == Controller::State::Conferencing)
    {
        DisconnectFromConference();
    }
}

void MainFrame::ActionDevices()
{

}

void MainFrame::ActionTurnCamera(bool my, int64_t actorId, std::string_view actorName)
{
    if (!IsCameraExists())
    {
        return messageBox->show(wui::locale("message", "no_camera_in_system"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok);
    }

    auto enabled = IsCameraEnabled();

    enabled = !enabled;

    if (my && BitIsSet(controller.GetCurrentConference().grants, static_cast<int32_t>(Proto::ConferenceGrants::DenyTurnCamera)) &&
        ((!memberList.IsMePresenter() && !memberList.IsMeModerator()) || memberList.IsMeReadOnly()))
    {
        return messageBox->show(wui::locale("message", "turn_camera_denied"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok);
    }

    miniWindow.SetCameraState(enabled);

    if (!my && enabled && wui::config::get_int("User", "RequestTurnCamMic", 1) != 0 && BitIsClear(controller.GetCurrentConference().grants, static_cast<int32_t>(Proto::ConferenceGrants::DontAskTurnDevices)))
    {
        if (actionQuested)
        {
            return controller.SendMemberActionResult(actorId, Proto::MEMBER_ACTION::Result::Busy);
        }

        actionQuested = true;

        return messageBox->show(std::string(actorName) + " " + wui::locale("message", "allow_enable_camera"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::yes_no, [this, actorId](wui::message_result result) {
                actionQuested = false;
                if (result == wui::message_result::yes)
                {
                    mainToolBar.EnableCamera(true);
                    wui::config::set_int("CaptureDevices", "CameraEnabled", 1);
                    StartCamera();
                }
                else
                {
                    controller.SendMemberActionResult(actorId, Proto::MEMBER_ACTION::Result::Rejected);
                }
        });

        actionQuested = false;
    }

    mainToolBar.EnableCamera(enabled);

    wui::config::set_int("CaptureDevices", "CameraEnabled", enabled ? 1 : 0);

    if (enabled)
    {
        StartCamera();
    }
    else
    {
        StopCamera();
    }
}

void MainFrame::ActionTurnMicrophone(bool my, int64_t actorId, std::string_view actorName)
{
    if (!IsMicrophoneExists())
    {
        return messageBox->show(wui::locale("message", "no_microphone_in_system"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok);
    }

    auto enabled = IsMicrophoneEnabled();

    enabled = !enabled;

    if (my && BitIsSet(controller.GetCurrentConference().grants, static_cast<int32_t>(Proto::ConferenceGrants::DenyTurnMicrophone)) &&
        ((!memberList.IsMePresenter() && !memberList.IsMeModerator()) || memberList.IsMeReadOnly()))
    {
        return messageBox->show(wui::locale("message", "turn_microphone_denied"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok);
    }

    miniWindow.SetMicrophoneState(enabled);

    if (!my && enabled && wui::config::get_int("User", "RequestTurnCamMic", 1) != 0 && BitIsClear(controller.GetCurrentConference().grants, static_cast<int32_t>(Proto::ConferenceGrants::DontAskTurnDevices)))
    {
        if (actionQuested)
        {
            return controller.SendMemberActionResult(actorId, Proto::MEMBER_ACTION::Result::Busy);
        }

        actionQuested = true;

        return messageBox->show(std::string(actorName) + " " + wui::locale("message", "allow_enable_microphone"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::yes_no, [this, actorId](wui::message_result result) {
            actionQuested = false;
            if (result == wui::message_result::yes)
            {
                mainToolBar.EnableMicrophone(true);
                wui::config::set_int("CaptureDevices", "MicrophoneEnabled", 1);
                StartMicrophone();
            }
            else
            {
                controller.SendMemberActionResult(actorId, Proto::MEMBER_ACTION::Result::Rejected);
            }
        });

        actionQuested = false;
    }

    mainToolBar.EnableMicrophone(enabled);

    wui::config::set_int("CaptureDevices", "MicrophoneEnabled", enabled ? 1 : 0);

    if (enabled)
    {
        StartMicrophone();
    }
    else
    {
        StopMicrophone();
    }
}

void MainFrame::ActionVolumeMicrophone()
{
    volumeBox->Acivate(mainToolBar.GetMicrophoneGainControl()->position(),
        wui::config::get_int("CaptureDevices", "MicrophoneGain", 80),
        VolumeBoxMode::Microphone,
        IsMicrophoneEnabled());
}

void MainFrame::ActionTurnLoudspeaker()
{
    auto enabled = audioRenderer.GetMute();

    mainToolBar.EnableLoudspeaker(enabled);

    audioRenderer.SetMute(!enabled);
}

void MainFrame::ActionVolumeLoudspeaker()
{
    volumeBox->Acivate(mainToolBar.GetSpeakerVolumeControl()->position(),
        audioRenderer.GetVolume(),
        VolumeBoxMode::Loudspeaker,
        wui::config::get_int("AudioRenderer", "Enabled", 1) != 0);
}

void MainFrame::ActionTurnRendererGrid()
{
    auto big = wui::config::get_int("VideoRenderer", "GridType", 1) != 0;

    big = !big;

    mainToolBar.ChangeRendererMode(big);

    renderersBox.SetGridType(big ? GridType::MainUser : GridType::Even);

    wui::config::set_int("VideoRenderer", "GridType", big ? 1 : 0);
}

void MainFrame::ActionTurnDemonstration(bool my, int64_t actorId, std::string_view actorName)
{
    if (controller.GetState() != Controller::State::Conferencing)
    {
        return messageBox->show(std::string(actorName) + " " + wui::locale("message", "demonstration_only_on_conference"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok);
    }

    auto enabled = IsDemonstrationEnabled();

    enabled = !enabled;

    if (!my && enabled && wui::config::get_int("User", "RequestTurnCamMic", 1) != 0 && BitIsClear(controller.GetCurrentConference().grants, static_cast<int32_t>(Proto::ConferenceGrants::DontAskTurnDevices)))
    {
        if (actionQuested)
        {
            return controller.SendMemberActionResult(actorId, Proto::MEMBER_ACTION::Result::Busy);
        }

        actionQuested = true;

        return messageBox->show(std::string(actorName) + " " + wui::locale("message", "allow_enable_screen_capturing"),
            wui::locale("message", "title_confirmation"),
            wui::message_icon::alert,
            wui::message_button::yes_no, [this, actorId](wui::message_result result) {
            actionQuested = false;
            if (result == wui::message_result::yes)
            {
                mainToolBar.EnableScreenCapturer(true);
                StartDemonstration();
            }
            else
            {
                controller.SendMemberActionResult(actorId, Proto::MEMBER_ACTION::Result::Rejected);
            }
        });

        actionQuested = false;
    }

    mainToolBar.EnableScreenCapturer(enabled);

    if (enabled)
    {
        StartDemonstration();
    }
    else
    {
        StopDemonstration();
    }
}

void MainFrame::ActionTurnRemoteControl(bool my, int64_t actorId, std::string_view actorName, bool enable)
{
    if (!demonstrationSession)
    {
        return errLog->critical("MainFrame::ActionTurnRemoteControl() -> no screen capture enabled");
    }

    if (actionQuested)
    {
        return controller.SendMemberActionResult(actorId, Proto::MEMBER_ACTION::Result::Busy);
    }

    if (demonstrationSession->IsRCActionsEnabled())
    {
        if (!enable)
        {
            demonstrationSession->SetRCActions(false);
        }
        else
        {
            controller.SendMemberActionResult(actorId, Proto::MEMBER_ACTION::Result::Rejected);
        }
    }
    else
    {
        if (enable)
        {
            actionQuested = true;

            return messageBox->show(std::string(actorName) + " " + wui::locale("message", "allow_enable_remote_control"),
                wui::locale("message", "title_confirmation"),
                wui::message_icon::alert,
                wui::message_button::yes_no, [this, actorId](wui::message_result result) {
                actionQuested = false;

                controller.SendMemberActionResult(actorId, result == wui::message_result::yes ?
                    Proto::MEMBER_ACTION::Result::Accepted : Proto::MEMBER_ACTION::Result::Rejected);

                if (result == wui::message_result::yes)
                {
                    demonstrationSession->SetRCActions(true);
                }
            });
        }
    }
}

void MainFrame::ActionHand(bool my)
{
    uint32_t grants = controller.GetCurrentConference().grants;

    bool isMeReadOnly = memberList.IsMeReadOnly();
    bool denyTurnSpeak = BitIsSet(grants, static_cast<int32_t>(Proto::ConferenceGrants::DenyTurnSpeak)) || isMeReadOnly;
    if (my && denyTurnSpeak && !memberList.IsMeSpeaker() && !memberList.IsMePresenter() && !memberList.IsMeModerator())
    {
        controller.SendWantSpeak();
        return messageBox->show(wui::locale("message", "turning_speaker_denied"),
            wui::locale("message", "title_notification"),
            wui::message_icon::alert,
            wui::message_button::ok);
    }

    bool disableCamerasIfNoSpeaker = BitIsSet(grants, static_cast<int32_t>(Proto::ConferenceGrants::DisableCameraIfNoSpeak)) || isMeReadOnly;
    bool disableMicrophonesIfNoSpeaker = BitIsSet(grants, static_cast<int32_t>(Proto::ConferenceGrants::DisableMicrophoneIfNoSpeak)) || isMeReadOnly;

    memberList.TurnMeSpeaker();
    controller.SendTurnSpeaker();

    if (disableCamerasIfNoSpeaker)
    {
        auto cameraEnabled = IsCameraEnabled();
        if (!memberList.IsMeSpeaker() && cameraEnabled)
        {
            mainToolBar.EnableCamera(false);
            wui::config::set_int("CaptureDevices", "CameraEnabled", 0);
            StopCamera();
        }
        else if (memberList.IsMeSpeaker() && !cameraEnabled)
        {
            mainToolBar.EnableCamera(true);
            wui::config::set_int("CaptureDevices", "CameraEnabled", 1);
            StartCamera();
        }
    }
    if (disableMicrophonesIfNoSpeaker)
    {
        auto microphoneEnabled = IsMicrophoneEnabled();
        if (!memberList.IsMeSpeaker() && microphoneEnabled)
        {
            mainToolBar.EnableMicrophone(false);
            wui::config::set_int("CaptureDevices", "MicrophoneEnabled", 0);
            StopMicrophone();
        }
        else if (memberList.IsMeSpeaker() && !microphoneEnabled)
        {
            mainToolBar.EnableMicrophone(true);
            wui::config::set_int("CaptureDevices", "MicrophoneEnabled", 1);
            StartMicrophone();
        }
    }
}

void MainFrame::ActionTurnListPanel()
{
    auto enabled = wui::config::get_int("ListPanel", "Enabled", 1) != 0;

    enabled = !enabled;

    mainToolBar.EnableList(enabled);

    wui::config::set_int("ListPanel", "Enabled", enabled ? 1 : 0);

    if (enabled)
    {
        listPanel.Run();
        if (listPanel.Pinned())
        {
            listPanel.UpdateTop(mainToolBar.Bottom());
        }

        UpdateVideoRendererPosition();
        UpdateContentPanelLeft();
    }
    else
    {
        listPanel.End();
    }
}

void MainFrame::ActionTurnContentPanel()
{
    auto enabled = wui::config::get_int("ContentPanel", "Enabled", 1) != 0;

    enabled = !enabled;

    mainToolBar.EnableContent(enabled);

    wui::config::set_int("ContentPanel", "Enabled", enabled ? 1 : 0);

    if (enabled)
    {
        contentPanel.Run();
        if (contentPanel.Pinned())
        {
            contentPanel.UpdateTop(mainToolBar.Bottom());
        }

        UpdateVideoRendererPosition();
        UpdateContentPanelLeft();
    }
    else
    {
        contentPanel.End();
    }
}

void MainFrame::ActionTurnRecord()
{
    int enabled = 1 - wui::config::get_int("Record", "Enabled", 0);
    wui::config::set_int("Record", "Enabled", enabled);

    mainMenu->update_item({ 2, wui::menu_item_state::separator, wui::locale("menu", enabled ? "disable_record" : "enable_record"), "Ctrl+R", nullptr, {}, [this](int32_t i) { ActionTurnRecord(); } });

    if (controller.GetState() == Controller::State::Conferencing)
    {
        if (enabled)
        {
            if (IsRecordAllowed())
            {
                recorder.Start(GetRecordName().c_str(), wui::config::get_int("Record", "MP3Mode", 0) != 0);
            }
        }
        else
        {
            recorder.Stop();
        }
    }
}

void MainFrame::ActionFullScreen()
{
    if (window->state() != wui::window_state::maximized)
    {
        window->expand();
    }
    else
    {
        window->set_style(wui::flags_map<wui::window_style>(2, wui::window_style::frame, wui::window_style::switch_theme_button));
        window->normal();
    }
}

void MainFrame::ActionMenu()
{
    mainMenu->show_on_control(mainToolBar.GetMenuControl(), 4);
}

/// IContactListCallback impl
void MainFrame::ContactSelected(int64_t id, std::string_view name)
{
    contentPanel.SetUser(id, name);
}

void MainFrame::ContactCall(std::string_view name)
{
    Call(name);
}

void MainFrame::ConferenceSelected(std::string_view tag, std::string_view name)
{
    contentPanel.SetConference(tag, name);
}

void MainFrame::ConferenceConnect(std::string_view tag, bool my)
{
    ConnectToConference(tag, my);
}

void MainFrame::ContactUnselected()
{
}

/// ListPanelCallback impl
void MainFrame::ListPanelMembersSelected()
{

}

void MainFrame::ListPanelClosed()
{
    mainToolBar.EnableList(false);
    wui::config::set_int("ListPanel", "Enabled", 0);

    UpdateVideoRendererPosition();
    UpdateContentPanelLeft();
}

void MainFrame::ListPanelPinChanged()
{
    if (listPanel.Pinned())
    {
        listPanel.UpdateTop(mainToolBar.Bottom());
    }

    UpdateVideoRendererPosition();
    UpdateContentPanelLeft();
}

void MainFrame::ListPanelWidthChanged(int32_t width)
{
    UpdateVideoRendererPosition();
    UpdateContentPanelLeft();
}

void MainFrame::ContentPanelClosed()
{
    mainToolBar.EnableContent(false);
    wui::config::set_int("ContentPanel", "Enabled", 0);
    
    UpdateVideoRendererPosition();
}

void MainFrame::ContentPanelPinChanged()
{
    if (contentPanel.Pinned())
    {
        contentPanel.UpdateTop(mainToolBar.Bottom());
        UpdateContentPanelLeft();
    }

    UpdateVideoRendererPosition();
}

void MainFrame::ContentPanelWidthChanged(int32_t width)
{
    UpdateVideoRendererPosition();
}

void MainFrame::ReceiveEvents(const wui::event &ev)
{
    switch (ev.type)
    {
        case wui::event_type::internal:
        {
            switch (ev.internal_event_.type)
            {
                case wui::internal_event_type::window_created:
                    Init();
                break;
                case wui::internal_event_type::size_changed:
                {
                    if (window->state() == wui::window_state::normal &&
                        ev.internal_event_.x > 0 && ev.internal_event_.y > 0)
                    {
                        wui::config::set_int("MainFrame", "Width", ev.internal_event_.x);
                        wui::config::set_int("MainFrame", "Height", ev.internal_event_.y);
                    }

                    miniWindow.End();

                    mainToolBar.UpdateWidth(ev.internal_event_.x);
                    listPanel.UpdateHeight(ev.internal_event_.y);
                    contentPanel.UpdateSize(ev.internal_event_.x, ev.internal_event_.y);
                    contentPanel.ScrollToEnd();
                    
                    renderersBox.SetPosition({ listPanel.Right(), mainToolBar.Bottom(), contentPanel.Left(), ev.internal_event_.y });
                    timerBar.UpdatePosition(listPanel.Right(), mainToolBar.Bottom());
                }
                break;
                case wui::internal_event_type::window_expanded:
                    mainToolBar.UpdateWidth(ev.internal_event_.x);

                    if (!controller.GetCurrentConference().tag.empty())
                    {
                        mainToolBar.FullScreen();
                        
                        listPanel.UpdateTop(0);
                        contentPanel.UpdateTop(0);
                        listPanel.UpdateHeight(ev.internal_event_.y);
                        contentPanel.UpdateSize(ev.internal_event_.x, ev.internal_event_.y);

                        renderersBox.SetPosition({ listPanel.Right(), 0, contentPanel.Left(), ev.internal_event_.y });
                        timerBar.UpdatePosition(listPanel.Right(), mainToolBar.Bottom());
                    }
                    else
                    {
                        listPanel.UpdateHeight(ev.internal_event_.y);
                        contentPanel.UpdateSize(ev.internal_event_.x, ev.internal_event_.y);
                    }
                break;
                case wui::internal_event_type::window_normalized:
                    mainToolBar.Normalize();

                    listPanel.UpdateTop(mainToolBar.Bottom());
                    contentPanel.UpdateTop(mainToolBar.Bottom());
                    listPanel.UpdateHeight(ev.internal_event_.y);
                    contentPanel.UpdateSize(ev.internal_event_.x, ev.internal_event_.y);
                    
                    renderersBox.SetPosition({ listPanel.Right(), mainToolBar.Bottom(), contentPanel.Left(), ev.internal_event_.y });
                    timerBar.UpdatePosition(listPanel.Right(), mainToolBar.Bottom());

                    wui::show_taskbar_icon(window->context());
                break;
                case wui::internal_event_type::window_minimized:
                    if (controller.GetState() == Controller::State::Conferencing && wui::config::get_int("User", "ShowMiniWindow", 1) != 0)
                    {
                        miniWindow.Run();
                        renderersBox.Update();
                    }
                break;
                case wui::internal_event_type::user_emitted:
                    switch (static_cast<MyEvent>(ev.internal_event_.x))
                    {
                        case MyEvent::Controller:
                            ProcessControllerEvent();
                        break;
                        case MyEvent::ShowBusy:
                            busyBox.Run(busyTitle);
                        break;
                        case MyEvent::SetBusyProgress:
                            busyBox.SetProgress(busySubTitle, ev.internal_event_.y);
                        break;
                        case MyEvent::SetMainProgress:
                            UpdateTitle(busySubTitle, ev.internal_event_.y);
                        break;
                        case MyEvent::HideBusy:
                            busyBox.End();
                        break;
                        case MyEvent::CredentialsReady:
                            controller.SetCredentials(credentialsDialog.login, credentialsDialog.password);

                            UpdateTitle();
                            if (controller.Connected())
                            {
                                controller.Disconnect();
                            }
                            storage.Connect(GetAppDataPath() + "local.db");
                        break;
                        case MyEvent::NewCredentials:
                            controller.SetCredentials(credentialsDialog.login, credentialsDialog.password);

                            UpdateTitle();
                            controller.Connect(wui::config::get_string("Connection", "Address", ""), wui::config::get_int("Connection", "Secure", 0) != 0);
                            storage.Connect(GetAppDataPath() + "local.db");
                        break;
                        case MyEvent::SpeedTestCompleted:
                            UpdateTitle();
                        break;
                        case MyEvent::ConnectivityTestCompleted:
                            if (showConnectivityResult)
                            {
                                if (udpTester.TestPassed())
                                {
                                    messageBox->show(wui::locale("net_test", "connectivity_ok"),
                                        wui::locale("message", "title_notification"),
                                        wui::message_icon::information,
                                        wui::message_button::ok);
                                }
                                else
                                {
                                    messageBox->show(wui::locale("net_test", "connectivity_tcp_only"),
                                        wui::locale("message", "title_notification"),
                                        wui::message_icon::alert,
                                        wui::message_button::ok);
                                }
                            }
                            
                            UpdateTitle();
                        break;
                        case MyEvent::RingerEnd:
                            switch (static_cast<Ringer::RingType>(ev.internal_event_.y))
                            {
                                case Ringer::RingType::CallIn:
                                    questionDialog.missed = wui::config::get_int("User", "CallAutoAnswer", 0) == 0;
                                    questionDialog.End(!questionDialog.missed);
                                break;
                                case Ringer::RingType::CallOut:
                                    CancelCall();
                                break;
                                case Ringer::RingType::ScheduleConnectQuick:
                                    HideBusy();
                                    ConnectToConference(callParams.scheduleConferenceTag, false);
                                break;
                                case Ringer::RingType::ScheduleConnectLong:
                                    questionDialog.End(false);
                                    callParams.scheduleConferenceTag.clear();
                                break;
                            }
                        break;
                        case MyEvent::Normalize:
                            miniWindow.End();
                            window->normal();
                        break;
                    }
                break;
            }
        }
        break;
        case wui::event_type::system:
            switch (ev.system_event_.type)
            {
                case wui::system_event_type::device_change:
                    UpdateConnectedCameras();
                    UpdateConnectedMicrophones();
                    UpdateConnectedAudioRenderers();
                break;
            }
        break;
        case wui::event_type::keyboard:
            if (ev.keyboard_event_.type == wui::keyboard_event_type::up && ev.keyboard_event_.key[0] == wui::vk_esc)
            {
                ActionFullScreen();
            }
        break;
    }
}

bool MainFrame::GetEventFromQueue(Controller::Event &ev)
{
    //std::lock_guard<std::mutex> lock(controllerEventsQueueMutex);
    if (!controllerEventsQueue.empty())
    {
        ev = controllerEventsQueue.front();
        controllerEventsQueue.pop();
        return true;
    }
    return false;
}

void MainFrame::ProcessControllerEvent()
{
    Controller::Event e;
    while (GetEventFromQueue(e))
    {
        switch (e.type)
        {
            /// Connection
            case Controller::Event::Type::LogonSuccess:
            {
                sysLog->info("Logon success");

                trayIcon->change_icon(ICO_ACTIVE);
                trayIcon->change_tip(wui::locale("client", "title") + "(" + wui::locale("common", "online") + ")");

                SetStanbyMode();

                useWSMedia = false;

                DetermineNetSpeed();
                CheckConnectivity();

                storage.SetMyClientId(controller.GetMyClientId());

                controller.LoadMessages(storage.GetLastMessageDT());
                controller.DeliveryMessages(storage.GetUndeliveredMessages());

                UpdateTitle();

                if (controller.GetMaxOutputBitrate() != 0)
                {
                    if (wui::config::get_int("User", "MaxOutputBitrate", 2048) > (int32_t)controller.GetMaxOutputBitrate())
                    {
                        wui::config::set_int("User", "MaxOutputBitrate", controller.GetMaxOutputBitrate());
                    }
                }

                online = true;

                controller.UpdateGroupList();
                controller.UpdateContactList();
                controller.UpdateConferencesList();

                if (wui::config::get_int("User", "FirstRun", 1) != 0)
                {
                    wui::config::set_int("User", "FirstRun", 0);
                    //ActionDevices();
                }

                CheckVGProtocol();

                ConnectToURIConference();
            }
            break;
            case Controller::Event::Type::NetworkError:
                controller.Disconnect();
                UpdateTitle();
            break;
            case Controller::Event::Type::Disconnected:
                if (online)
                {
                    errLog->warn("Client is disconnected");

                    UpdateTitle(wui::locale("common", "offline"));
                    trayIcon->change_icon(ICO_INACTIVE);
                    trayIcon->change_tip(wui::locale("client", "title") + "(" + wui::locale("common", "offline") + ")");

                    storage.SetMyClientId(0);

                    online = false;
                    useWSMedia = false;

                    DisconnectFromConference();
                    renderersBox.Update();
                }

                UpdateTitle();

                if (working)
                {
                    controller.Connect(wui::config::get_string("Connection", "Address", ""), wui::config::get_int("Connection", "Secure", 0) != 0);
                }
            break;
            case Controller::Event::Type::ServerChanged:
                errLog->error("Server changed to {0}, secure: {0:d}", e.data, e.iData);

                wui::config::set_string("Connection", "Address", e.data);
                wui::config::set_int("Connection", "Secure", static_cast<int32_t>(e.iData));
            break;
            case Controller::Event::Type::GrantsUpdated:
                sysLog->info("Grants updated");
            break;
            case Controller::Event::Type::AuthNeeded:
            {
                sysLog->info("Auth needed");

                UpdateTitle();

                if (e.iData == 0)
                {
                    credentialsDialog.Run();
                }
                else if (e.iData == 1)
                {
                    messageBox->show(wui::locale("message", "invalid_credentionals"),
                        wui::locale("message", "title_error"),
                        wui::message_icon::alert,
                        wui::message_button::ok,
                        [this](wui::message_result) { credentialsDialog.Run(); });
                }
                else if (e.iData == 2)
                {
                    /// Don't show message in run with no remember credentionals
                    if (!wui::config::get_string("User", "Name", "").empty())
                    {
                        AutoRegisterUser(wui::config::get_string("User", "Name", ""));
                        wui::config::delete_value("User", "Name");
                    }
                    else
                    {
                        credentialsDialog.Run();
                    }
                }
            }
            break;
            case Controller::Event::Type::UpdateRTPAddress:
                udpTester.AddAddress(e.data.c_str(), static_cast<int16_t>(e.iData));
            break;
            case Controller::Event::Type::ReadyToMakeMediaTest:
                udpTester.DoTheTest();
            break;

            /// Updating
            case Controller::Event::Type::UpdateRequired:
                sysLog->info("Update required");
                
                UpdateTitle();
                
                ShowBusy(wui::locale("common", "updating"));
#ifdef _WIN32
                controller.Update(SYSTEM_NAME "Client.exe", "\\IVS\\" SYSTEM_NAME "\\");
#else
                controller.Update(SYSTEM_NAME "Client");
#endif
            break;
            case Controller::Event::Type::Updated:
                sysLog->info("Updated success");
                HideBusy();
                userForceClose = true;
                window->destroy();
            break;

            /// Server errors
            case Controller::Event::Type::ServerOutdated:
                errLog->error("Server outdated");
                messageBox->show(wui::locale("message", "server_outdated"),
                    wui::locale("message", "title_error"),
                    wui::message_icon::alert,
                    wui::message_button::ok,
                    [this](wui::message_result) { 
                        userForceClose = true;
                        window->destroy();
                    });
            break;
            case Controller::Event::Type::ServerFull:
                errLog->error("Server full");
                messageBox->show(wui::locale("message", "server_full"),
                    wui::locale("message", "title_error"),
                    wui::message_icon::alert,
                    wui::message_button::ok,
                    [this](wui::message_result) { 
                        userForceClose = true;
                        window->destroy();
                    });
            break;
            case Controller::Event::Type::ServerInternalError:
            {
                errLog->error("Server internal error {0}", e.data);

                UpdateTitle();

                HideBusy();

                messageBox->show(wui::locale("message", "internal_server_error") + ": " + e.data,
                    wui::locale("message", "title_error"),
                    wui::message_icon::alert,
                    wui::message_button::ok);
            }
            break;

            /// User updating
            case Controller::Event::Type::UserUpdateResponse:
                switch (static_cast<Proto::USER_UPDATE_RESPONSE::Result>(e.iData))
                {
                    case Proto::USER_UPDATE_RESPONSE::Result::OK:
                    {
                        auto action = static_cast<Proto::USER_UPDATE_REQUEST::Action>(e.deviceValues.pos);

                        switch (action)
                        {
                            case Proto::USER_UPDATE_REQUEST::Action::Register:
                            case Proto::USER_UPDATE_REQUEST::Action::ChangeMeta:
                                wui::config::set_string("Credentials", "Login", settingsDialog.GetLogin());
                                wui::config::set_string("Credentials", "Password", settingsDialog.GetPassword());

                                UpdateTitle();
                            break;
                        }
                    }
                break;
                case Proto::USER_UPDATE_RESPONSE::Result::DuplicateName:
                    settingsDialog.ResetUserName();
                    messageBox->show(wui::locale("message", "duplicate_name"),
                        wui::locale("message", "title_error"),
                        wui::message_icon::alert,
                        wui::message_button::ok, [this](wui::message_result) {
                            settingsDialog.Run(SettingsSection::Account);
                    });
                break;
                case Proto::USER_UPDATE_RESPONSE::Result::DuplicateLogin:
                    settingsDialog.ResetUserLogin();
                    messageBox->show(wui::locale("message", "duplicate_credentionals"),
                        wui::locale("message", "title_error"),
                        wui::message_icon::alert,
                        wui::message_button::ok, [this](wui::message_result) {
                            settingsDialog.Run(SettingsSection::Account);
                    });
                break;
            }
            break;

            /// Devices
            case Controller::Event::Type::CameraCreated:
            {
                const auto &device = e.deviceValues;

                int bitrate = wui::config::get_int("CaptureDevices", "CameraBitrate", 320);

                auto cvs = std::make_shared<CaptureSession::CaptureVideoSession>(timeMeter);

                cvs->SetDeviceType(device.type);
                cvs->SetName(device.name);
                cvs->SetResolution(device.resolution);
                cvs->SetBitrate(bitrate);
                cvs->SetDeviceNotifyCallback(std::bind(&MainFrame::ReceiveDeviceNotify, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));

                if (device.type == Proto::DeviceType::Camera)
                {
                    captureVideoSession = cvs;
                }
                else if (device.type == Proto::DeviceType::Demonstration)
                {
                    demonstrationSession = cvs;
                    cvs->SetFrameRate(5);
                }

                if (wui::config::get_int("User", "DontUpdateBitrates", 0) == 0)
                {
                    UpdateOutputBitrates();
                }

                controller.ConnectCapturer(device);
            }
            break;
            case Controller::Event::Type::CameraStarted:
            {
                auto cvs = GetCaptureVideoSession(e.deviceValues.type);
                if (cvs)
                {
                    const auto &device = e.deviceValues;

                    cvs->SetRTPParams(!useWSMedia ? device.addr.c_str() : "127.0.0.1", !useWSMedia ? device.port : tcpClient.CreatePipe(device.port));
                    cvs->Start(device.authorSSRC, device.colorSpace, device.deviceId, device.secureKey);
                }
            }
            break;
            case Controller::Event::Type::MicrophoneCreated:
            {
                const Controller::DeviceValues &device = e.deviceValues;

                int32_t bitrate = wui::config::get_int("CaptureDevices", "MicrophoneBitrate", 30);
                uint16_t sensitivity = wui::config::get_int("CaptureDevices", "MicrophoneGain", 80);
                bool aec = wui::config::get_int("CaptureDevices", "MicrophoneAEC", 1) != 0;
                bool ns = wui::config::get_int("CaptureDevices", "MicrophoneNS", 1) != 0;
                bool agc = wui::config::get_int("CaptureDevices", "MicrophoneAGC", 1) != 0;

                int quality = 10;
                auto flops = wui::config::get_int("User", "Flops", 0);
                if (flops < 99000000)
                {
                    quality = 0;
                }
                else if (flops > 99000000)
                {
                    quality = 5;
                }

                captureAudioSession = std::make_shared<CaptureSession::CaptureAudioSession>(timeMeter);
                captureAudioSession->SetDeviceName(device.name);
                captureAudioSession->SetAudioRenderer(&audioRenderer);
                captureAudioSession->EnableAEC(aec);
                captureAudioSession->EnableNS(ns);
                captureAudioSession->EnableAGC(agc);
                captureAudioSession->SetQuality(quality);
                captureAudioSession->SetBitrate(bitrate);
                captureAudioSession->SetGain(sensitivity);
                captureAudioSession->SetRenderLatency(audioRenderer.GetLatency());
                captureAudioSession->SetDeviceNotifyCallback(std::bind(&MainFrame::ReceiveDeviceNotify, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));

                if (wui::config::get_int("User", "DontUpdateBitrates", 0) == 0)
                {
                    UpdateOutputBitrates();
                }

                controller.ConnectCapturer(device);
            }
            break;
            case Controller::Event::Type::MicrophoneStarted:
            {
                if (captureAudioSession)
                {
                    auto &device = e.deviceValues;
                    captureAudioSession->SetRTPParams(!useWSMedia ? device.addr.c_str() : "127.0.0.1", !useWSMedia ? device.port : tcpClient.CreatePipe(device.port));
                    captureAudioSession->Start(device.authorSSRC, device.deviceId, device.secureKey);
                }
            }
            break;
            case Controller::Event::Type::CameraError:
            {
                errLog->warn("Camera {0} start error, hresult: {1:x}", e.deviceValues.name, e.iData);

                StopCamera();
                mainToolBar.EnableCamera(false);

                messageBox->show(wui::locale("message", "camera_busy_another_app_error"),
                    wui::locale("message", "title_error"),
                    wui::message_icon::alert,
                    wui::message_button::ok);
            }
            break;
            case Controller::Event::Type::OvertimeCoding:
            {
                if (e.deviceValues.type != Proto::DeviceType::Demonstration && wui::config::get_int("User", "DisableDownResolution", 0) == 0)
                {
                    std::vector<Video::VideoFormat> *formats = nullptr;
                    for (auto &camera : cameraDevices)
                    {
                        if (camera.name == e.deviceValues.name)
                        {
                            formats = &camera.formats;
                            break;
                        }
                    }

                    auto cvs = GetCaptureVideoSession(e.deviceValues.type);
                    if (cvs)
                    {
                        auto rv = Video::GetValues(cvs->GetResolution());

                        if (formats == nullptr || rv.height == 120)
                        {
                            StopCamera();
                            mainToolBar.EnableCamera(false);

                            return messageBox->show(wui::locale("message", "camera_disabled_lacks_performance"),
                                wui::locale("message", "title_error"),
                                wui::message_icon::alert,
                                wui::message_button::ok);
                        }

                        uint32_t height = 480;
                        if (rv.height > 720)
                        {
                            height = 720;
                        }
                        if (rv.height <= 720 && rv.height > 480)
                        {
                            height = 480;
                        }
                        else if (rv.height <= 480 && rv.height > 240)
                        {
                            height = 240;
                        }
                        else if (rv.height <= 240 && rv.height > 120)
                        {
                            height = 120;
                        }

                        auto resolution = GetBestResolution(*formats, height);
                        controller.ChangeResolution(captureVideoSession->GetDeviceId(), resolution);
                        captureVideoSession->SetResolution(resolution);
                        UpdateOutputBitrates();
                    }
                }
            }
            break;
            case Controller::Event::Type::OvertimeRendering:
            {
                auto rvs = GetRendererVideoSession(e.deviceValues.deviceId);
                if (rvs)
                {
                    controller.DisconnectRenderer(rvs->GetDeviceId(), rvs->GetSSRC());
                    DeleteVideoRenderer(rvs->GetDeviceId(), rvs->GetClientId(), e.deviceValues.type);
                }

                return messageBox->show(wui::locale("message", "videorenderer_disabled_lacks_performance"),
                    wui::locale("message", "title_error"),
                    wui::message_icon::alert,
                    wui::message_button::ok);
            }
            break;
            case Controller::Event::Type::MicrophoneError:
            {
                errLog->warn("Microphone {0} start error, result: {1:x}", e.deviceValues.name, e.iData);

                StopMicrophone();
                mainToolBar.EnableMicrophone(false);

                messageBox->show(wui::locale("message", "microphone_start_error"),
                    wui::locale("message", "title_error"),
                    wui::message_icon::alert,
                    wui::message_button::ok);
            }
            break;
            case Controller::Event::Type::DeviceEnded:
            {
                errLog->warn("Device {0} ended", e.deviceValues.name);

                switch (e.deviceValues.type)
                {
                    case Proto::DeviceType::Camera:
                        StopCamera();
                        mainToolBar.EnableCamera(false);
                    break;
                    case Proto::DeviceType::Demonstration:
                        StopDemonstration();
                        mainToolBar.EnableScreenCapturer(false);
                    break;
                    case Proto::DeviceType::Microphone:
                        StopMicrophone();
                        mainToolBar.EnableMicrophone(false);
                    break;
                }
            }
            break;
            case Controller::Event::Type::MemoryError:
            {
                std::string msg;

                switch (e.deviceValues.type)
                {
                    case Proto::DeviceType::Camera:
                        StopCamera();
                        mainToolBar.EnableCamera(false);
                        msg = "camera_start_error_no_memory";
                    break;
                    case Proto::DeviceType::Demonstration:
                        StopDemonstration();
                        mainToolBar.EnableScreenCapturer(false);
                        msg = "screen_capture_start_error_no_memory";
                    break;
                    case Proto::DeviceType::Microphone:
                        StopMicrophone();
                        mainToolBar.EnableMicrophone(false);
                        msg = "microphone_start_error_no_memory";
                    break;
                    case Proto::DeviceType::VideoRenderer:
                    {
                        int64_t clientId = 0;
                        for (auto &rvs : renderersVideo)
                        {
                            if (rvs->GetDeviceId() == e.deviceValues.deviceId)
                            {
                                clientId = rvs->GetClientId();
                                break;
                            }
                        }
                        DeleteVideoRenderer(e.deviceValues.deviceId, clientId, e.deviceValues.type);

                        msg = "videorenderer_start_error_no_memory";
                    }
                    break;
                    case Proto::DeviceType::AudioRenderer:
                    {
                        auto clientId = GetRendererAudioSession(e.deviceValues.deviceId)->GetClientId();
                        DeleteAudioRenderer(e.deviceValues.deviceId, clientId);
                        msg = "audiorenderer_start_error_no_memory";
                    }
                    break;
                }

                messageBox->show(wui::locale("message", msg),
                    wui::locale("message", "title_error"),
                    wui::message_icon::alert,
                    wui::message_button::ok);
            }
            break;
            case Controller::Event::Type::DeviceResolutionChanged:
            {
                auto cvs = GetCaptureVideoSession(e.deviceValues.type);
                if (cvs)
                {
                    controller.ChangeResolution(cvs->GetDeviceId(), static_cast<Video::Resolution>(e.iData));
                    cvs->SetResolution(static_cast<Video::Resolution>(e.iData));
                }
            }
            break;
            case Controller::Event::Type::MicrophoneSpeak:
            {
                if (captureAudioSession)
                {
                    controller.MicrophoneActive(captureAudioSession->GetDeviceId(), Proto::MICROPHONE_ACTIVE::ActiveType::Speak);
                }
            }
            break;
            case Controller::Event::Type::MicrophoneSilent:
            {
                if (captureAudioSession)
                {
                    controller.MicrophoneActive(captureAudioSession->GetDeviceId(), Proto::MICROPHONE_ACTIVE::ActiveType::Silent);
                }
            }
            break;

            /// Conference
            case Controller::Event::Type::ConferenceConnected:
            {
                sysLog->info("Conference connected");

                ringer.Ring(Ringer::RingType::Dial);

                const auto &cc = controller.GetCurrentConference();

                if (wui::config::get_int("Record", "Enabled", 0) != 0 && IsRecordAllowed())
                {
                    recorder.Start(GetRecordName().c_str(), wui::config::get_int("Record", "MP3Mode", 0) != 0);
                }

                timeMeter.Reset();

                if (CheckInputBitrateTooSmall())
                {
                    messageBox->show(wui::locale("message", "bitrate_too_small_to_show_video"),
                        wui::locale("message", "title_error"),
                        wui::message_icon::alert,
                        wui::message_button::ok);
                }

                bool outputBitrateTooSmall = CheckOutputBitrateTooSmall();
                if (outputBitrateTooSmall ||
                    (BitIsSet(cc.grants, static_cast<int32_t>(Proto::ConferenceGrants::DisableCameraIfNoSpeak)) && !memberList.IsMePresenter() && !memberList.IsMeSpeaker()) ||
                    memberList.IsMeReadOnly())
                {
                    miniWindow.SetCameraState(false);
                    if (IsCameraEnabled())
                    {
                        mainToolBar.EnableCamera(false);
                        wui::config::set_int("CaptureDevices", "CameraEnabled", 0);
                        wui::config::set_int("User", "EnableCamOnEndConference", 1);
                    }

                    if (outputBitrateTooSmall)
                    {
                        messageBox->show(wui::locale("message", "bitrate_too_small_to_start_camera"),
                            wui::locale("message", "title_error"),
                            wui::message_icon::alert,
                            wui::message_button::ok);
                    }
                }
                else
                {
                    StartCamera();
                }
                if ((BitIsSet(cc.grants, static_cast<int32_t>(Proto::ConferenceGrants::DisableMicrophoneIfNoSpeak)) && !memberList.IsMePresenter() && !memberList.IsMeSpeaker()) ||
                    memberList.IsMeReadOnly())
                {
                    miniWindow.SetMicrophoneState(false);
                    if (IsMicrophoneEnabled())
                    {
                        mainToolBar.EnableMicrophone(false);
                        wui::config::set_int("CaptureDevices", "MicrophoneEnabled", 0);
                        wui::config::set_int("User", "EnableMicOnEndConference", 1);
                    }
                }
                else
                {
                    StartMicrophone();
                }

                UpdateTitle();
                
                //killScreenSaverTimer.start(30000);

                callParams.scheduleConferenceTag.clear();

                SetConferencingMode();

                if (wui::config::get_int("User", "ShowTimer", 1) != 0 || callParams.timeLimit != 0)
                {
                    timerBar.Run(callParams.timeLimit, listPanel.Right(), mainToolBar.Bottom());
                }
            }
            break;
            case Controller::Event::Type::DisconnectedFromConference:
            {
                sysLog->info("Disconnected from conference");

                renderersBox.Update();

                //killScreenSaverTimer.stop();

                timerBar.End();

                SetStanbyMode();

                callParams.Clear();

                memberList.ClearItems();

                tcpClient.EndSession();

                mainToolBar.EnableScreenCapturer(false);

                if (wui::config::get_int("User", "EnableCamOnEndConference", 0) != 0 && !IsCameraEnabled())
                {
                    ActionTurnCamera();

                    wui::config::set_int("User", "EnableCamOnEndConference", 0);
                }

                if (wui::config::get_int("User", "EnableMicOnEndConference", 0) != 0 && !IsMicrophoneEnabled())
                {
                    ActionTurnMicrophone();

                    wui::config::set_int("User", "EnableMicOnEndConference", 0);
                }

                if (callParams.currentConferenceOneTime)
                {
                    messageBox->show(wui::locale("message", "ask_delete_one_time_conference"),
                        wui::locale("message", "title_notification"),
                        wui::message_icon::alert,
                        wui::message_button::ok, [this, &e](wui::message_result) { controller.DeleteConference(static_cast<int32_t>(e.iData)); });
                }
                callParams.currentConferenceOneTime = false;

                UpdateTitle();

                miniWindow.End();
            }
            break;
            case Controller::Event::Type::ConferenceNotExists:
                errLog->warn("Conference not exists");
                messageBox->show(wui::locale("message", "connecting_to_non_existent_conference"),
                    wui::locale("message", "title_error"),
                    wui::message_icon::alert,
                    wui::message_button::ok);
            break;
            case Controller::Event::Type::ConferenceNotAllowed:
                errLog->warn("Conference not allowed");
                messageBox->show(wui::locale("message", "connect_to_conference_denied"),
                    wui::locale("message", "title_error"),
                    wui::message_icon::alert,
                    wui::message_button::ok);
            break;
            case Controller::Event::Type::ConferenceLicenseFull:
                errLog->warn("Conference not connected, because license is full");
                messageBox->show(wui::locale("message", "connect_to_conference_license_full"),
                    wui::locale("message", "title_error"),
                    wui::message_icon::alert,
                    wui::message_button::ok);
            break;
            case Controller::Event::Type::DisallowedActionInThisConference:
                messageBox->show(wui::locale("message", "managing_conference_denied"),
                    wui::locale("message", "title_error"),
                    wui::message_icon::alert,
                    wui::message_button::ok);
            break;
            case Controller::Event::Type::RejectedAction:
                renderersBox.EnableRC(e.iData, false);
                messageBox->show(wui::locale("common", "user") + " " + e.data + " " + wui::locale("message", "action_rejected"),
                    wui::locale("message", "title_error"),
                    wui::message_icon::alert,
                    wui::message_button::ok);
            break;
            case Controller::Event::Type::AcceptedAction:
                renderersBox.EnableRC(e.iData, true);
            break;
            case Controller::Event::Type::BusyAction:
                messageBox->show(wui::locale("common", "user") + " " + e.data + " " + wui::locale("message", "action_busy"),
                    wui::locale("message", "title_error"),
                    wui::message_icon::alert,
                    wui::message_button::ok);
            break;

            case Controller::Event::Type::MemberAdded:
                if (e.deviceValues.disabled)
                {
                    ShowAvatar(e.iData, e.data);
                }
                memberList.Update();
            break;
            case Controller::Event::Type::MemberRemoved:
                HideAvatar(e.iData);
                memberList.Update();
            break;

            /// Calling
            case Controller::Event::Type::CallRequest:
            {
                sysLog->info("Call request from {0}, type {1}", e.callValues.name, Proto::CALL_REQUEST::str(e.callValues.requestType));
                if (e.callValues.requestType == Proto::CALL_REQUEST::Type::Invocation)
                {
                    if (controller.GetState() != Controller::State::Conferencing && !questionDialog.IsInQuestion())
                    {
                        KillScreenSaver();

                        ringer.Ring(Ringer::RingType::CallIn);

                        questionDialog.Run(wui::locale("message", "call_from") + " " + e.callValues.name + "\n" + wui::locale("message", "call_accept"));

                        callParams.subscriberId = e.callValues.subscriberId;
                        callParams.subscriberConnectionId = e.callValues.subscriberConnectionId;
                        callParams.subscriberName = e.callValues.name;
                    }
                    else
                    {
                        controller.CallResponse(Controller::CallValues(Proto::CALL_RESPONSE::Type::Busy, e.callValues.subscriberId, e.callValues.subscriberConnectionId, e.callValues.name));
                        messageBox->show(e.callValues.name + " " + wui::locale("message", "called_you"),
                            wui::locale("message", "title_notification"),
                            wui::message_icon::information,
                            wui::message_button::ok);

                        callParams.timeLimit = e.callValues.timeLimit;
                    }
                }
                else // Cancel bugging user
                {
                    callParams.Clear();
                    ringer.Stop();

                    questionDialog.End(false);
                }
            }
            break;
            case Controller::Event::Type::CallResponse:
            {
                sysLog->info("Call response {0:d}", static_cast<int32_t>(e.callValues.responseType));
                switch (e.callValues.responseType)
                {
                    case Proto::CALL_RESPONSE::Type::Accept:
                        RestoreAfterCall();
                        callParams.subscriberId = e.callValues.subscriberId;
                        callParams.subscriberConnectionId = e.callValues.subscriberConnectionId;
                        e.callValues.name = e.callValues.name;
                        if (controller.GetState() != Controller::State::Conferencing)
                        {
                            controller.CreateTempConference();
                        }
                        else
                        {
                            controller.SendConnectToConference(controller.GetCurrentConference().tag, callParams.subscriberId, callParams.subscriberConnectionId, static_cast<uint32_t>(Proto::SEND_CONNECT_TO_CONFERENCE::Flag::InviteCall));
                        }
                        callParams.timeLimit = e.callValues.timeLimit;
                    break;
                    case Proto::CALL_RESPONSE::Type::Refuse:
                    {
                        auto prevCallingSubscriber = callParams.callingSubscriber;
                        RestoreAfterCall();
                        if (!prevCallingSubscriber.empty())
                        {
                            messageBox->show(wui::locale("message", "subscriber_reject"),
                                wui::locale("message", "title_notification"),
                                wui::message_icon::alert,
                                wui::message_button::ok);
                        }
                    }
                    break;
                    case Proto::CALL_RESPONSE::Type::Busy:
                        RestoreAfterCall();
                        messageBox->show(wui::locale("message", "subscriber_busy"),
                            wui::locale("message", "title_notification"),
                            wui::message_icon::alert,
                            wui::message_button::ok);
                    break;
                    case Proto::CALL_RESPONSE::Type::Timeout:
                        RestoreAfterCall();
                        messageBox->show(wui::locale("message", "subscriber_missed"),
                            wui::locale("message", "title_notification"),
                            wui::message_icon::alert,
                            wui::message_button::ok);
                    break;
                }
            }
            break;
            case Controller::Event::Type::ConferenceCreated:
                sysLog->info("Conference created");
                if (callParams.subscriberId != 0)
                {
                    controller.SendConnectToConference(e.conference.tag, callParams.subscriberId, callParams.subscriberConnectionId, static_cast<uint32_t>(Proto::SEND_CONNECT_TO_CONFERENCE::Flag::InviteCall));
                    ConnectToConference(e.conference.tag, false);
                }
            break;
            case Controller::Event::Type::ResponderNotRegistered:
                errLog->warn("Responder not registered");
                RestoreAfterCall();
                messageBox->show(wui::locale("message", "subscriber_not_exists"),
                    wui::locale("message", "title_notification"),
                    wui::message_icon::alert,
                    wui::message_button::ok);
            break;
            case Controller::Event::Type::ResponderNotConnected:
                errLog->warn("Responder not connected");
                RestoreAfterCall();
                messageBox->show(wui::locale("message", "subscriber_offline"),
                    wui::locale("message", "title_notification"),
                    wui::message_icon::alert,
                    wui::message_button::ok);
            break;
            case Controller::Event::Type::AutoCallDenied:
                errLog->warn("Auto call denied");
                RestoreAfterCall();
                messageBox->show(wui::locale("message", "auto_call_denied"),
                    wui::locale("message", "title_notification"),
                    wui::message_icon::alert,
                    wui::message_button::ok);
            break;
            case Controller::Event::Type::DoDisconnectFromConference: case Controller::Event::Type::ActionDisconnectFromConference:
                sysLog->info("Do Disconnect from conference");
                DisconnectFromConference();
                break;
            case Controller::Event::Type::StartConnectToConference:
                sysLog->info("Start connect to conference {0}", e.conference.tag);
                if (controller.GetState() != Controller::State::Conferencing && callParams.scheduleConferenceTag.empty())
                {
                    switch (static_cast<Proto::SEND_CONNECT_TO_CONFERENCE::Flag>(e.conference.grants))
                    {
                        case Proto::SEND_CONNECT_TO_CONFERENCE::Flag::InviteCall:
                            ConnectToConference(e.conference.tag, false);
                            callParams.Clear();
                        break;
                        case Proto::SEND_CONNECT_TO_CONFERENCE::Flag::AddMember:
                            ScheduleConnect(e.conference.tag, e.conference.name, true);
                        break;
                    }
                }
            break;

            /// Renderers
            case Controller::Event::Type::DeviceConnect:
                switch (e.deviceValues.type)
                {
                    case Proto::DeviceType::Camera:
                    case Proto::DeviceType::Demonstration:
                    {
                        const auto &renderer = e.deviceValues;

                        if (renderer.type == Proto::DeviceType::Demonstration && renderer.mySource)
                        {
                            return; /// not show our screen capture
                        }

                        /*if (cpuMeter.GetAVGUsage() > 80)
                        {
                            return messageBox->show(wui::locale("message", "cpu_too_many_loading") + renderer.name + " " + wui::locale("message", "not_turned_on"),
                                wui::locale("message", "title_notification"),
                                wui::message_icon::alert,
                                wui::message_button::ok);
                        }*/

                        if (renderer.type == Proto::DeviceType::Camera && !renderer.mySource && CheckInputBitrateTooSmall())
                        {
                            return ShowAvatar(renderer.clientId, renderer.name);
                        }

                        if (renderer.type == Proto::DeviceType::Camera)
                        {
                            HideAvatar(renderer.clientId);
                        }

                        if (!GetRendererVideoSession(renderer.deviceId))
                        {
                            auto rvs = std::make_shared<RendererSession::RendererVideoSession>(timeMeter);

                            rvs->SetResolution(renderer.resolution);
                            rvs->SetDecoderType(Video::CodecType::VP8);
                            rvs->SetName(renderer.name);
                            rvs->SetClientId(renderer.clientId);
                            rvs->SetDeviceType(renderer.type);
                            rvs->SetMy(renderer.mySource);
                            rvs->SetRTPParams(!useWSMedia ? renderer.addr.c_str() : "127.0.0.1", !useWSMedia ? renderer.port : tcpClient.CreatePipe(renderer.port));
                            rvs->SetMirrorVideo(wui::config::get_int("VideoRendererMirrors", renderer.name, -1) == 1);
                            rvs->SetDeviceNotifyCallback(std::bind(&MainFrame::ReceiveDeviceNotify, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
                            rvs->SetRecorder(&recorder);
                            rvs->SetFrameRate(renderer.type != Proto::DeviceType::Demonstration ? 25 : 5);

                            renderersVideo.emplace_back(rvs);

                            if (renderer.mySource)
                            {
                                auto cvs = GetCaptureVideoSession(renderer.type);
                                if (cvs)
                                {
                                    cvs->SetLocalReceiver(rvs->GetDirectReceiver());
                                    rvs->SetLocalCVS(cvs);
                                    if (renderer.type == Proto::DeviceType::Camera &&
                                        (wui::config::get_int("VideoRendererMirrors", renderer.name, -1) == -1 || wui::config::get_int("VideoRendererMirrors", renderer.name, 1) != 0))
                                    {
                                        rvs->SetMirrorVideo(true);
                                    }
                                }
                            }

                            rvs->Start(renderer.receiverSSRC, renderer.authorSSRC, renderer.deviceId, renderer.secureKey);

                            UpdateVideoRenderers();
                        }
                    }
                    break;
                    case Proto::DeviceType::Microphone:
                    {
                        const auto &renderer = e.deviceValues;

                        if (!GetRendererAudioSession(renderer.deviceId))
                        {
                            auto ras = std::make_shared<RendererSession::RendererAudioSession>(timeMeter, audioMixer);
                            ras->SetVolume(wui::config::get_int("Volumes", renderer.name, 100));
                            ras->SetDecoderType(Audio::CodecType::Opus);
                            ras->SetName(renderer.name);
                            ras->SetClientId(renderer.clientId);
                            ras->SetMetadata(renderer.metadata);
                            ras->SetMy(renderer.mySource);
                            ras->SetRTPParams(!useWSMedia ? renderer.addr.c_str() : "127.0.0.1", !useWSMedia ? renderer.port : tcpClient.CreatePipe(renderer.port));
                            ras->SetDeviceNotifyCallback(std::bind(&MainFrame::ReceiveDeviceNotify, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
                            ras->SetRecorder(&recorder);

                            if (renderer.mySource)
                            {
                                ras->SetMute(true);

                                if (captureAudioSession)
                                {
                                    captureAudioSession->SetLocalReceiver(ras->GetDirectReceiver());
                                }
                            }

                            renderersAudio[renderer.deviceId] = ras;

                            ras->Start(renderer.receiverSSRC, renderer.authorSSRC, renderer.deviceId, renderer.secureKey);
                        }
                    }
                    break;
                    default: break;
                }
                memberList.Update();
            break;
            case Controller::Event::Type::DeviceDisconnect:
            {
                switch (e.deviceValues.type)
                {
                    case Proto::DeviceType::Camera: case Proto::DeviceType::Demonstration:
                        DeleteVideoRenderer(e.deviceValues.deviceId, e.deviceValues.clientId, e.deviceValues.type);
                        
                        if (e.deviceValues.type == Proto::DeviceType::Camera)
                        {
                            std::string name;
                            {
                                std::lock_guard<std::recursive_mutex> lock(memberList.GetItemsMutex());
                                name = memberList.GetMemberName(e.deviceValues.clientId);
                            }
                            if (!name.empty())
                            {
                                ShowAvatar(e.deviceValues.clientId, name);
                            }
                        }
                    break;
                    case Proto::DeviceType::Microphone:
                    {
                        DeleteAudioRenderer(e.deviceValues.deviceId, e.deviceValues.clientId);
                    }
                    break;
                    default: break;
                }
                memberList.Update();
            }
            break;
            case Controller::Event::Type::ResolutionChanged:
            {
                auto rvs = GetRendererVideoSession(e.deviceValues.deviceId);
                if (rvs)
                {
                    sysLog->info("Camera resolution changed {0:d},{1:d} -> {2:d}", e.deviceValues.deviceId, e.deviceValues.clientId, e.deviceValues.resolution);

                    rvs->SetResolution((Video::Resolution)e.deviceValues.resolution);
                    renderersBox.Update();
                }
            }
            break;
            case Controller::Event::Type::MicrophoneActive:
            {
                for (auto rvs : renderersVideo)
                {
                    if (rvs->GetClientId() == e.deviceValues.clientId)
                    {
                        rvs->SetSpeak(e.iData == 2);
                    }
                }

                if (e.iData == 2)
                {
                    if (wui::config::get_int("User", "AccentuateSpeakerRenderer", 1) != 0 &&
                        BitIsClear(controller.GetCurrentConference().grants, static_cast<int32_t>(Proto::ConferenceGrants::DisableSpeakerChange)))
                    {
                        sysLog->trace("Current speaker is: {0}\n", e.deviceValues.clientId);
                        renderersBox.AccentuateSpeakerRenderer(e.deviceValues.clientId);
                    }

                    if (wui::config::get_int("Record", "Enabled", 0) != 0)
                    {
                        recorder.SpeakerChanged(e.deviceValues.clientId);
                    }
                }
            }
            break;

            /// Actions
            case Controller::Event::Type::UpdateMembers:
                if (wui::config::get_int("User", "DontUpdateBitrates", 0) == 0)
                {
                    UpdateOutputBitrates();
                }
                UpdateVideoRenderers();
            break;
            case Controller::Event::Type::ScheduleConnect:
            {
                sysLog->info("Schedule Connect {0}", e.conference.tag);

                if (controller.GetState() != Controller::State::Conferencing && callParams.scheduleConferenceTag.empty())
                {
                    callParams.timeLimit = e.conference.duration;

                    ScheduleConnect(e.conference.tag, e.conference.name, false);
                }
            }
            break;
            case Controller::Event::Type::ActionTurnCamera:
                ActionTurnCamera(false, e.iData, e.data);
            break;
            case Controller::Event::Type::ActionTurnMicrophone:
                ActionTurnMicrophone(false, e.iData, e.data);
            break;
            case Controller::Event::Type::ActionTurnDemonstration:
                ActionTurnDemonstration(false, e.iData, e.data);
            break;
            case Controller::Event::Type::ActionTurnSpeaker:
                ActionHand(false);
            break;
            case Controller::Event::Type::ActionMuteMicrophone:
                if (IsMicrophoneEnabled())
                {
                    StopMicrophone();
                    mainToolBar.EnableMicrophone(false);
                    wui::config::set_int("CaptureDevices", "MicrophoneEnabled", 0);
                    wui::config::set_int("User", "EnableMicOnEndConference", 1);
                }
            break;
            case Controller::Event::Type::ActionEnableRemoteControl:
                ActionTurnRemoteControl(false, e.iData, e.data, true);
            break;
            case Controller::Event::Type::ActionDisableRemoteControl:
                ActionTurnRemoteControl(false, e.iData, e.data, false);
            break;
            case Controller::Event::Type::WantSpeak:
            {
                sysLog->info("WantSpeak from {0:d} received", e.iData);
                auto id = e.iData;
                messageBox->show(wui::locale("common", "member") + ": " + e.data + " " + wui::locale("message", "member_want_speak"),
                    wui::locale("message", "title_notification"),
                    wui::message_icon::question,
                    wui::message_button::yes_no,
                    [this, id](wui::message_result result) {
                        if (result == wui::message_result::yes)
                        {
                            controller.SendMemberAction({ id }, Proto::MEMBER_ACTION::Action::TurnSpeaker);
                        }
                });
            }
            break;
        }
    }
}

void MainFrame::UpdateTitle(std::string_view text, int32_t progress)
{
    std::string title = wui::locale("client", "title") + SHORT_VERSION;

    switch (controller.GetState())
    { 
        case Controller::State::NetworkError:
            title += " :: " + wui::locale("message", "network_error");
        break;
        case Controller::State::UpdateRequired:
            title += " :: " + wui::locale("message", "update_required");
        break;
        case Controller::State::CredentialsError:
            title += " :: " + wui::locale("message", "invalid_credentionals");
        break;
        case Controller::State::ServerError:
            title += " :: " + wui::locale("message", "internal_server_error");
        break;
        case Controller::State::Ended:
            title += " :: " + wui::locale("common", "offline");
        break;

        case Controller::State::Initial:
            title += " :: " + wui::locale("common", "connecting_to_server");
        break;
        case Controller::State::Ready:
            title += " :: [" + controller.GetMyClientName() + " - " + controller.GetServerName() + "]";
        break;
        case Controller::State::Conferencing:
            title += " :: [" + controller.GetMyClientName() + " - " + controller.GetServerName() + "] :: " + 
                controller.GetCurrentConference().name;
        break;
        
    }

    if (!text.empty())
    {
        title += " :: " + std::string(text);
    }

    if (useWSMedia)
    {
        title += " {" + wui::locale("common", "tcp_media") + "}";
    }

    window->set_caption(title);

    static size_t prevSize = 0;

    mainProgress->set_value(progress);
    if (progress > 0)
    {
        if (!mainProgress->showed() || prevSize != title.size())
        {
#ifdef _WIN32
            auto memGr = std::unique_ptr<wui::graphic>(new wui::graphic(wui::system_context{ window->context().hwnd }));
#elif __linux__
            auto memGr = std::unique_ptr<wui::graphic>(new wui::graphic(window->context()));
#endif
            memGr->init(window->position(), 0);

            auto titleWidth = memGr->measure_text(title, wui::theme_font("window", "caption_font")).width();

            mainProgress->set_position({ titleWidth + 10, 8, titleWidth + 110, 22 });

            mainProgress->show();

            prevSize = title.size();
        }
    }
    else
    {
        mainProgress->hide();
    }
}

void MainFrame::Init()
{
    mainToolBar.Run();

    if (wui::config::get_int("ListPanel", "Enabled", 1) != 0)
    {
        listPanel.Run();
        listPanel.UpdateTop(mainToolBar.Bottom());
    }

    if (wui::config::get_int("ContentPanel", "Enabled", 1) != 0)
    {
        contentPanel.Run();
        contentPanel.UpdateTop(mainToolBar.Bottom());
        contentPanel.UpdateLeft(wui::config::get_int("ListPanel", "Enabled", 1) != 0 ? wui::config::get_int("ListPanel", "Width", 300) + 5 : 0);
    }

    UpdateTitle();
    
    LoadCameras(cameraDevices);
    FindCamera();
    mainToolBar.EnableCamera(IsCameraEnabled() && IsCameraExists());
    if (!IsCameraExists())
    {
        wui::config::set_int("CaptureDevices", "CameraEnabled", 0);
    }
    miniWindow.SetCameraState(IsCameraEnabled());

    LoadMicrophones(microphoneDevices);
    FindMicrophone();
    mainToolBar.EnableMicrophone(IsMicrophoneEnabled() && IsMicrophoneExists());
    if (!IsMicrophoneExists())
    {
        miniWindow.SetMicrophoneState(false);
        wui::config::set_int("CaptureDevices", "MicrophoneEnabled", 0);
    }

    miniWindow.SetMicrophoneState(IsMicrophoneEnabled());

    if (window->state() == wui::window_state::minimized)
    {
        wui::hide_taskbar_icon(window->context());
    }

    auto gridType = wui::config::get_int("VideoRenderer", "GridType", 1) != 0 ? GridType::MainUser : GridType::Even;
    mainToolBar.ChangeRendererMode(gridType == GridType::MainUser);
    renderersBox.SetGridType(gridType);
}

void MainFrame::WindowControlCallback(wui::window_control control, std::string &tooltip_text, bool &continue_)
{
    switch (control)
    {
        case wui::window_control::theme:
        {
            auto theme_name = wui::get_default_theme()->get_name();

            wui::error err;

            auto nextTheme = wui::get_next_app_theme();
            wui::set_default_theme_from_name(nextTheme, err);
            if (!err.is_ok())
            {
                errLog->critical("Change theme problem: {0}", err.str());
                return;
            }

            wui::config::set_string("User", "Theme", nextTheme);

            window->update_theme();
            timerBar.UpdateTheme();
        }
        break;
        case wui::window_control::close:
            if (controller.GetState() == Controller::State::Conferencing)
            {
                if (!callParams.callingSubscriber.empty())
                {
                    CancelCall();
                }
                DisconnectFromConference();
            }

            if (wui::config::get_int("MainFrame", "HideToTray", 1) != 0 && !userForceClose)
            {
                continue_ = false;

                wui::hide_taskbar_icon(window->context());
                window->minimize();

                trayIcon->show_message(wui::locale("client", "title"), wui::locale("message", "application_hided"));
            }
        break;
        case wui::window_control::state:
            if (tooltip_text == "expand" && !controller.GetCurrentConference().tag.empty())
            {
                window->set_style(wui::window_style::topmost);
            }
        break;
    }
}

void MainFrame::InitAudio()
{
    audioRendererDevices.clear();
    audioRendererDevices = audioRenderer.GetSoundRenderers();
    if (audioRendererDevices.empty())
    {
        return;
    }

    auto speakerName = wui::config::get_string("AudioRenderer", "Speaker", "");

    bool speakerFound = false;
    for (const auto &ar : audioRendererDevices)
    {
        if (ar == speakerName)
        {
            speakerFound = true;
            break;
        }
    }
    if (!speakerFound)
    {
        speakerName = audioRendererDevices[0];
    }

    if (wui::config::get_string("AudioRenderer", "Speaker", "").empty() || !speakerFound)
    {
        wui::config::set_string("AudioRenderer", "Speaker", speakerName);
    }

    audioRenderer.SetDeviceName(speakerName);
    audioRenderer.SetVolume((uint16_t)wui::config::get_int("AudioRenderer", "Volume", 100));

    audioMixer.Stop();
    audioRenderer.Stop();
    resampler.SetSampleFreq(48000, wui::config::get_int("AudioRenderer", "SampleFreq", 48000));
    audioRenderer.Start(wui::config::get_int("AudioRenderer", "SampleFreq", 48000));
    audioMixer.Start();

    audioRenderer.SetMute(wui::config::get_int("AudioRenderer", "Enabled", 1) == 0);
}

void MainFrame::ShowBusy(std::string_view title)
{
    busyTitle = title;
    if (window)
    {
        window->emit_event(static_cast<int32_t>(MyEvent::ShowBusy), 0);
    }
}

void MainFrame::SetBusyProgess(std::string_view sub_title, int32_t value)
{
    busySubTitle = sub_title;
    if (window)
    {
        window->emit_event(static_cast<int32_t>(MyEvent::SetBusyProgress), value);
    }
}

void MainFrame::HideBusy()
{
    if (window)
    {
        window->emit_event(static_cast<int32_t>(MyEvent::HideBusy), 0);
    }
}

void MainFrame::SetMainProgess(std::string_view sub_title, int32_t value)
{
    busySubTitle = sub_title;
    if (window)
    {
        window->emit_event(static_cast<int32_t>(MyEvent::SetMainProgress), value);
    }
}

void MainFrame::CheckConnectivity(bool showResult)
{
    showConnectivityResult = showResult;
    udpTester.ClearAddresses();
    controller.RequestMediaAddresses();

    UpdateTitle(wui::locale("net_test", "checking_connectivity"));
}

void MainFrame::DetermineNetSpeed(bool force)
{
    auto autoDetermineNetSpeed = wui::config::get_int("User", "AutoDetermineNetSpeed", 1) != 0;

    auto maxInputBitrate = wui::config::get_int("User", "MaxInputBitrate", 0);
    auto maxOutputBitrate = wui::config::get_int("User", "MaxOutputBitrate", 0);

    if (force || maxInputBitrate == 0 || maxOutputBitrate == 0 || autoDetermineNetSpeed)
    {
        if (window->state() != wui::window_state::minimized)
        {
            UpdateTitle(wui::locale("common", "determine_network_speed"));
        }

        speedTester.SetParams(wui::config::get_string("Connection", "Address", ""),
            wui::config::get_int("Connection", "Secure", 1) != 0,
            wui::config::get_string("Credentials", "Login", ""),
            wui::config::get_string("Credentials", "Password", ""));
        speedTester.DoTheTest();
    }
    else
    {
        controller.SetMaxBitrate(maxInputBitrate);
    }
}

void MainFrame::SetStanbyMode()
{
    listPanel.DeactivateMembers();
    contentPanel.SetStandbyMode();
    contentPanel.UpdateLeft(listPanel.Right());

    if (window->state() == wui::window_state::maximized)
    {
        window->set_style(wui::flags_map<wui::window_style>(2, wui::window_style::frame, wui::window_style::switch_theme_button));
        mainToolBar.Normalize();
        listPanel.UpdateTop(mainToolBar.Bottom());
    }

    renderersBox.End();
}

void MainFrame::SetConferencingMode()
{
    listPanel.ActivateMembers();
    contentPanel.SetConferencingMode();
    
    if (window->state() == wui::window_state::maximized)
    {
        window->set_style(wui::window_style::topmost);

        mainToolBar.FullScreen();
        listPanel.UpdateTop(0);
    }

    auto windowPos = window->position();
    renderersBox.Run({ listPanel.Right(), mainToolBar.Bottom(), contentPanel.Left(), windowPos.height() });
}

void MainFrame::SpeedTestCompleted(uint32_t inputSpeed, uint32_t outputSpeed)
{
    auto maxOutputBitrate = (controller.GetMaxOutputBitrate() != 0 && outputSpeed > controller.GetMaxOutputBitrate()) ?
        controller.GetMaxOutputBitrate() : outputSpeed;
    wui::config::set_int("User", "MaxInputBitrate", inputSpeed);
    wui::config::set_int("User", "MaxOutputBitrate", maxOutputBitrate);

    controller.SetMaxBitrate(inputSpeed);

    if (window)
    {
        window->emit_event(static_cast<int32_t>(MyEvent::SpeedTestCompleted), 0);
    }
}

void MainFrame::TCPTestCompleted()
{
    if (!window)
    {
        return;
    }

    useWSMedia = !udpTester.TestPassed();

    window->emit_event(static_cast<int32_t>(MyEvent::ConnectivityTestCompleted), 0);
}

void MainFrame::ReceiveDeviceNotify(std::string_view name, DeviceNotifyType notifyType, Proto::DeviceType deviceType, uint32_t deviceId, int32_t iData)
{
    if (!window)
    {
        return;
    }

    Controller::Event event_;
    switch (notifyType)
    {
        case DeviceNotifyType::Undefined: case DeviceNotifyType::OtherError: break;
        case DeviceNotifyType::CameraError:
            event_.type = Controller::Event::Type::CameraError;
        break;
        case DeviceNotifyType::MicrophoneError:
            event_.type = Controller::Event::Type::MicrophoneError;
        break;
        case DeviceNotifyType::DeviceEnded:
            event_.type = Controller::Event::Type::DeviceEnded;
        break;
        case DeviceNotifyType::MemoryError:
            event_.type = Controller::Event::Type::MemoryError;
        break;
        case DeviceNotifyType::OvertimeCoding:
            event_.type = Controller::Event::Type::OvertimeCoding;
        break;
        case DeviceNotifyType::OvertimeRendering:
            event_.type = Controller::Event::Type::OvertimeRendering;
        break;
        case DeviceNotifyType::ResolutionChanged:
            event_.type = Controller::Event::Type::DeviceResolutionChanged;
        break;
        case DeviceNotifyType::MicrophoneSpeak:
            event_.type = Controller::Event::Type::MicrophoneSpeak;
        break;
        case DeviceNotifyType::MicrophoneSilent:
            event_.type = Controller::Event::Type::MicrophoneSilent;
        break;
    }

    event_.deviceValues.name = name;
    event_.deviceValues.deviceId = deviceId;
    event_.deviceValues.type = deviceType;
    event_.iData = iData;

    controllerEventsQueue.push(event_);
    window->emit_event(static_cast<int32_t>(MyEvent::Controller), 0);
}

void MainFrame::AudioRendererErrorCallback(uint32_t code, std::string_view msg)
{
    errLog->warn("Audio renderer error: code: {0}, msg: {1}", code, msg);

    if (code == 110110110)
    {
        // FREQ_NOT_OPTIMAL
    }
}

void MainFrame::VolumeBoxChangeCallback(int32_t value, VolumeBoxMode mode, bool enabled)
{
    switch (mode)
    {
        case VolumeBoxMode::Microphone:
            if (captureAudioSession)
            {
                captureAudioSession->SetGain(value);
                if (!enabled)
                {
                    ActionTurnMicrophone();
                }
            }
            else
            {
                wui::config::set_int("CaptureDevices", "MicrophoneGain", value);
                if ((enabled && !IsMicrophoneEnabled()) ||
                    (!enabled && IsMicrophoneEnabled()))
                {
                    ActionTurnMicrophone();
                }
            }
            mainToolBar.EnableMicrophone(enabled);
        break;
        case VolumeBoxMode::Loudspeaker:
            audioRenderer.SetVolume(value);
            audioRenderer.SetMute(!enabled);
            mainToolBar.EnableLoudspeaker(enabled);
        break;
    }
}

void MainFrame::ActivateCallback()
{
    ConnectToURIConference();

    window->normal();
}

#ifndef _WIN32
std::string get_path()
{
    char arg1[20];
    char exepath[PATH_MAX + 1] = { 0 };

    sprintf(arg1, "/proc/%d/exe", getpid());
    int ret = readlink(arg1, exepath, 1024);
    if (ret != 0) {}
    return std::string(exepath);
}
#endif

void MainFrame::SettingsReadyCallback()
{
    if (controller.GetState() == Controller::State::Conferencing)
    {
        if (captureVideoSession)
        {
            if (captureVideoSession->GetName() != wui::config::get_string("CaptureDevices", "CameraName", ""))
            {
                StopCamera();
                StartCamera();
            }
            else if (captureVideoSession->GetResolution() != (Video::Resolution)wui::config::get_int("CaptureDevices", "CameraResolution", 0))
            {
                auto resolution = (Video::Resolution)wui::config::get_int("CaptureDevices", "CameraResolution", 0);
                
                controller.ChangeResolution(captureVideoSession->GetDeviceId(), resolution);
                captureVideoSession->SetResolution(resolution);
                
                UpdateOutputBitrates();
            }
        }

        if (captureAudioSession)
        {
            captureAudioSession->SetSampleFreq(wui::config::get_int("CaptureDevices", "MicrophoneSampleFreq", 48000));
            if (captureAudioSession->GetName() != wui::config::get_string("CaptureDevices", "MicrophoneName", ""))
            {
                StopMicrophone();
                StartMicrophone();
            }
            else
            {
                captureAudioSession->SetGain(wui::config::get_int("CaptureDevices", "MicrophoneGain", 80));
                captureAudioSession->EnableAEC(wui::config::get_int("CaptureDevices", "MicrophoneAEC", 1) != 0);
                captureAudioSession->EnableNS(wui::config::get_int("CaptureDevices", "MicrophoneNS", 1) != 0);
                captureAudioSession->EnableAGC(wui::config::get_int("CaptureDevices", "MicrophoneAGC", 1) != 0);
            }
        }
    }

    if (controller.GetServerAddress() != wui::config::get_string("Connection", "Address", "") ||
        controller.GetLogin() != wui::config::get_string("Credentials", "Login", "") ||
        controller.GetPassword() != wui::config::get_string("Credentials", "Password", ""))
    {
        UpdateTitle(wui::locale("common", "offline"));
        trayIcon->change_icon(ICO_INACTIVE);
        trayIcon->change_tip(wui::locale("client", "title") + "(" + wui::locale("common", "offline") + ")");

        credentialsDialog.login = wui::config::get_string("Credentials", "Login", "");
        credentialsDialog.password = wui::config::get_string("Credentials", "Password", "");
        
        window->emit_event(static_cast<int32_t>(MyEvent::CredentialsReady), 0);
    }

    auto localeType = static_cast<wui::locale_type>(wui::config::get_int("User", "Locale", static_cast<int32_t>(wui::locale_type::rus)));
    if (wui::get_locale()->get_type() != localeType)
    {
        messageBox->show(wui::locale("message", "app_need_restart"),
            wui::locale("message", "title_notification"),
            wui::message_icon::alert,
            wui::message_button::ok, [this](wui::message_result) {
#ifdef _WIN32
                wchar_t pathW[MAX_PATH] = { 0 };
                GetModuleFileNameW(NULL, pathW, MAX_PATH);
                std::string currentPath(boost::nowide::narrow(pathW));
                ShellExecute(NULL, L"open", pathW, L"/restart", 0, SW_SHOW);
#else
                const std::string path = get_path() + " /restart &";
                auto ret = system(path.c_str());
                if (ret != 0) {}
#endif
                userForceClose = true;
                window->destroy();
        });
    }

    if (controller.GetState() == Controller::State::Conferencing)
    {
        if (wui::config::get_int("User", "ShowTimer", 1) != 0)
        {
            timerBar.Run(callParams.timeLimit, listPanel.Right(), mainToolBar.Bottom());
        }
        else if (callParams.timeLimit == 0)
        {
            timerBar.End();
        }
    }

#ifdef _WIN32
	wui::config::config_impl_reg cir("Software\\Microsoft\\Windows\\CurrentVersion");
    if (wui::config::get_int("User", "AutoRunApp", 1) != 0)
    {
        wchar_t pathW[MAX_PATH] = { 0 };
        GetModuleFileNameW(NULL, pathW, MAX_PATH);
		cir.set_string("Run", SYSTEM_NAME "Client", boost::nowide::narrow(pathW) + " /autorun");
    }
    else
    {
        cir.delete_value("Run", SYSTEM_NAME "Client");
    }
#else
#endif
}

void MainFrame::TrayIconCallback(wui::tray_icon_action action)
{
    switch (action)
    {
        case wui::tray_icon_action::right_click:
            ConfirmClose(false);
        break;
        case wui::tray_icon_action::left_click: case wui::tray_icon_action::message_click:
            window->normal();
        break;
    }
}

void MainFrame::CredentialsCloseCallback()
{
    userForceClose = true;
    window->destroy();
}

void MainFrame::MessagesUpdatedCallback(Storage::MessageAction, const Storage::Messages &)
{
    size_t unreaded = 0;

    {
        std::lock_guard<std::recursive_mutex> lock(storage.GetContactsMutex());
        auto contacts = storage.GetContacts();
        for (auto &c : contacts)
        {
            unreaded += c.unreaded_count;
        }
    }

    {
        std::lock_guard<std::recursive_mutex> lock(storage.GetConferencesMutex());
        auto conferences = storage.GetConferences();
        for (auto &c : conferences)
        {
            unreaded += c.unreaded_count;
        }
    }

    static size_t prevUnreaded = unreaded;

    if (unreaded != 0 && prevUnreaded != unreaded)
    {
        ringer.Ring(Ringer::RingType::NewMessage);
        if (!Common::IsForegroundProcess())
        {
            trayIcon->show_message(wui::locale("client", "title"), wui::locale("message", "new_message_received") + std::to_string(unreaded));
        }
    }
    prevUnreaded = unreaded;
}

void MainFrame::AutoRegisterUser(std::string_view name)
{
    Registrator::Registrator registrator;
    registrator.Connect((wui::config::get_int("Connection", "Secure", 0) != 0 ? "https://" : "http://") + wui::config::get_string("Connection", "Address", "server_address:8778"));
    if (!registrator.Connected())
    {
        return messageBox->show(wui::locale("common", "offline"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok, [this](wui::message_result) {
                credentialsDialog.Run();
        });
    }

    std::string login, password;

#ifdef _WIN32
    wchar_t login_[32767];
    DWORD size = 32767;
    GetUserName(login_, &size);
    login = boost::nowide::narrow(login_);
#elif __linux__
    char login_[32767];
    getlogin_r(login_, sizeof(login_));
    login = login_;
#endif

    std::mt19937 gen(static_cast<uint32_t>(time(0)));
    std::uniform_int_distribution<> uid(0, 1000000);
    
    password = Common::toBase64(std::to_string(uid(gen)));

    std::uniform_int_distribution<> uidLogin(0, 1000);
    login += std::to_string(uidLogin(gen));

    auto result = registrator.Register(name, "", login, password);
    switch (result)
    {
        case Proto::USER_UPDATE_RESPONSE::Result::OK:
            messageBox->show(wui::locale("message", "registration_success"),
                wui::locale("message", "title_notification"),
                wui::message_icon::information,
                wui::message_button::ok,
                [this, login, password](wui::message_result) {
                    wui::config::set_string("Credentials", "Login", login);
                    wui::config::set_string("Credentials", "Password", password);
                    
                    credentialsDialog.login = login;
                    credentialsDialog.password = password;

                    window->emit_event(static_cast<int32_t>(MyEvent::CredentialsReady), 0);
            });
        break;
        case Proto::USER_UPDATE_RESPONSE::Result::DuplicateName:
            messageBox->show(wui::locale("message", "duplicate_name"),
                wui::locale("message", "title_error"),
                wui::message_icon::alert,
                wui::message_button::ok, [this](wui::message_result) {
                    credentialsDialog.Run();
            });
        break;
        case Proto::USER_UPDATE_RESPONSE::Result::DuplicateLogin:
            messageBox->show(wui::locale("message", "duplicate_credentionals"),
                wui::locale("message", "title_error"),
                wui::message_icon::alert,
                wui::message_button::ok, [this](wui::message_result) {
                    credentialsDialog.Run();
            });
        break;
        case Proto::USER_UPDATE_RESPONSE::Result::RegistrationDenied:
            messageBox->show(wui::locale("message", "registration_denied"),
                wui::locale("message", "title_error"),
                wui::message_icon::alert,
                wui::message_button::ok,
                [this](wui::message_result) {
                    credentialsDialog.Run();
            });
        break;
    }
}

/// Inner

std::shared_ptr<CaptureSession::CaptureVideoSession> MainFrame::GetCaptureVideoSession(Proto::DeviceType deviceType)
{
    if (deviceType == Proto::DeviceType::Camera && captureVideoSession)
    {
        return captureVideoSession;
    }
    else if (deviceType == Proto::DeviceType::Demonstration)
    {
        return demonstrationSession;
    }
    return nullptr;
}

std::shared_ptr<RendererSession::RendererVideoSession> MainFrame::GetRendererVideoSession(uint32_t deviceId)
{
    auto rvs = std::find_if(renderersVideo.begin(), renderersVideo.end(), [deviceId](const std::shared_ptr<RendererSession::RendererVideoSession> &p) { return p->GetDeviceId() == deviceId; });
    if (rvs != renderersVideo.end())
    {
        return *rvs;
    }
    return nullptr;
}

std::shared_ptr<RendererSession::RendererAudioSession> MainFrame::GetRendererAudioSession(uint32_t deviceId)
{
    auto it = renderersAudio.find(deviceId);
    if (it != renderersAudio.end())
    {
        return it->second;
    }
    return nullptr;
}

void SetVideoCapturerBitrate(std::shared_ptr<CaptureSession::CaptureVideoSession> cvs, uint32_t maxCameraBitrate)
{
    uint32_t cameraBitrate = maxCameraBitrate;

    auto height = Video::GetValues(cvs->GetResolution()).height;
    if (height >= 1080)
    {
        cameraBitrate = maxCameraBitrate >= 2048 ? 2048 : maxCameraBitrate;
    }
    else if (height < 1080 && height >= 720)
    {
        cameraBitrate = maxCameraBitrate >= 1536 ? 1536 : maxCameraBitrate;
    }
    else if (height < 720 && height >= 480)
    {
        cameraBitrate = maxCameraBitrate >= 1024 ? 1024 : maxCameraBitrate;
    }
    else if (height < 480 && height >= 240)
    {
        cameraBitrate = maxCameraBitrate >= 640 ? 640 : maxCameraBitrate;
    }
    else if (height < 240)
    {
        cameraBitrate = maxCameraBitrate >= 512 ? 512 : maxCameraBitrate;
    }

    cvs->SetBitrate(cameraBitrate);
}

#ifdef _WIN32
void MainFrame::CheckVGProtocol()
{
    wchar_t pathW[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, pathW, MAX_PATH);
    std::string currentPath(boost::nowide::narrow(pathW));

	wui::config::config_impl_reg cir("vg\\shell\\open", HKEY_CLASSES_ROOT);

    if (cir.get_string("command", "", "") != "\"" + currentPath + "\" %1")
    {
        bool haveAdminPrivileges = false;
        HANDLE hToken = NULL;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        {
            TOKEN_ELEVATION elevation;
            DWORD dwSize;
            if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize))
            {
                haveAdminPrivileges = elevation.TokenIsElevated;
            }
        }
        CloseHandle(hToken);

        if (!haveAdminPrivileges)
        {
            messageBox->show(wui::locale("message", "app_needed_admin_privilleges"),
                wui::locale("message", "title_notification"),
                wui::message_icon::alert,
                wui::message_button::ok, [](wui::message_result) {
                wchar_t pathW[MAX_PATH] = { 0 };
                GetModuleFileNameW(NULL, pathW, MAX_PATH);
                ShellExecute(NULL, L"runas", pathW, L"/regproto", 0, SW_HIDE);
            });
        }
        else
        {
            RegisterVGProtocol();
        }
    }
}

void MainFrame::RegisterVGProtocol()
{
    wchar_t pathW[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, pathW, MAX_PATH);
    std::string path(boost::nowide::narrow(pathW));

	wui::config::config_impl_reg cir(BROWSER_PROTO, HKEY_CLASSES_ROOT);

    cir.set_string("", "", "URL:" SYSTEM_NAME " Conferencing");
    cir.set_string("", "URL Protocol", "");

    cir.set_string("DefaultIcon", "", Common::FileNameOf(path) + ",1");

	wui::config::config_impl_reg cir1(BROWSER_PROTO "\\shell\\open", HKEY_CLASSES_ROOT);
    cir1.set_string("command", "", "\"" + path + "\" %1");
}

#elif __linux__

void MainFrame::CheckVGProtocol()
{
}
void MainFrame::RegisterVGProtocol()
{
}

#endif

void MainFrame::UpdateOutputBitrates()
{
    uint32_t maximumInputBitrate = 0;
    uint32_t speakersCount = 0;

    {
        std::lock_guard<std::recursive_mutex> lock(memberList.GetItemsMutex());
        maximumInputBitrate = (uint32_t)(memberList.GetMaximumInputBitrate() * 0.9);
        speakersCount = memberList.GetSpeakersCount();
    }
    if (speakersCount > 1)
    {
        maximumInputBitrate /= speakersCount;
    }

    uint32_t maximumOutputBitrate = (uint32_t)(wui::config::get_int("User", "MaxOutputBitrate", 2048) * 0.9);
    if (maximumInputBitrate > maximumOutputBitrate)
    {
        maximumInputBitrate = maximumOutputBitrate;
    }

    if (captureAudioSession)
    {
        uint32_t microphoneBitrate = 30;

        if (microphoneBitrate > maximumInputBitrate)
        {
            microphoneBitrate = maximumInputBitrate / 4;
        }

        captureAudioSession->SetBitrate(microphoneBitrate);

        maximumInputBitrate -= microphoneBitrate;        
    }

    size_t camerasCount = 0;
    
    if (captureVideoSession)
    {
        ++camerasCount;
    }
    if (demonstrationSession)
    {
        ++camerasCount;
    }

    if (camerasCount != 0)
    {
        uint32_t maxCameraBitrate = (uint32_t)(maximumInputBitrate / camerasCount);

        if (captureVideoSession)
        {
            SetVideoCapturerBitrate(captureVideoSession, maxCameraBitrate);
        }

        if (demonstrationSession)
        {
            SetVideoCapturerBitrate(demonstrationSession, maxCameraBitrate);
        }
    }
}

bool MainFrame::CheckOutputBitrateTooSmall()
{
    return wui::config::get_int("User", "MaxOutputBitrate", 2048) < 256;
}

bool MainFrame::CheckInputBitrateTooSmall()
{
    return wui::config::get_int("User", "MaxInputBitrate", 2048) < 256;
}

void MainFrame::KillScreenSaver()
{

}

void MainFrame::Call(std::string_view subscriber)
{
    if (controller.GetState() != Controller::State::Ready && controller.GetState() != Controller::State::Conferencing)
    {
        return;
    }

    ringer.Ring(Ringer::RingType::CallOut);

    callParams.callingSubscriber = subscriber;

    Controller::CallValues cv;
    cv.name = subscriber;
    cv.requestType = Proto::CALL_REQUEST::Type::Invocation;
    controller.CallRequest(cv);

    dialingDialog.Run(subscriber);
}

void MainFrame::CancelCall()
{
    Controller::CallValues cv;
    cv.name = callParams.callingSubscriber;
    cv.requestType = Proto::CALL_REQUEST::Type::Cancel;
    controller.CallRequest(cv);

    RestoreAfterCall();
}

void MainFrame::RestoreAfterCall()
{
    callParams.Clear();
    if (ringer.Runned())
    {
        ringer.Stop();
    }
    dialingDialog.End();
}

void MainFrame::QuestionCall(bool yes)
{
    if (callParams.scheduleConferenceTag.empty())
    {
        if (!questionDialog.missed && ringer.Runned())
        {
            ringer.Stop();
        }

        if (callParams.subscriberId != -1 && callParams.subscriberConnectionId != -1)
        {
            controller.CallResponse(Controller::CallValues(
                questionDialog.missed ? Proto::CALL_RESPONSE::Type::Timeout : (yes ? Proto::CALL_RESPONSE::Type::Accept : Proto::CALL_RESPONSE::Type::Refuse),
                callParams.subscriberId,
                callParams.subscriberConnectionId,
                callParams.subscriberName));

            if (yes)
            {
                contactList.SelectUser(callParams.subscriberId, callParams.subscriberName);
            }
        }
    }
    else
    {
        if (ringer.Runned())
        {
            ringer.Stop();
        }

        if (yes)
        {
            ConnectToConference(callParams.scheduleConferenceTag, false);
            contactList.SelectConference(callParams.scheduleConferenceTag, callParams.scheduleConferenceName);
        }

        callParams.scheduleConferenceTag.clear();
    }
}

void MainFrame::ConnectToURIConference()
{
    auto tag = wui::config::get_string("User", "ConferenceTag", "");
    if (!tag.empty())
    {
        controller.AddMeToConference(tag);
        ConnectToConference(tag, false);
        wui::config::delete_value("User", "ConferenceTag");
    }
}

void MainFrame::ConnectToConference(std::string_view tag, bool connectMembers)
{
    if (tag.empty() || controller.GetState() != Controller::State::Ready || controller.GetState() == Controller::State::Conferencing)
    {
        return;
    }

    bool cameraEnabled = IsCameraEnabled() && !CheckOutputBitrateTooSmall();

    Proto::Conference conf;
    conf.tag = tag;
    conf.connect_members = connectMembers;
    controller.ConnectToConference(conf, cameraEnabled, IsMicrophoneEnabled(), IsDemonstrationEnabled());
}

void MainFrame::DisconnectFromConference()
{
    if (wui::config::get_int("Record", "Enabled", 0) != 0)
    {
        recorder.Stop();
    }

    if (!controller.GetCurrentConference().tag.empty())
    {
        ringer.Ring(Ringer::RingType::Hangup);
    }

    std::vector<Controller::DeviceValues> capturers;
    
    Controller::DeviceValues capturer;

    if (captureAudioSession)
    {
        capturer.deviceId = captureAudioSession->GetDeviceId();
        capturers.emplace_back(capturer);

        captureAudioSession.reset();
    }

    if (captureVideoSession)
    {
        capturer.deviceId = captureVideoSession->GetDeviceId();
        capturers.emplace_back(capturer);

        captureVideoSession.reset();
    }

    if (demonstrationSession)
    {
        capturer.deviceId = demonstrationSession->GetDeviceId();
        capturers.emplace_back(capturer);

        demonstrationSession.reset();
    }

    std::vector<Controller::DeviceValues> renderers;
    for (auto &ras : renderersAudio)
    {
        Controller::DeviceValues renderer;
        renderer.deviceId = ras.second->GetDeviceId();
        renderer.receiverSSRC = ras.second->GetSSRC();
        renderers.emplace_back(renderer);
    }
    renderersAudio.clear();

    for (auto &rvs : renderersVideo)
    {
        Controller::DeviceValues renderer;
        renderer.deviceId = rvs->GetDeviceId();
        renderer.receiverSSRC = rvs->GetSSRC();
        renderers.emplace_back(renderer);
    }
    renderersVideo.clear();

    controller.DisconnectFromConference(capturers, renderers);
}

void MainFrame::ScheduleConnect(std::string_view tag, std::string_view name, bool forceQuestion)
{
    callParams.scheduleConferenceTag = tag;
    callParams.scheduleConferenceName = name;

    KillScreenSaver();

    if (wui::config::get_int("User", "AutoConnectToConf", 1) != 0 && !forceQuestion)
    {
        ringer.Ring(Ringer::RingType::ScheduleConnectQuick);
        ShowBusy(wui::locale("common", "starting_conference") + "\n" + std::string(name));
        contactList.SelectConference(tag, name);
    }
    else
    {
        ringer.Ring(Ringer::RingType::ScheduleConnectLong);
        questionDialog.Run(wui::locale("common", "starting_conference") + "\n" + std::string(name) + "\n" + wui::locale("common", "connect_to_starting_conference"));
    }
}

void MainFrame::StartCamera()
{
    if (!IsCameraEnabled())
    {
        return;
    }

    if (!IsCameraExists())
    {
        return messageBox->show(wui::locale("message", "no_camera_in_system"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok);
    }

    auto deviceName = wui::config::get_string("CaptureDevices", "CameraName", "");

    int ord = 0;
    std::vector<Video::VideoFormat> *formats = nullptr;
    for (auto &camera : cameraDevices)
    {
        if (camera.name == deviceName)
        {
            formats = &camera.formats;
            break;
        }
        ++ord;
    }

    if (formats && controller.GetState() == Controller::State::Conferencing)
    {
        Controller::DeviceValues device;
        device.pos = ord;
        device.name = deviceName;
        device.type = Proto::DeviceType::Camera;
        Video::Resolution resolution = (Video::Resolution)wui::config::get_int("CaptureDevices", "CameraResolution", GetBestResolution(*formats, wui::config::get_int("User", "Flops", 0) < 400000000 ? 240 : 480));
        device.resolution = resolution;
        device.colorSpace = GetBestColorSpace(*formats, resolution);
        controller.CreateCapturer(device);
    }
}

void MainFrame::StopCamera()
{
    if (captureVideoSession)
    {
        auto deviceId = captureVideoSession->GetDeviceId();
        captureVideoSession->Stop();
        captureVideoSession.reset();
        controller.DeleteCapturer(deviceId);
    }
}

void MainFrame::StartDemonstration()
{
    if (!IsDemonstrationEnabled() && controller.GetState() == Controller::State::Conferencing)
    {
        auto screenSize = wui::get_screen_size(window->context());

        Controller::DeviceValues device;
        device.name = wui::locale("device", "screen_capturer");
        device.type = Proto::DeviceType::Demonstration;
        device.resolution = Video::GetResolution(Video::ResolutionValues(screenSize.width(), screenSize.height()));
        controller.CreateCapturer(device);
    }
}

void MainFrame::StopDemonstration()
{
    if (IsDemonstrationEnabled())
    {
        auto deviceId = demonstrationSession->GetDeviceId();
        demonstrationSession->Stop();
        demonstrationSession.reset();
        controller.DeleteCapturer(deviceId);
    }
}

void MainFrame::StartMicrophone()
{
    if (!IsMicrophoneEnabled())
    {
        return;
    }

    if (!IsMicrophoneExists())
    {
        return messageBox->show(wui::locale("message", "no_microphone_in_system"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok);
    }

    auto deviceName = wui::config::get_string("CaptureDevices", "MicrophoneName", "");

    int ord = 0; bool found = false;
    for (const auto &microphone : microphoneDevices)
    {
        if (microphone.name == deviceName)
        {
            found = true;
            break;
        }
        ++ord;
    }

    if (found && controller.GetState() == Controller::State::Conferencing)
    {
        Controller::DeviceValues device;
        device.pos = ord;
        device.name = deviceName;
        device.type = Proto::DeviceType::Microphone;
        controller.CreateCapturer(device);        
    }
}

void MainFrame::StopMicrophone()
{
    if (captureAudioSession)
    {
        auto deviceId = captureAudioSession->GetDeviceId();

        controller.MicrophoneActive(deviceId, Proto::MICROPHONE_ACTIVE::ActiveType::Silent);

        captureAudioSession->Stop();
        controller.DeleteCapturer(deviceId);

        captureAudioSession.reset();

        controller.MicrophoneActive(deviceId, Proto::MICROPHONE_ACTIVE::ActiveType::Silent);
    }
}

bool MainFrame::IsCameraEnabled()
{
    return wui::config::get_int("CaptureDevices", "CameraEnabled", 1) != 0;
}

bool MainFrame::IsCameraExists()
{
    auto name = wui::config::get_string("CaptureDevices", "CameraName", "");
    for (const auto &camera : cameraDevices)
    {
        if (camera.type == DeviceType::Camera && camera.name == name)
        {
            return true;
        }
    }

    if (cameraDevices.size() > 1)
    {
        for (auto &device : cameraDevices)
        {
            if (device.type == DeviceType::Camera)
            {
                wui::config::set_string("CaptureDevices", "CameraName", cameraDevices.front().name);
                
                return true;
            }
        }
    }

    return false;
}

bool MainFrame::IsMicrophoneEnabled()
{
    return wui::config::get_int("CaptureDevices", "MicrophoneEnabled", 1) != 0;
}

bool MainFrame::IsMicrophoneExists()
{
    auto name = wui::config::get_string("CaptureDevices", "MicrophoneName", "");
    for (const auto &microphone : microphoneDevices)
    {
        if (microphone.name == name)
        {
            return true;
        }
    }

    if (!microphoneDevices.empty())
    {
        wui::config::set_string("CaptureDevices", "MicrophoneName", microphoneDevices.front().name);
        return true;
    }

    return false;
}

bool MainFrame::IsDemonstrationEnabled()
{
    return demonstrationSession != nullptr;
}

bool MainFrame::IsRecordAllowed()
{
    if (BitIsSet(controller.GetCurrentConference().grants, static_cast<int32_t>(Proto::ConferenceGrants::DenyRecord)) && !memberList.IsMePresenter() && !memberList.IsMeModerator())
    {
        wui::config::set_int("Record", "Enabled", 0);
        mainMenu->update_item({ 2, wui::menu_item_state::separator, wui::locale("menu", "enable_record"), "Ctrl+R", nullptr, {}, [this](int32_t i) { ActionTurnRecord(); } });
        
        messageBox->show(wui::locale("message", "record_denied"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok);
        
        return false;
    }
    return true;
}

void MainFrame::FindCamera()
{
    if (wui::config::get_string("CaptureDevices", "CameraName", "").empty())
    {
        for (auto &camera : cameraDevices)
        {
            if (camera.type == DeviceType::Camera)
            {
                wui::config::set_string("CaptureDevices", "CameraName", camera.name);
                break;
            }
        }
    }
}

void MainFrame::FindMicrophone()
{
    if (!wui::config::get_string("CaptureDevices", "MicrophoneName", "").empty() || microphoneDevices.empty())
    {
        return;
    }

    wui::config::set_string("CaptureDevices", "MicrophoneName", microphoneDevices.front().name);
}

void MainFrame::ShowAvatar(int64_t clientId, std::string_view name)
{
    if (wui::config::get_int("User", "OutputAvatar", 1) == 0)
    {
        return;
    }

    for (auto &rvs : renderersVideo)
    {
        if (rvs->GetClientId() == clientId && rvs->GetDeviceType() == Proto::DeviceType::Avatar)
        {
            return;
        }
    }

    auto rvs = std::make_shared<RendererSession::RendererVideoSession>(timeMeter);

    rvs->SetName(name);
    rvs->SetClientId(clientId);
    rvs->SetDeviceType(Proto::DeviceType::Avatar);
    rvs->SetResolution(Video::GetResolution(Video::ResolutionValues(910, 512)));
    rvs->SetMy(clientId == controller.GetMyClientId());

    renderersVideo.emplace_back(rvs);

    static uint32_t devid = 0xFFFFFFFF;
    rvs->Start(0, 0, --devid, "");

    renderersBox.Update();
}

void MainFrame::HideAvatar(int64_t clientId)
{
    if (wui::config::get_int("User", "OutputAvatar", 1) == 0)
    {
        return;
    }

    for (auto rvs = renderersVideo.begin(); rvs != renderersVideo.end(); ++rvs)
    {
        if ((*rvs)->GetClientId() == clientId && (*rvs)->GetDeviceType() == Proto::DeviceType::Avatar)
        {
            (*rvs)->Stop();

            renderersVideo.erase(rvs);

            UpdateVideoRenderers();

            break;
        }
    }
}

void MainFrame::DeleteVideoRenderer(uint32_t deviceId, int64_t clientId, Proto::DeviceType deviceType)
{
    auto rvs = GetRendererVideoSession(deviceId);
    if (rvs)
    {
        rvs->Stop();

        if (rvs->GetMy())
        {
            auto cvs = GetCaptureVideoSession(deviceType);
            if (cvs)
            {
                cvs->SetLocalReceiver(nullptr);
            }
        }

        auto it = std::find_if(renderersVideo.begin(), renderersVideo.end(), [deviceId](const std::shared_ptr<RendererSession::RendererVideoSession> &p) { return p->GetDeviceId() == deviceId; });
        if (it != renderersVideo.end())
        {
            renderersVideo.erase(it);
        }

        renderersBox.DeleteRenderer(deviceId);
        UpdateVideoRenderers();
    }
}

void MainFrame::DeleteAudioRenderer(uint32_t deviceId, int64_t clientId)
{
    auto ras = GetRendererAudioSession(deviceId);
    if (ras)
    {
        ras->Stop();

        if (ras->GetMy() && captureAudioSession)
        {
            captureAudioSession->SetLocalReceiver(nullptr);
        }

        renderersAudio.erase(deviceId);
    }
}

void MainFrame::UpdateVideoRenderers()
{
    {
        std::lock_guard<std::recursive_mutex> lock(memberList.GetItemsMutex());

        const auto &members = memberList.GetItems();
        for (const auto &member : members)
        {
            for (auto &rvs : renderersVideo)
            {
                if (rvs->GetClientId() == member.id)
                {
                    rvs->SetOrder(member.order + 10);

                    if (rvs->GetDeviceType() == Proto::DeviceType::Demonstration)
                    {
                        rvs->SetOrder(0);
                    }
                }
            }
        }
    }

    if (renderersVideo.size() > 2)
    {
        std::sort(renderersVideo.begin(), renderersVideo.end(),
            [](const RendererSession::RendererVideoSessionPtr_t &a, const RendererSession::RendererVideoSessionPtr_t &b)
            {
                return a->GetOrder() < b->GetOrder();
            });
    }

    renderersBox.Update();
}

void MainFrame::UpdateVideoRendererPosition()
{
    renderersBox.SetPosition({ listPanel.Right(), mainToolBar.Bottom(), contentPanel.Left(), window->position().height() });
    timerBar.UpdatePosition(listPanel.Right(), mainToolBar.Bottom());
}

void MainFrame::UpdateConnectedCameras()
{
    auto oldCams = cameraDevices;

    cameraDevices.clear();
    LoadCameras(cameraDevices);

    if (cameraDevices.size() != oldCams.size())
    {
        for (auto &d : oldCams)
        {
            if (std::find(cameraDevices.begin(), cameraDevices.end(), d.name) == cameraDevices.end() &&
               wui::config::get_string("CaptureDevices", "CameraName", "") == d.name)
            {
                StopCamera();
                mainToolBar.EnableCamera(false);
                wui::config::set_int("CaptureDevices", "CameraEnabled", 0);
            }
        }

        settingsDialog.UpdateCameras();
    }
}

void MainFrame::UpdateConnectedMicrophones()
{
    auto oldMics = microphoneDevices;

    microphoneDevices.clear();
    LoadMicrophones(microphoneDevices);

    if (microphoneDevices.size() != oldMics.size())
    {
        for (auto &d : oldMics)
        {
            if (std::find(microphoneDevices.begin(), microphoneDevices.end(), d.name) == microphoneDevices.end() &&
                wui::config::get_string("CaptureDevices", "MicrophoneName", "") == d.name)
            {
                StopMicrophone();
                mainToolBar.EnableMicrophone(false);
                wui::config::set_int("CaptureDevices", "MicrophoneEnabled", 0);
            }
        }

        settingsDialog.UpdateMicrophones();
    }
}

void MainFrame::UpdateConnectedAudioRenderers()
{
    std::vector<std::string> newPlayers = audioRenderer.GetSoundRenderers();

    if (newPlayers.size() != audioRendererDevices.size())
    {
        sysLog->info("changed system audio renderers");

        InitAudio();

        settingsDialog.UpdateLoudspeakers();
    }
}

void MainFrame::UpdateContentPanelLeft()
{
    if (controller.GetCurrentConference().tag.empty())
    {
        contentPanel.UpdateLeft(listPanel.Right());
    }
}

void MainFrame::ConfirmClose(bool minimizeAfterNo)
{
    messageBox->show(wui::locale("message", "confirm_app_close"),
        wui::locale("message", "title_notification"),
        wui::message_icon::alert,
        wui::message_button::yes_no, [this, minimizeAfterNo](wui::message_result r) {
        if (r == wui::message_result::yes)
        {
            userForceClose = true;
            window->destroy();
        }
        else
        {
            if (minimizeAfterNo)
            {
                wui::hide_taskbar_icon(window->context());
                window->minimize();

                trayIcon->show_message(wui::locale("client", "title"), wui::locale("message", "application_hided"));
            }
        }
    });
}

void MainFrame::ShowHelp()
{
    auto serverAddress = controller.GetServerAddress();
    auto secureConnection = controller.IsSecureConnection();

    std::vector<std::string> vals;
    boost::split(vals, serverAddress, boost::is_any_of(":"));
    if (vals.size() == 2 && (vals[1] == "5060" || vals[1] == "80" || vals[1] == "443"))
    {
        serverAddress = vals[0];
    }

    if (!wui::open_uri((secureConnection ? "https://" : "http://")
        + serverAddress + "/doc/client_manual_"
        + std::string(wui::str(wui::get_locale()->get_type())) + ".pdf"))
    {
        errLog->critical("MainFrame :: Can't open documentation");
    }
}

}
