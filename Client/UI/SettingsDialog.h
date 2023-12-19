/**
 * SettingsDialog.h - Contains settings dialog header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#pragma once

#include <wui/window/window.hpp>
#include <wui/control/text.hpp>
#include <wui/control/input.hpp>
#include <wui/control/list.hpp>
#include <wui/control/select.hpp>
#include <wui/control/slider.hpp>
#include <wui/control/image.hpp>
#include <wui/control/button.hpp>
#include <wui/control/message.hpp>

#include <Common/TimeMeter.h>
#include <CaptureSession/CaptureVideoSession.h>
#include <RendererSession/RendererVideoSession.h>
#include <Device/Device.h>
#include <Microphone/Microphone.h>
#include <UI/SoundIndicator.h>
#include <UI/Ringer.h>
#include <AudioRenderer/AudioRenderer.h>
#include <Audio/Resampler.h>
#include <Audio/AudioMixer.h>
#include <Controller/IController.h>

#include <string>
#include <memory>
#include <functional>

#include <mt/timer.h>

namespace Client
{

enum class SettingsSection
{
    Camera,
    Microphone,
    Loudspeaker,
    Account,
    Connection,
    Preferences,
    Record
};

class SettingsDialog
{
public:
    SettingsDialog(std::weak_ptr<wui::window> transientWindow,
        Controller::IController &controller,
        AudioRenderer::IAudioRenderer &audioRenderer,
        Audio::Resampler &resampler,
        Audio::AudioMixer &audioMixer,
        Ringer &ringer,
        std::function<void(bool)> netSpeedDeterminer,
        std::function<void(bool)> connectivityDeterminer,
        std::function<void()> readyCallback);
    ~SettingsDialog();

    void Run(SettingsSection section);

    std::string GetLogin();
    std::string GetPassword();
    void ResetUserName();
    void ResetUserLogin();

    void UpdateCameras();
    void UpdateMicrophones();
    void UpdateLoudspeakers();

private:
    static const int32_t WND_WIDTH = 700, WND_HEIGHT = 460;
    static const int32_t XBITMAP = 48;

    std::function<void()> readyCallback;
    std::function<void(bool)> netSpeedDeterminer;
    std::function<void(bool)> connectivityDeterminer;

    Controller::IController &controller;

    std::weak_ptr<wui::window> transientWindow;    

    std::shared_ptr<wui::window> window;
    std::shared_ptr<wui::list> sectionList;
    std::shared_ptr<wui::button> okButton, cancelButton;

    std::shared_ptr<wui::image> cameraImg, microphoneImg, loudspeakerImg, accountImg, connectionImg, preferencesImg, recordImg;
    std::shared_ptr<wui::image> noSignalImg;

    std::shared_ptr<wui::message> messageBox;

    Common::TimeMeter timeMeter;
    
    std::shared_ptr<wui::select> cameraSelect;
    std::shared_ptr<wui::text> cameraResolutionText;
    std::shared_ptr<wui::select> cameraResolutionSelect;
    std::shared_ptr<wui::text> cameraErrorText;
    std::vector<Device> cameraDevices;
    CaptureSession::CaptureVideoSession captureVideoSession;
    RendererSession::RendererVideoSession rendererVideoSession;
    mt::timer redrawVideoTimer;
    int64_t currentCameraId, currentCameraResolutionId;

    std::shared_ptr<wui::select> microphoneSelect;
    std::shared_ptr<wui::text> microphoneSensitivityText;
    std::shared_ptr<wui::slider> microphoneSensitivitySlider;
    std::shared_ptr<wui::button> microphoneAECCheck, microphoneNSCheck, microphoneAGCCheck;
    std::shared_ptr<wui::button> microphone16SampleRateCheck, microphone48SampleRateCheck;
    std::vector<Device> microphoneDevices;
    std::shared_ptr<SoundIndicator> soundIndicator;
    MicrophoneNS::Microphone microphone;
    int64_t currentMicrophoneId;

    std::shared_ptr<wui::select> loudspeakerSelect;
    std::shared_ptr<wui::text> loudspeakerVolumeText;
    std::shared_ptr<wui::slider> loudspeakerSlider;
    std::shared_ptr<wui::button> loudspeakerCheckButton;
    std::shared_ptr<wui::button> loudspeaker16SampleRateCheck, loudspeaker48SampleRateCheck;
    AudioRenderer::IAudioRenderer &audioRenderer;
    Audio::Resampler &resampler;
    Audio::AudioMixer &audioMixer;
    Ringer &ringer;
    int64_t currentLoudspeakerId;

    std::shared_ptr<wui::text> userNameText;
    std::shared_ptr<wui::input> userNameInput;
    std::shared_ptr<wui::button> userChangeChandentialsLink;
    std::shared_ptr<wui::text> userLoginText;
    std::shared_ptr<wui::input> userLoginInput;
    std::shared_ptr<wui::text> userPasswordText;
    std::shared_ptr<wui::input> userPasswordInput;
    std::shared_ptr<wui::text> userPasswordConfirmText;
    std::shared_ptr<wui::input> userPasswordConfirmInput;
    std::string userName, userLogin, userPassword, userPasswordConfirm;

    std::shared_ptr<wui::text> connectionServerText;
    std::shared_ptr<wui::input> connectionServerInput;
    std::shared_ptr<wui::text> connectionLoginText;
    std::shared_ptr<wui::input> connectionLoginInput;
    std::shared_ptr<wui::text> connectionPasswordText;
    std::shared_ptr<wui::input> connectionPasswordInput;
    std::shared_ptr<wui::button> connectionAutoDetermineSpeedCheck, connectionSetSpeedManualCheck;
    std::shared_ptr<wui::text> connectionInSpeedText;
    std::shared_ptr<wui::input> connectionInSpeedInput;
    std::shared_ptr<wui::text> connectionOutSpeedText;
    std::shared_ptr<wui::input> connectionOutSpeedInput;
    std::shared_ptr<wui::button> connectionDetermineSpeedNowButton, connectionCheckConnectivityNowButton;
    std::string connectionServer, connectionLogin, connectionPassword, connectionInSpeed, connectionOutSpeed;

    std::shared_ptr<wui::button> prefTimerCheck;
    std::shared_ptr<wui::button> prefAutoAnswerCheck;
    std::shared_ptr<wui::button> prefAutoConnectToConfCheck;
    std::shared_ptr<wui::button> prefShowAvatarCheck;
    std::shared_ptr<wui::button> prefRequestTurnCamMicCheck;
    std::shared_ptr<wui::text> prefLanguageText;
    std::shared_ptr<wui::select> prefLanguageSelect;
    std::shared_ptr<wui::button> prefAutoStartAppCheck;
    std::shared_ptr<wui::button> prefHideToTrayCheck;
    std::shared_ptr<wui::button> prefAccentuateSpeakerCheck;
    std::shared_ptr<wui::button> prefUseDemonstrationWindowCheck;

    std::shared_ptr<wui::text> recordPathText;
    std::shared_ptr<wui::input> recordPathInput;
    std::shared_ptr<wui::button> recordPathButton;
    std::shared_ptr<wui::button> recordMP3Check;
    std::string recordPath;
    
    void OK();

    void DrawListItem(wui::graphic &gr, int32_t nItem, const wui::rect &pos, wui::list::item_state state);
    void ChangeListItem(int32_t nItem);

    void ShowCamera();
    void HideCamera();
    void ChangeCamera(int32_t nItem, int64_t id);
    void ChangeResouliton(int32_t nItem, int64_t id);
    void FillResolutions(const Device &device);
    void RedrawVideo();
    void ReceiveDeviceNotify(std::string_view name, DeviceNotifyType notifyType, Proto::DeviceType deviceType, uint32_t deviceId, int32_t iData);
    bool UpdateCamera();

    void ShowMicrophone();
    void HideMicrophone();
    void ChangeMicrophone(int32_t nItem, int64_t id);
    bool UpdateMicrophone();

    void ShowLoudspeaker();
    void HideLoudspeaker();
    void ChangeLoudspeaker(int32_t nItem, int64_t id);
    void ChangeLoudspeakerSampleRate(int32_t sampleRate);
    bool UpdateLoudspeaker();

    void ShowAccount();
    void HideAccount();
    void ShowChangeChandentials();
    bool UpdateAccount();

    void ShowConnection();
    void HideConnection();
    bool UpdateConnection();
    void UpdateSpeedInputs();
    void DetermineSpeedNow();
    void CheckConnectivityNow();
    void ShowIncorrectURLError();

    void ShowPreferences();
    void HidePreferences();
    bool UpdatePreferences();

    void ShowRecord();
    void HideRecord();
    void SelectPath();
    bool UpdateRecord();
};

}
