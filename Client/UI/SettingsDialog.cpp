/**
 * SettingsDialog.cpp - Contains settings dialog impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <UI/SettingsDialog.h>

#include <wui/locale/locale.hpp>
#include <wui/theme/theme.hpp>
#include <wui/system/tools.hpp>

#include <wui/config/config.hpp>

#ifdef WIN32
#include <Shlobj.h>
#else
#include <pwd.h>
#include <sys/types.h>
#endif

#include <boost/nowide/convert.hpp>
#include <boost/algorithm/string.hpp>

#include <resource.h>

namespace Client
{

const auto UNCHANGED = "----****----";

std::shared_ptr<wui::i_theme> MakeErrorTextTheme()
{
    auto errorTextTheme = wui::make_custom_theme();

    errorTextTheme->load_theme(*wui::get_default_theme());

    errorTextTheme->set_color(wui::text::tc, wui::text::tv_color, wui::make_color(205, 15, 20));
    auto font = errorTextTheme->get_font(wui::text::tc, wui::text::tv_font);
    font.size = 36;
    font.decorations_ = wui::decorations::bold;
    errorTextTheme->set_font(wui::text::tc, wui::text::tv_font, font);

    return errorTextTheme;
}

SettingsDialog::SettingsDialog(std::weak_ptr<wui::window> transientWindow_,
    Controller::IController &controller_,
    AudioRenderer::IAudioRenderer &audioRenderer_,
    Audio::Resampler &resampler_,
    Audio::AudioMixer &audioMixer_,
    Ringer &ringer_,
    std::function<void(bool)> netSpeedDeterminer_,
    std::function<void(bool)> connectivityDeterminer_,
    std::function<void()> readyCallback_)
    : readyCallback(readyCallback_),
    netSpeedDeterminer(netSpeedDeterminer_),
    connectivityDeterminer(connectivityDeterminer_),
    controller(controller_),
    transientWindow(transientWindow_),
    window(new wui::window()),
    sectionList(new wui::list()),
    okButton(new wui::button(wui::locale("button", "ok"), std::bind(&SettingsDialog::OK, this), "green_button")),
    cancelButton(new wui::button(wui::locale("button", "cancel"), [this]() { window->destroy(); }, "red_button")),
    cameraImg(new wui::image(IMG_SETTING_CAMERA)),
    microphoneImg(new wui::image(IMG_SETTING_MICROPHONE)),
    loudspeakerImg(new wui::image(IMG_SETTING_LOUDSPEAKER)),
    accountImg(new wui::image(IMG_SETTING_ACCOUNT)),
    connectionImg(new wui::image(IMG_SETTING_CONNECTION)),
    preferencesImg(new wui::image(IMG_SETTING_PREFERENCES)),
    recordImg(new wui::image(IMG_SETTING_RECORD)),
    noSignalImg(new wui::image(IMG_SETTING_NO_SIGNAL)),
    messageBox(new wui::message(window)),

    timeMeter(),

    cameraSelect(new wui::select()),
    cameraResolutionText(new wui::text(wui::locale("settings", "camera_resolution"))),
    cameraResolutionSelect(new wui::select()),
    cameraErrorText(new wui::text()),
    cameraDevices(),
    captureVideoSession(timeMeter),
    rendererVideoSession(timeMeter),
    redrawVideoTimer(std::bind(&SettingsDialog::RedrawVideo, this)),
    currentCameraId(-1), currentCameraResolutionId(-1),

    microphoneSelect(new wui::select()),
    microphoneSensitivityText(new wui::text(wui::locale("settings", "microphone_sensitivty"))),
    microphoneSensitivitySlider(new wui::slider(0, 100, wui::config::get_int("CaptureDevices", "MicrophoneGain", 80), [this](int32_t value) { microphone.SetGain(value); })),
    microphoneAECCheck(new wui::button(wui::locale("settings", "microphone_aec"), []() {}, wui::button_view::switcher)),
    microphoneNSCheck(new wui::button(wui::locale("settings", "microphone_ns"), []() {}, wui::button_view::switcher)),
    microphoneAGCCheck(new wui::button(wui::locale("settings", "microphone_agc"), []() {}, wui::button_view::switcher)),
    microphone16SampleRateCheck(new wui::button(wui::locale("settings", "microphone_16_sample_rate"), [this]() { microphone48SampleRateCheck->turn(!microphone48SampleRateCheck->turned()); }, wui::button_view::radio)),
    microphone48SampleRateCheck(new wui::button(wui::locale("settings", "microphone_48_sample_rate"), [this]() { microphone16SampleRateCheck->turn(!microphone16SampleRateCheck->turned()); }, wui::button_view::radio)),
    soundIndicator(new SoundIndicator()),
    microphone(timeMeter, *soundIndicator),
    currentMicrophoneId(-1),

    loudspeakerSelect(new wui::select()),
    loudspeakerVolumeText(new wui::text(wui::locale("settings", "loudspeaker_volume"))),
    loudspeakerSlider(new wui::slider(0, 100, wui::config::get_int("AudioRenderer", "Volume", 100), [this](int32_t value) { audioRenderer.SetVolume(value); })),
    loudspeakerCheckButton(new wui::button(wui::locale("settings", "loudspeaker_check"), [this]() { ringer.Ring(Ringer::RingType::SoundCheck); })),
    loudspeaker16SampleRateCheck(new wui::button(wui::locale("settings", "loudspeaker_16_sample_rate"), [this]() { ChangeLoudspeakerSampleRate(16000); loudspeaker48SampleRateCheck->turn(!loudspeaker48SampleRateCheck->turned()); }, wui::button_view::radio)),
    loudspeaker48SampleRateCheck(new wui::button(wui::locale("settings", "loudspeaker_48_sample_rate"), [this]() { ChangeLoudspeakerSampleRate(48000); loudspeaker16SampleRateCheck->turn(!loudspeaker16SampleRateCheck->turned()); }, wui::button_view::radio)),
    audioRenderer(audioRenderer_),
    resampler(resampler_),
    audioMixer(audioMixer_),
    ringer(ringer_),
    currentLoudspeakerId(-1),

    userNameText(new wui::text(wui::locale("settings", "user_name"))),
    userNameInput(new wui::input()),
    userChangeChandentialsLink(new wui::button(wui::locale("settings", "show_change_chandentials"), std::bind(&SettingsDialog::ShowChangeChandentials, this), wui::button_view::anchor)),
    userLoginText(new wui::text(wui::locale("settings", "user_login"))),
    userLoginInput(new wui::input()),
    userPasswordText(new wui::text(wui::locale("settings", "user_password"))),
    userPasswordInput(new wui::input("", wui::input_view::password)),
    userPasswordConfirmText(new wui::text(wui::locale("settings", "user_password_confirm"))),
    userPasswordConfirmInput(new wui::input("", wui::input_view::password)),
    userName(UNCHANGED), userLogin(UNCHANGED), userPassword(UNCHANGED), userPasswordConfirm(UNCHANGED),

    connectionServerText(new wui::text(wui::locale("settings", "connection_server"))),
    connectionServerInput(new wui::input()),
    connectionLoginText(new wui::text(wui::locale("settings", "user_login"))),
    connectionLoginInput(new wui::input()),
    connectionPasswordText(new wui::text(wui::locale("settings", "user_password"))),
    connectionPasswordInput(new wui::input("", wui::input_view::password)),
    connectionAutoDetermineSpeedCheck(new wui::button(wui::locale("settings", "auto_determine_network_speed"), [this]() { connectionSetSpeedManualCheck->turn(!connectionAutoDetermineSpeedCheck->turned()); UpdateSpeedInputs(); }, wui::button_view::radio)),
    connectionSetSpeedManualCheck(new wui::button(wui::locale("settings", "set_network_speed_manual"), [this]() { connectionAutoDetermineSpeedCheck->turn(!connectionSetSpeedManualCheck->turned()); UpdateSpeedInputs(); }, wui::button_view::radio)),
    connectionInSpeedText(new wui::text(wui::locale("settings", "input_network_speed"))),
    connectionInSpeedInput(new wui::input()),
    connectionOutSpeedText(new wui::text(wui::locale("settings", "output_network_speed"))),
    connectionOutSpeedInput(new wui::input()),
    connectionDetermineSpeedNowButton(new wui::button(wui::locale("settings", "determine_network_speed"), std::bind(&SettingsDialog::DetermineSpeedNow, this))),
    connectionCheckConnectivityNowButton(new wui::button(wui::locale("settings", "check_connectivity"), std::bind(&SettingsDialog::CheckConnectivityNow, this))),
    connectionServer(UNCHANGED), connectionLogin(UNCHANGED), connectionPassword(UNCHANGED), connectionInSpeed(UNCHANGED), connectionOutSpeed(UNCHANGED),

    prefTimerCheck(new wui::button(wui::locale("settings", "show_timer"), []() {}, wui::button_view::switcher)),
    prefAutoAnswerCheck(new wui::button(wui::locale("settings", "auto_answer"), []() {}, wui::button_view::switcher)),
    prefAutoConnectToConfCheck(new wui::button(wui::locale("settings", "auto_connect_to_conference"), []() {}, wui::button_view::switcher)),
    prefShowAvatarCheck(new wui::button(wui::locale("settings", "show_avatars"), []() {}, wui::button_view::switcher)),
    prefRequestTurnCamMicCheck(new wui::button(wui::locale("settings", "request_turn_cam_mic"), []() {}, wui::button_view::switcher)),
    prefLanguageText(new wui::text(wui::locale("settings", "language"))),
    prefLanguageSelect(new wui::select()),
    prefAutoStartAppCheck(new wui::button(wui::locale("settings", "auto_start_app"), []() {}, wui::button_view::switcher)),
    prefHideToTrayCheck(new wui::button(wui::locale("settings", "hide_to_tray"), []() {}, wui::button_view::switcher)),
    prefAccentuateSpeakerCheck(new wui::button(wui::locale("settings", "accentuate_speaker"), []() {}, wui::button_view::switcher)),
    prefUseDemonstrationWindowCheck(new wui::button(wui::locale("settings", "use_demonstration_window"), []() {}, wui::button_view::switcher)),

    recordPathText(new wui::text(wui::locale("settings", "record_path"))),
    recordPathInput(new wui::input()),
    recordPathButton(new wui::button("...", std::bind(&SettingsDialog::SelectPath, this))),
    recordMP3Check(new wui::button(wui::locale("settings", "only_mp3_sound"), []() {}, wui::button_view::switcher)),
    recordPath(UNCHANGED)
{
    window->set_transient_for(transientWindow.lock());

    sectionList->set_item_height_callback([](int32_t, int32_t& h) { h = XBITMAP; });
    sectionList->set_draw_callback(std::bind(&SettingsDialog::DrawListItem, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    sectionList->set_item_change_callback(std::bind(&SettingsDialog::ChangeListItem, this, std::placeholders::_1));
    sectionList->set_item_count(7);
    sectionList->select_item(-1);

    cameraSelect->set_change_callback(std::bind(&SettingsDialog::ChangeCamera, this, std::placeholders::_1, std::placeholders::_2));
    cameraResolutionSelect->set_change_callback(std::bind(&SettingsDialog::ChangeResouliton, this, std::placeholders::_1, std::placeholders::_2));
    cameraErrorText->update_theme(MakeErrorTextTheme());

    captureVideoSession.SetBitrate(8096);
    captureVideoSession.SetFrameRate(25);
    captureVideoSession.SetLocalReceiver(rendererVideoSession.GetDirectReceiver());
    captureVideoSession.SetDeviceNotifyCallback(std::bind(&SettingsDialog::ReceiveDeviceNotify, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    
    rendererVideoSession.SetMirrorVideo(true);
    rendererVideoSession.SetMy(true);

    microphoneSelect->set_change_callback(std::bind(&SettingsDialog::ChangeMicrophone, this, std::placeholders::_1, std::placeholders::_2));
    microphoneAECCheck->turn(wui::config::get_int("CaptureDevices", "MicrophoneAEC", 1) != 0);
    microphoneNSCheck->turn(wui::config::get_int("CaptureDevices", "MicrophoneNS", 1) != 0);
    microphoneAGCCheck->turn(wui::config::get_int("CaptureDevices", "MicrophoneAGC", 1) != 0);
    microphone16SampleRateCheck->turn(wui::config::get_int("CaptureDevices", "MicrophoneSampleFreq", 48000) == 16000);
    microphone48SampleRateCheck->turn(wui::config::get_int("CaptureDevices", "MicrophoneSampleFreq", 48000) == 48000);

    loudspeakerSelect->set_change_callback(std::bind(&SettingsDialog::ChangeLoudspeaker, this, std::placeholders::_1, std::placeholders::_2));
    loudspeaker16SampleRateCheck->turn(wui::config::get_int("AudioRenderer", "SampleFreq", 48000) == 16000);
    loudspeaker48SampleRateCheck->turn(wui::config::get_int("AudioRenderer", "SampleFreq", 48000) == 48000);

    userNameInput->set_change_callback([this](std::string_view v) { userName = v; });
    userLoginInput->set_change_callback([this](std::string_view v) { userLogin = v; });
    userPasswordInput->set_change_callback([this](std::string_view v) { userPassword = v; });
    userPasswordConfirmInput->set_change_callback([this](std::string_view v) { userPasswordConfirm = v; });

    connectionServerInput->set_change_callback([this](std::string_view v) { connectionServer = v; });
    connectionLoginInput->set_change_callback([this](std::string_view v) { connectionLogin = v; });
    connectionPasswordInput->set_change_callback([this](std::string_view v) { connectionPassword = v; });
    connectionInSpeedInput->set_change_callback([this](std::string_view v) { connectionInSpeed = v; });
    connectionOutSpeedInput->set_change_callback([this](std::string_view v) { connectionOutSpeed = v; });
    connectionAutoDetermineSpeedCheck->turn(wui::config::get_int("User", "AutoDetermineNetSpeed", 1) != 0);
    connectionSetSpeedManualCheck->turn(wui::config::get_int("User", "AutoDetermineNetSpeed", 1) == 0);

    prefTimerCheck->turn(wui::config::get_int("User", "ShowTimer", 1) != 0);
    prefAutoAnswerCheck->turn(wui::config::get_int("User", "CallAutoAnswer", 0) != 0);
    prefAutoConnectToConfCheck->turn(wui::config::get_int("User", "AutoConnectToConf", 1) != 0);
    prefShowAvatarCheck->turn(wui::config::get_int("User", "OutputAvatar", 1) != 0);
    prefRequestTurnCamMicCheck->turn(wui::config::get_int("User", "RequestTurnCamMic", 1) != 0);
    prefLanguageSelect->set_items({ 
        { static_cast<int32_t>(wui::locale_type::eng), wui::locale("settings", "lang_en") },
        { static_cast<int32_t>(wui::locale_type::rus), wui::locale("settings", "lang_ru") },
        { static_cast<int32_t>(wui::locale_type::kaz), wui::locale("settings", "lang_kz") } });
    prefLanguageSelect->select_item_id(wui::config::get_int("User", "Locale", static_cast<int32_t>(wui::locale_type::rus)));
    prefAutoStartAppCheck->turn(wui::config::get_int("User", "AutoRunApp", 1) != 0);
    prefHideToTrayCheck->turn(wui::config::get_int("MainFrame", "HideToTray", 1) != 0);
    prefAccentuateSpeakerCheck->turn(wui::config::get_int("User", "AccentuateSpeakerRenderer", 1) != 0);
    prefUseDemonstrationWindowCheck->turn(wui::config::get_int("User", "UseDemonstrationWindow", 1) != 0);

    recordPathInput->set_change_callback([this](std::string_view v) { recordPath = v; });
    recordMP3Check->turn(wui::config::get_int("Record", "MP3Mode", 0) != 0);
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::Run(SettingsSection section_)
{
    sectionList->select_item(static_cast<int32_t>(section_));
    
    sectionList->update_theme();

    window->add_control(sectionList, { 10, 30, 200, WND_HEIGHT - 45 });
    window->add_control(okButton, { WND_WIDTH - 230, WND_HEIGHT - 35, WND_WIDTH - 130, WND_HEIGHT - 10 });
    window->add_control(cancelButton, { WND_WIDTH - 110, WND_HEIGHT - 35, WND_WIDTH - 10, WND_HEIGHT - 10 });

    window->set_default_push_control(okButton);

    cameraImg->update_theme();
    microphoneImg->update_theme();
    loudspeakerImg->update_theme();
    accountImg->update_theme();
    connectionImg->update_theme();
    preferencesImg->update_theme();
    recordImg->update_theme();
    noSignalImg->update_theme();

    sectionList->update_theme();
    okButton->update_theme();
    cancelButton->update_theme();

    window->init(wui::locale("settings", "title"), { -1, -1, WND_WIDTH, WND_HEIGHT }, wui::window_style::dialog, [this]() {
        HideCamera();
        HideMicrophone();
        HideLoudspeaker();
        HideAccount();
        HideConnection();
        HidePreferences();
        HideRecord();

        currentCameraId = -1;
        currentCameraResolutionId = -1;

        currentMicrophoneId = -1;
        microphoneAECCheck->turn(wui::config::get_int("CaptureDevices", "MicrophoneAEC", 1) != 0);
        microphoneNSCheck->turn(wui::config::get_int("CaptureDevices", "MicrophoneNS", 1) != 0);
        microphoneAGCCheck->turn(wui::config::get_int("CaptureDevices", "MicrophoneAGC", 1) != 0);
        microphone16SampleRateCheck->turn(wui::config::get_int("CaptureDevices", "MicrophoneSampleFreq", 48000) == 16000);
        microphone48SampleRateCheck->turn(wui::config::get_int("CaptureDevices", "MicrophoneSampleFreq", 48000) == 48000);

        currentLoudspeakerId = -1;

        userName = UNCHANGED;
        userLogin = UNCHANGED;
        userPassword = UNCHANGED;
        userPasswordConfirm = UNCHANGED;

        connectionServer = UNCHANGED;
        connectionLogin = UNCHANGED;
        connectionPassword = UNCHANGED;
        connectionInSpeed = UNCHANGED;
        connectionOutSpeed = UNCHANGED;

        prefTimerCheck->turn(wui::config::get_int("User", "ShowTimer", 1) != 0);
        prefAutoAnswerCheck->turn(wui::config::get_int("User", "CallAutoAnswer", 0) != 0);
        prefAutoConnectToConfCheck->turn(wui::config::get_int("User", "AutoConnectToConf", 1) != 0);
        prefShowAvatarCheck->turn(wui::config::get_int("User", "OutputAvatar", 1) != 0);
        prefRequestTurnCamMicCheck->turn(wui::config::get_int("User", "RequestTurnCamMic", 1) != 0);
        prefLanguageSelect->select_item_id(wui::config::get_int("User", "Locale", static_cast<int32_t>(wui::locale_type::rus)));
        prefAutoStartAppCheck->turn(wui::config::get_int("User", "AutoRunApp", 1) != 0);
        prefHideToTrayCheck->turn(wui::config::get_int("MainFrame", "HideToTray", 1) != 0);
        prefAccentuateSpeakerCheck->turn(wui::config::get_int("User", "AccentuateSpeakerRenderer", 1) != 0);
        prefUseDemonstrationWindowCheck->turn(wui::config::get_int("User", "UseDemonstrationWindow", 1) != 0);

        recordPath = UNCHANGED;
        recordMP3Check->turn(wui::config::get_int("Record", "MP3Mode", 0) != 0);

        sectionList->select_item(-1);
    });
}

std::string SettingsDialog::GetLogin()
{
    return userLoginInput->text();
}

std::string SettingsDialog::GetPassword()
{
    return userPasswordInput->text();
}

void SettingsDialog::ResetUserName()
{
    userName = UNCHANGED;
}

void SettingsDialog::ResetUserLogin()
{
    userLogin = UNCHANGED;
}

void SettingsDialog::UpdateCameras()
{
    if (static_cast<SettingsSection>(sectionList->selected_item()) == SettingsSection::Camera)
    {
        currentCameraId = -1;
        currentCameraResolutionId = -1;
        ShowCamera();
    }
}

void SettingsDialog::UpdateMicrophones()
{
    if (static_cast<SettingsSection>(sectionList->selected_item()) == SettingsSection::Microphone)
    {
        currentMicrophoneId = -1;
        ShowMicrophone();
    }
}

void SettingsDialog::UpdateLoudspeakers()
{
    if (static_cast<SettingsSection>(sectionList->selected_item()) == SettingsSection::Loudspeaker)
    {
        currentLoudspeakerId = -1;
        ShowLoudspeaker();
    }
}

void SettingsDialog::OK()
{
    if (!UpdateCamera()) return;
    if (!UpdateMicrophone()) return;
    if (!UpdateLoudspeaker()) return;
    if (!UpdateAccount()) return;
    if (!UpdateConnection()) return;
    if (!UpdatePreferences()) return;
    if (!UpdateRecord()) return;

    window->destroy();
    
    readyCallback();
}

void SettingsDialog::DrawListItem(wui::graphic &gr, int32_t nItem, const wui::rect &itemRect, wui::list::item_state state)
{
    std::string name;
    std::shared_ptr<wui::image> image;

    switch (static_cast<SettingsSection>(nItem))
    {
        case SettingsSection::Camera:
            name = wui::locale("settings", "camera");
            image = cameraImg;
        break;
        case SettingsSection::Microphone:
            name = wui::locale("settings", "microphone");
            image = microphoneImg;
        break;
        case SettingsSection::Loudspeaker:
            name = wui::locale("settings", "loudspeaker");
            image = loudspeakerImg;
        break;
        case SettingsSection::Account:
            name = wui::locale("settings", "account");
            image = accountImg;
        break;
        case SettingsSection::Connection:
            name = wui::locale("settings", "connection");
            image = connectionImg;
        break;
        case SettingsSection::Preferences:
            name = wui::locale("settings", "preferences");
            image = preferencesImg;
        break;
        case SettingsSection::Record:
            name = wui::locale("settings", "record");
            image = recordImg;
        break;
    }

    auto border_width = wui::theme_dimension(wui::list::tc, wui::list::tv_border_width);

    if (state == wui::list::item_state::active)
    {
        gr.draw_rect(itemRect, wui::theme_color(wui::list::tc, wui::list::tv_active_item));
    }
    else if (state == wui::list::item_state::selected)
    {
        gr.draw_rect(itemRect, wui::theme_color(wui::list::tc, wui::list::tv_selected_item));
    }

    auto textColor = wui::theme_color(wui::input::tc, wui::input::tv_text);
    auto font = wui::theme_font(wui::list::tc, wui::list::tv_font);

    auto textRect = itemRect;
    textRect.move(border_width + XBITMAP + 6, (itemRect.height() - gr.measure_text("Qq", font).height()) / 2);
    textRect.right -= border_width + XBITMAP + 6;

    truncate_line(name, gr, font, textRect.width(), 4);

    gr.draw_text(textRect, name, textColor, font);

    image->set_position({ itemRect.left,
        itemRect.top,
        itemRect.left + XBITMAP,
        itemRect.top + XBITMAP });
    image->draw(gr, { 0 });
}

void SettingsDialog::ChangeListItem(int32_t nItem)
{
    switch (static_cast<SettingsSection>(nItem))
    {
        case SettingsSection::Camera:
            HideMicrophone();
            HideLoudspeaker();
            HideAccount();
            HideConnection();
            HidePreferences();
            HideRecord();

            ShowCamera();
        break;
        case SettingsSection::Microphone:
            HideCamera();
            HideLoudspeaker();
            HideAccount();
            HideConnection();
            HidePreferences();
            HideRecord();

            ShowMicrophone();
        break;
        case SettingsSection::Loudspeaker:
            HideCamera();
            HideMicrophone();
            HideAccount();
            HideConnection();
            HidePreferences();
            HideRecord();

            ShowLoudspeaker();
        break;
        case SettingsSection::Account:
            HideCamera();
            HideMicrophone();
            HideLoudspeaker();
            HideConnection();
            HidePreferences();
            HideRecord();

            ShowAccount();
        break;
        case SettingsSection::Connection:
            HideCamera();
            HideMicrophone();
            HideLoudspeaker();
            HideAccount();
            HidePreferences();
            HideRecord();

            ShowConnection();
        break;
        case SettingsSection::Preferences:
            HideCamera();
            HideMicrophone();
            HideLoudspeaker();
            HideAccount();
            HideConnection();
            HideRecord();

            ShowPreferences();
        break;
        case SettingsSection::Record:
            HideCamera();
            HideMicrophone();
            HideLoudspeaker();
            HideAccount();
            HideConnection();
            HidePreferences();

            ShowRecord();
        break;
    }
}

/// Camera settings

void SettingsDialog::ShowCamera()
{
    wui::select_items_t cameraItems;
    cameraDevices.clear();
    LoadCameras(cameraDevices);
    int32_t i = 0;
    for (const auto &camera : cameraDevices)
    {
        if (camera.type == DeviceType::Camera)
        {
            cameraItems.push_back({ i, camera.name });

            if (currentCameraId == -1 && camera.name == wui::config::get_string("CaptureDevices", "CameraName", ""))
            {
                currentCameraId = i;
            }
            ++i;
        }
    }

    cameraSelect->set_items(cameraItems);
    cameraSelect->select_item_id(currentCameraId);
    
    window->add_control(noSignalImg, { 210, 30, WND_WIDTH - 10, 280 });
    window->add_control(rendererVideoSession.GetControl(), { 210, 30, WND_WIDTH - 10, 280 });
    window->add_control(cameraSelect, { 210, 290, WND_WIDTH - 10, 315 });
    window->add_control(cameraResolutionText, { 210, 330, 310, 350 });
    window->add_control(cameraResolutionSelect, { 315, 325, WND_WIDTH - 10, 350 });
    window->add_control(cameraErrorText, { 210, 360, WND_WIDTH - 10, 395 });

    window->set_focused(cameraSelect);

    cameraSelect->update_theme();
    cameraResolutionSelect->update_theme();

    rendererVideoSession.Start(0, 0, 0, "");

    redrawVideoTimer.start(40);
}

void SettingsDialog::HideCamera()
{
    redrawVideoTimer.stop();

    captureVideoSession.Stop();
    rendererVideoSession.Stop();
    
    window->remove_control(noSignalImg);
    window->remove_control(rendererVideoSession.GetControl());
    window->remove_control(cameraSelect);
    window->remove_control(cameraResolutionText);
    window->remove_control(cameraResolutionSelect);
    window->remove_control(cameraErrorText);
}

void SettingsDialog::ChangeCamera(int32_t nItem, int64_t id)
{
    if (nItem > static_cast<int32_t>(cameraDevices.size()))
    {
        return;
    }

    cameraErrorText->set_text("");

    currentCameraId = id;

    auto device = cameraDevices[nItem];

    FillResolutions(device);

    rendererVideoSession.GetControl()->show();

    captureVideoSession.Stop();
    captureVideoSession.SetName(device.name);
    captureVideoSession.Start(0, device.formats[static_cast<uint32_t>(cameraResolutionSelect->selected_item().id)].colorSpaces[0], 0, "");
}

void SettingsDialog::ChangeResouliton(int32_t nItem, int64_t id)
{
    if (cameraDevices.empty())
    {
        return;
    }

    currentCameraResolutionId = id;

    auto &camera = cameraDevices[static_cast<uint32_t>(cameraSelect->selected_item().id)];
    auto resolution = camera.formats[nItem].resolution;

    captureVideoSession.SetResolution(resolution);
    rendererVideoSession.SetResolution(resolution);
}

void SettingsDialog::FillResolutions(const Device &device)
{
    wui::select_items_t resolutionItems;

    auto resolution = (Video::Resolution)wui::config::get_int("CaptureDevices", "CameraResolution",
        GetBestResolution(device.formats, wui::config::get_int("User", "Flops", 0) < 400000000 ? 240 : 480));

    currentCameraResolutionId = 0;

    int32_t i = 0;
    for (auto &format : device.formats)
    {
        Video::ResolutionValues rv = Video::GetValues(format.resolution);
        std::string strResolution = std::to_string(rv.width) + "x" + std::to_string(rv.height);

        resolutionItems.push_back({ i, strResolution });
        
        if (resolution == format.resolution)
        {
            currentCameraResolutionId = i;
        }

        ++i;
    }

    cameraResolutionSelect->set_items(resolutionItems);
    cameraResolutionSelect->select_item_id(currentCameraResolutionId);
}

void SettingsDialog::RedrawVideo()
{
    wui::rect pos = { 0 };
    if (window->parent().lock())
    {
        pos = window->position();
    }
    window->redraw({ pos.left + 210, pos.top + 30, pos.left + WND_WIDTH - 10, pos.top + 280 });
}

void SettingsDialog::ReceiveDeviceNotify(std::string_view name, DeviceNotifyType notifyType, Proto::DeviceType deviceType, uint32_t deviceId, int32_t iData)
{
    if (notifyType == DeviceNotifyType::CameraError)
    {
        rendererVideoSession.GetControl()->hide();
        cameraErrorText->set_text(wui::locale("message", "camera_busy_error"));
    }
}

bool SettingsDialog::UpdateCamera()
{
    if (!cameraSelect->items().empty())
    {
        auto &camera = cameraDevices[static_cast<uint32_t>(cameraSelect->selected_item().id)];
        wui::config::set_string("CaptureDevices", "CameraName", camera.name);
        wui::config::set_int("CaptureDevices", "CameraResolution", camera.formats[static_cast<uint32_t>(cameraResolutionSelect->selected_item().id)].resolution);
    }

    return true;
}

/// Microphone settings

void SettingsDialog::ShowMicrophone()
{
    wui::select_items_t microphoneItems;
    microphoneDevices.clear();
    LoadMicrophones(microphoneDevices);
    int32_t i = 0;
    for (const auto &microphone : microphoneDevices)
    {
        microphoneItems.push_back({ i, microphone.name });

        if (currentMicrophoneId == -1 && microphone.name == wui::config::get_string("CaptureDevices", "MicrophoneName", ""))
        {
            currentMicrophoneId = i;
        }
        ++i;
    }

    microphoneSelect->set_items(microphoneItems);
    microphoneSelect->select_item_id(currentMicrophoneId);
    microphoneSelect->update_theme();

    microphoneAECCheck->update_theme();
    microphoneNSCheck->update_theme();
    microphoneAGCCheck->update_theme();
    microphone16SampleRateCheck->update_theme();
    microphone48SampleRateCheck->update_theme();

    microphoneAECCheck->turn(wui::config::get_int("CaptureDevices", "MicrophoneAEC", 1) != 0);
    microphoneNSCheck->turn(wui::config::get_int("CaptureDevices", "MicrophoneNS", 1) != 0);
    microphoneAGCCheck->turn(wui::config::get_int("CaptureDevices", "MicrophoneAGC", 1) != 0);
    microphone16SampleRateCheck->turn(wui::config::get_int("CaptureDevices", "MicrophoneSampleFreq", 48000) == 16000);
    microphone48SampleRateCheck->turn(wui::config::get_int("CaptureDevices", "MicrophoneSampleFreq", 48000) == 48000);

    wui::rect pos = { 210, 30, WND_WIDTH - 10, 130 };
    window->add_control(soundIndicator, pos);
    wui::line_up_top_bottom(pos, 25, 10);
    window->add_control(microphoneSelect, pos);
    wui::line_up_top_bottom(pos, 25, 10);
    pos.right = 370;
    window->add_control(microphoneSensitivityText, pos);
    pos.left = 380;
    pos.right = WND_WIDTH - 10;
    window->add_control(microphoneSensitivitySlider, pos);
    pos.left = 210;
    wui::line_up_top_bottom(pos, 20, 10);
    window->add_control(microphoneAECCheck, pos);
    wui::line_up_top_bottom(pos, 20, 10);
    window->add_control(microphoneNSCheck, pos);
    wui::line_up_top_bottom(pos, 20, 10);
    window->add_control(microphoneAGCCheck, pos);
    wui::line_up_top_bottom(pos, 20, 20);
    window->add_control(microphone16SampleRateCheck, pos);
    wui::line_up_top_bottom(pos, 20, 10);
    window->add_control(microphone48SampleRateCheck, pos);

    window->set_focused(microphoneSelect);
}

void SettingsDialog::HideMicrophone()
{
    microphone.Stop();

    window->remove_control(soundIndicator);
    window->remove_control(microphoneSelect);
    window->remove_control(microphoneSensitivityText);
    window->remove_control(microphoneSensitivitySlider);
    window->remove_control(microphoneAECCheck);
    window->remove_control(microphoneNSCheck);
    window->remove_control(microphoneAGCCheck);
    window->remove_control(microphone16SampleRateCheck);
    window->remove_control(microphone48SampleRateCheck);
}

void SettingsDialog::ChangeMicrophone(int32_t nItem, int64_t id)
{
    if (nItem > static_cast<int32_t>(microphoneDevices.size()))
    {
        return;
    }

    currentMicrophoneId = id;

    auto device = microphoneDevices[nItem];
    microphone.Stop();
    microphone.SetDeviceName(device.name);
    microphone.SetGain(microphoneSensitivitySlider->get_value());
    microphone.Start();
}

bool SettingsDialog::UpdateMicrophone()
{
    if (!microphoneSelect->items().empty())
    {
        wui::config::set_string("CaptureDevices", "MicrophoneName", microphoneSelect->selected_item().text);
        wui::config::set_int("CaptureDevices", "MicrophoneGain", microphoneSensitivitySlider->get_value());
        wui::config::set_int("CaptureDevices", "MicrophoneAEC", microphoneAECCheck->turned());
        wui::config::set_int("CaptureDevices", "MicrophoneNS", microphoneNSCheck->turned());
        wui::config::set_int("CaptureDevices", "MicrophoneAGC", microphoneAGCCheck->turned());
        wui::config::set_int("CaptureDevices", "MicrophoneSampleFreq", microphone16SampleRateCheck->turned() ? 16000 : 48000);
    }

    return true;
}

/// Loudspeaker settings

void SettingsDialog::ShowLoudspeaker()
{
    wui::select_items_t loudspeakerItems;   
    int32_t i = 0;

    auto devices = audioRenderer.GetSoundRenderers();

    for (const auto &device : devices)
    {
        loudspeakerItems.push_back({ i, device });

        if (currentLoudspeakerId == -1 && device == wui::config::get_string("AudioRenderer", "Speaker", ""))
        {
            currentLoudspeakerId = i;
        }
        ++i;
    }

    loudspeakerSelect->set_items(loudspeakerItems);
    loudspeakerSelect->select_item_id(currentLoudspeakerId);
    loudspeakerSelect->update_theme();

    loudspeaker16SampleRateCheck->update_theme();
    loudspeaker48SampleRateCheck->update_theme();

    loudspeakerSlider->update_theme();
    loudspeakerCheckButton->update_theme();

    wui::rect pos = { 210, 30, WND_WIDTH - 10, 55 };
    window->add_control(loudspeakerSelect, pos);
    wui::line_up_top_bottom(pos, 25, 10);
    pos.right = 350;
    window->add_control(loudspeakerVolumeText, pos);
    pos.left = 360; pos.right = WND_WIDTH - 10;
    window->add_control(loudspeakerSlider, pos);
    pos.left = 210;
    wui::line_up_top_bottom(pos, 25, 10);
    pos.right = 380;
    window->add_control(loudspeakerCheckButton, pos);
    
    pos.right = WND_WIDTH - 10;
    wui::line_up_top_bottom(pos, 20, 30);
    window->add_control(loudspeaker16SampleRateCheck, pos);
    wui::line_up_top_bottom(pos, 20, 10);
    window->add_control(loudspeaker48SampleRateCheck, pos);

    window->set_focused(loudspeakerSelect);
}

void SettingsDialog::HideLoudspeaker()
{
    window->remove_control(loudspeakerSelect);
    window->remove_control(loudspeakerVolumeText);
    window->remove_control(loudspeakerSlider);
    window->remove_control(loudspeakerCheckButton);
    window->remove_control(loudspeaker16SampleRateCheck);
    window->remove_control(loudspeaker48SampleRateCheck);
}

void SettingsDialog::ChangeLoudspeaker(int32_t nItem, int64_t id)
{
    auto devices = audioRenderer.GetSoundRenderers();

    if (nItem > static_cast<int32_t>(devices.size()))
    {
        return;
    }

    currentLoudspeakerId = id;

    audioRenderer.SetDeviceName(devices[nItem]);
}

void SettingsDialog::ChangeLoudspeakerSampleRate(int32_t sampleRate)
{
    audioMixer.Stop();
    audioRenderer.Stop();
    resampler.SetSampleFreq(48000, sampleRate);
    audioRenderer.Start(sampleRate);
    audioMixer.Start();

    wui::config::set_int("AudioRenderer", "SampleFreq", sampleRate);
}

bool SettingsDialog::UpdateLoudspeaker()
{
    if (!loudspeakerSelect->items().empty())
    {
        wui::config::set_string("AudioRenderer", "Speaker", loudspeakerSelect->selected_item().text);
        wui::config::set_int("AudioRenderer", "Volume", loudspeakerSlider->get_value());
    }

    return true;
}

/// Account settings

void SettingsDialog::ShowAccount()
{
    userNameInput->update_theme();
    userLoginInput->update_theme();
    userPasswordInput->update_theme();
    userPasswordConfirmInput->update_theme();

    if (userName == UNCHANGED) userName = controller.GetMyClientName();
    if (userLogin == UNCHANGED) userLogin = controller.GetLogin();
    if (userPassword == UNCHANGED) userPassword = controller.GetPassword();
    if (userPasswordConfirm == UNCHANGED) userPasswordConfirm = userPassword;

    userNameInput->set_text(userName);
    userLoginInput->set_text(userLogin);
    userPasswordInput->set_text(userPassword);
    userPasswordConfirmInput->set_text(userPasswordConfirm);

    window->add_control(userNameText, { 210, 30, WND_WIDTH - 10, 45 });
    window->add_control(userNameInput, { 210, 55, WND_WIDTH - 10, 80 });
    window->add_control(userChangeChandentialsLink, { 210, WND_HEIGHT - 185, WND_WIDTH - 80, WND_HEIGHT - 170 });
    window->add_control(userLoginText, { 210, WND_HEIGHT - 150, 320, WND_HEIGHT - 125 });
    window->add_control(userLoginInput, { 330, WND_HEIGHT - 150, WND_WIDTH - 80, WND_HEIGHT - 125 });
    window->add_control(userPasswordText, { 210, WND_HEIGHT - 105, 320, WND_HEIGHT - 80 });
    window->add_control(userPasswordInput, { 330, WND_HEIGHT - 105, WND_WIDTH - 80, WND_HEIGHT - 80 });
    window->add_control(userPasswordConfirmText, { 210, WND_HEIGHT - 70, 320, WND_HEIGHT - 45 });
    window->add_control(userPasswordConfirmInput, { 330, WND_HEIGHT - 70, WND_WIDTH - 80, WND_HEIGHT - 45 });

    window->set_focused(userNameInput);

    userLoginText->hide();
    userLoginInput->hide();
    userPasswordText->hide();
    userPasswordInput->hide();
    userPasswordConfirmText->hide();
    userPasswordConfirmInput->hide();
    userChangeChandentialsLink->set_caption(wui::locale("settings", "show_change_chandentials"));
}

void SettingsDialog::HideAccount()
{
    window->remove_control(userNameText);
    window->remove_control(userNameInput);
    window->remove_control(userChangeChandentialsLink);
    window->remove_control(userLoginText);
    window->remove_control(userLoginInput);
    window->remove_control(userPasswordText);
    window->remove_control(userPasswordInput);
    window->remove_control(userPasswordConfirmText);
    window->remove_control(userPasswordConfirmInput);
}

void SettingsDialog::ShowChangeChandentials()
{
    if (userLoginText->showed())
    {
        userLoginText->hide();
        userLoginInput->hide();
        userPasswordText->hide();
        userPasswordInput->hide();
        userPasswordConfirmText->hide();
        userPasswordConfirmInput->hide();
        userChangeChandentialsLink->set_caption(wui::locale("settings", "show_change_chandentials"));

        window->set_focused(userNameInput);
    }
    else
    {
        userLoginText->show();
        userLoginInput->show();
        userPasswordText->show();
        userPasswordInput->show();
        userPasswordConfirmText->show();
        userPasswordConfirmInput->show();
        userChangeChandentialsLink->set_caption(wui::locale("settings", "hide_change_chandentials"));

        window->set_focused(userLoginInput);
    }
}

bool SettingsDialog::UpdateAccount()
{
    bool hasChanges = false;
    if (userName != UNCHANGED && userName != controller.GetMyClientName())
    {
        if (userName.empty())
        {
            messageBox->show(wui::locale("message", "user_name_empty"),
                wui::locale("message", "title_error"),
                wui::message_icon::alert,
                wui::message_button::ok, [this](wui::message_result) {
                sectionList->select_item(static_cast<int32_t>(SettingsSection::Account));
                window->set_focused(userNameInput);
            });

            return false;
        }

        hasChanges = true;
    }

    if (userLogin != UNCHANGED && userLogin != controller.GetLogin())
    {
        if (userLogin.empty())
        {
            messageBox->show(wui::locale("message", "user_login_empty"),
                wui::locale("message", "title_error"),
                wui::message_icon::alert,
                wui::message_button::ok, [this](wui::message_result) {
                sectionList->select_item(static_cast<int32_t>(SettingsSection::Account));
                window->set_focused(userLoginInput);
            });

            return false;
        }

        hasChanges = true;
    }

    if (userPassword != UNCHANGED && userPassword != controller.GetPassword())
    {
        if (userPassword.empty())
        {
            messageBox->show(wui::locale("message", "user_password_empty"),
                wui::locale("message", "title_error"),
                wui::message_icon::alert,
                wui::message_button::ok, [this](wui::message_result) {
                sectionList->select_item(static_cast<int32_t>(SettingsSection::Account));
                window->set_focused(userPasswordInput);
            });

            return false;
        }

        if (userPassword != userPasswordConfirm)
        {
            messageBox->show(wui::locale("message", "user_password_not_eq_confirm"),
                wui::locale("message", "title_error"),
                wui::message_icon::alert,
                wui::message_button::ok, [this](wui::message_result) {
                sectionList->select_item(static_cast<int32_t>(SettingsSection::Account));
                window->set_focused(userPasswordConfirmInput);
            });

            return false;
        }

        hasChanges = true;
    }

    if (hasChanges)
    {
        controller.UserUpdate(controller.GetMyClientId(), userName, "", userLogin, userPassword);
    }

    return true;
}

/// Connection settings

void SettingsDialog::ShowConnection()
{
    connectionServerInput->update_theme();
    connectionLoginInput->update_theme();
    connectionPasswordInput->update_theme();
    connectionAutoDetermineSpeedCheck->update_theme();
    connectionSetSpeedManualCheck->update_theme();
    connectionInSpeedInput->update_theme();
    connectionOutSpeedInput->update_theme();
    connectionDetermineSpeedNowButton->update_theme();
    connectionCheckConnectivityNowButton->update_theme();

    if (connectionServer == UNCHANGED) connectionServer = (wui::config::get_int("Connection", "Secure", 0) != 0 ? "https://" : "http://") + wui::config::get_string("Connection", "Address", "");
    if (connectionLogin == UNCHANGED) connectionLogin = wui::config::get_string("Credentials", "Login", "");
    if (connectionPassword == UNCHANGED) connectionPassword = wui::config::get_string("Credentials", "Password", "");
    if (connectionInSpeed == UNCHANGED) connectionInSpeed = std::to_string(wui::config::get_int("User", "MaxInputBitrate", 0));
    if (connectionOutSpeed == UNCHANGED) connectionOutSpeed = std::to_string(wui::config::get_int("User", "MaxOutputBitrate", 0));

    connectionServerInput->set_text(connectionServer);
    connectionLoginInput->set_text(connectionLogin);
    connectionPasswordInput->set_text(connectionPassword);
    connectionInSpeedInput->set_text(connectionInSpeed);
    connectionOutSpeedInput->set_text(connectionOutSpeed);

    UpdateSpeedInputs();

    window->add_control(connectionServerText, { 210, 30, 290, 55 });
    window->add_control(connectionServerInput, { 300, 30, WND_WIDTH - 10, 55 });
    window->add_control(connectionLoginText, { 210, 65, 290, 90 });
    window->add_control(connectionLoginInput, { 300, 65, WND_WIDTH - 80, 90 });
    window->add_control(connectionPasswordText, { 210, 100, 290, 125 });
    window->add_control(connectionPasswordInput, { 300, 100, WND_WIDTH - 80, 125 });
    window->add_control(connectionAutoDetermineSpeedCheck, { 210, 170, WND_WIDTH - 10, 185 });
    window->add_control(connectionSetSpeedManualCheck, { 210, 195, WND_WIDTH - 10, 210 });
    window->add_control(connectionInSpeedText, { 210, 225, 330, 250 });
    window->add_control(connectionInSpeedInput, { 340, 225, WND_WIDTH - 80, 250 });
    window->add_control(connectionOutSpeedText, { 210, 260, 330, 285 });
    window->add_control(connectionOutSpeedInput, { 340, 260, WND_WIDTH - 80, 285 });
    window->add_control(connectionDetermineSpeedNowButton, { 340, 295, WND_WIDTH - 80, 320 });
    window->add_control(connectionCheckConnectivityNowButton, { 340, 330, WND_WIDTH - 80, 355 });

    window->set_focused(connectionServerInput);
}

void SettingsDialog::HideConnection()
{
    window->remove_control(connectionServerText);
    window->remove_control(connectionServerInput);
    window->remove_control(connectionLoginText);
    window->remove_control(connectionLoginInput);
    window->remove_control(connectionPasswordText);
    window->remove_control(connectionPasswordInput);
    window->remove_control(connectionAutoDetermineSpeedCheck);
    window->remove_control(connectionSetSpeedManualCheck);
    window->remove_control(connectionInSpeedText);
    window->remove_control(connectionInSpeedInput);
    window->remove_control(connectionOutSpeedText);
    window->remove_control(connectionOutSpeedInput);
    window->remove_control(connectionDetermineSpeedNowButton);
    window->remove_control(connectionCheckConnectivityNowButton);
}

bool SettingsDialog::UpdateConnection()
{
    if (connectionServer != UNCHANGED)
    {
        std::vector<std::string> vals;
        boost::split(vals, connectionServer, boost::is_any_of("/"));
        if (vals.size() > 2)
        {
            if (vals[0] == "http:")
            {
                wui::config::set_int("Connection", "Secure", 0);
            }
            else if (vals[0] == "https:")
            {
                wui::config::set_int("Connection", "Secure", 1);
            }
            else
            {
                ShowIncorrectURLError();
                return false;
            }

            if (vals[2].empty())
            {
                ShowIncorrectURLError();
                return false;
            }
        }
        else
        {
            ShowIncorrectURLError();
            return false;
        }

        wui::config::set_string("Connection", "Address", vals[2]);
    }

    if (connectionLogin != UNCHANGED) wui::config::set_string("Credentials", "Login", connectionLogin);
    if (connectionPassword != UNCHANGED) wui::config::set_string("Credentials", "Password", connectionPassword);
    wui::config::set_int("User", "AutoDetermineNetSpeed", connectionAutoDetermineSpeedCheck->turned() ? 1 : 0);
    if (connectionInSpeed != UNCHANGED) wui::config::set_int("User", "MaxInputBitrate", atoi(connectionInSpeed.c_str()));
    if (connectionOutSpeed != UNCHANGED) wui::config::set_int("User", "MaxOutputBitrate", atoi(connectionOutSpeed.c_str()));

    return true;
}

void SettingsDialog::ShowIncorrectURLError()
{
    messageBox->show(wui::locale("message", "incorrect_server_url"),
        wui::locale("message", "title_error"),
        wui::message_icon::alert,
        wui::message_button::ok);
}

void SettingsDialog::UpdateSpeedInputs()
{
    if (connectionSetSpeedManualCheck->turned())
    {
        connectionInSpeedInput->enable();
        connectionOutSpeedInput->enable();
    }
    else
    {
        connectionInSpeedInput->disable();
        connectionOutSpeedInput->disable();
    }
}

void SettingsDialog::DetermineSpeedNow()
{
    netSpeedDeterminer(true);

    connectionInSpeed = std::to_string(wui::config::get_int("User", "MaxInputBitrate", 0));
    connectionOutSpeed = std::to_string(wui::config::get_int("User", "MaxOutputBitrate", 0));
    connectionInSpeedInput->set_text(connectionInSpeed);
    connectionOutSpeedInput->set_text(connectionOutSpeed);
}

void SettingsDialog::CheckConnectivityNow()
{
    connectivityDeterminer(true);
}

/// Preferences settings

void SettingsDialog::ShowPreferences()
{
    prefTimerCheck->update_theme();
    prefAutoAnswerCheck->update_theme();
    prefAutoConnectToConfCheck->update_theme();
    prefShowAvatarCheck->update_theme();
    prefRequestTurnCamMicCheck->update_theme();
    prefLanguageText->update_theme();
    prefLanguageSelect->update_theme();
    prefAutoStartAppCheck->update_theme();
    prefHideToTrayCheck->update_theme();
    prefAccentuateSpeakerCheck->update_theme();
    prefUseDemonstrationWindowCheck->update_theme();

    wui::rect pos = { 210, 30, WND_WIDTH - 10, 45 };
    window->add_control(prefTimerCheck, pos);
    wui::line_up_top_bottom(pos, 15, 10);
    window->add_control(prefAutoAnswerCheck, pos);
    wui::line_up_top_bottom(pos, 15, 10);
    window->add_control(prefAutoConnectToConfCheck, pos);
    wui::line_up_top_bottom(pos, 15, 10);
    window->add_control(prefShowAvatarCheck, pos);
    wui::line_up_top_bottom(pos, 25, 10);
    window->add_control(prefRequestTurnCamMicCheck, pos);
    pos.right = 340;
    wui::line_up_top_bottom(pos, 25, 10);
    window->add_control(prefLanguageText, pos);
    pos.left = 350; pos.right = WND_WIDTH - 10;
    window->add_control(prefLanguageSelect, pos);
    pos.left = 210;
    wui::line_up_top_bottom(pos, 15, 10);
    window->add_control(prefAutoStartAppCheck, pos);
    wui::line_up_top_bottom(pos, 15, 10);
    window->add_control(prefHideToTrayCheck, pos);
    wui::line_up_top_bottom(pos, 15, 15);
    window->add_control(prefAccentuateSpeakerCheck, pos);
    wui::line_up_top_bottom(pos, 15, 10);
    window->add_control(prefUseDemonstrationWindowCheck, pos);

    window->set_focused(prefTimerCheck);
}

void SettingsDialog::HidePreferences()
{
    window->remove_control(prefTimerCheck);
    window->remove_control(prefAutoAnswerCheck);
    window->remove_control(prefAutoConnectToConfCheck);
    window->remove_control(prefShowAvatarCheck);
    window->remove_control(prefRequestTurnCamMicCheck);
    window->remove_control(prefLanguageText);
    window->remove_control(prefLanguageSelect);
    window->remove_control(prefAutoStartAppCheck);
    window->remove_control(prefHideToTrayCheck);
    window->remove_control(prefAccentuateSpeakerCheck);
    window->remove_control(prefUseDemonstrationWindowCheck);
}

bool SettingsDialog::UpdatePreferences()
{
    wui::config::set_int("User", "ShowTimer", prefTimerCheck->turned() ? 1 : 0);
    wui::config::set_int("User", "CallAutoAnswer", prefAutoAnswerCheck->turned() ? 1 : 0);
    wui::config::set_int("User", "AutoConnectToConf", prefAutoConnectToConfCheck->turned() ? 1 : 0);
    wui::config::set_int("User", "OutputAvatar", prefShowAvatarCheck->turned() ? 1 : 0);
    wui::config::set_int("User", "RequestTurnCamMic", prefRequestTurnCamMicCheck->turned() ? 1 : 0);
    wui::config::set_int("User", "Locale", static_cast<int32_t>(prefLanguageSelect->selected_item().id));
    wui::config::set_int("User", "AutoRunApp", prefAutoStartAppCheck->turned() ? 1 : 0);
    wui::config::set_int("MainFrame", "HideToTray", prefHideToTrayCheck->turned() ? 1 : 0);
    wui::config::set_int("User", "AccentuateSpeakerRenderer", prefAccentuateSpeakerCheck->turned() ? 1 : 0);
    wui::config::set_int("User", "UseDemonstrationWindow", prefUseDemonstrationWindowCheck->turned() ? 1 : 0);

    return true;
}

/// Record settings

void SettingsDialog::ShowRecord()
{
    std::string recordPath_;
    if (recordPath == UNCHANGED) recordPath_ = wui::config::get_string("Record", "Path", ""); else recordPath_ = recordPath;

#ifdef _WIN32
    if (recordPath_.empty())
    {
        wchar_t myDocuments[MAX_PATH] = { 0 };
        HRESULT result = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, myDocuments);
        if (result == S_OK)
        {
            recordPath_ = boost::nowide::narrow(myDocuments);
        }
    }
#else
    if (recordPath_.empty())
    {
        recordPath_ = getpwuid(getuid())->pw_dir;
    }
#endif
    recordPathInput->set_text(recordPath_);

    recordPathInput->update_theme();
    recordPathButton->update_theme();
    recordMP3Check->update_theme();

    window->add_control(recordPathText, { 210, 30, WND_WIDTH - 10, 45});
    window->add_control(recordPathInput, { 210, 55, WND_WIDTH - 50, 80 });
    window->add_control(recordPathButton, { WND_WIDTH - 40, 55, WND_WIDTH - 10, 80 });
    window->add_control(recordMP3Check, { 210, 90, WND_WIDTH - 10, 90 });

    window->set_focused(recordPathInput);
}

void SettingsDialog::HideRecord()
{
    window->remove_control(recordPathText);
    window->remove_control(recordPathInput);
    window->remove_control(recordPathButton);
    window->remove_control(recordMP3Check);
}

#ifdef WIN32
void SettingsDialog::SelectPath()
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

        recordPathInput->set_text(boost::nowide::narrow(path));
    }
}
#else
void SettingsDialog::SelectPath()
{
}
#endif

bool SettingsDialog::UpdateRecord()
{
    if (!recordPathInput->text().empty())
    {
        wui::config::set_string("Record", "Path", recordPathInput->text());
        wui::config::set_int("Record", "MP3Mode", recordMP3Check->turned());
    }

    return true;
}

}
