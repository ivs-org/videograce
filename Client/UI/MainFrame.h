/**
 * MainFrame.h - Contains main frame header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#pragma once

#include <queue>

#include <wui/window/window.hpp>
#include <wui/control/menu.hpp>
#include <wui/control/button.hpp>
#include <wui/control/progress.hpp>
#include <wui/control/input.hpp>
#include <wui/control/message.hpp>
#include <wui/control/tray_icon.h>

#include <Record/Recorder.h>
#include <AudioRenderer/AudioRenderer.h>
#include <Audio/Resampler.h>
#include <Audio/AudioMixer.h>
#include <Controller/Controller.h>
#include <Storage/Storage.h>

#include <NetTester/SpeedTester.h>
#include <NetTester/UDPTester.h>

#include <UI/IControlActions.h>
#include <UI/ContactList.h>
#include <UI/MemberList.h>

#include <UI/CallParams.h>

#include <UI/ActivateHandler.h>

#include <UI/BusyBox.h>
#include <UI/Ringer.h>
#include <UI/CPUMeter.h>

#include <UI/MainToolBar.h>
#include <UI/ListPanel.h>
#include <UI/ContentPanel.h>
#include <UI/RenderersBox.h>
#include <UI/TimerBar.h>

#include <UI/VolumeBox.h>

#include <UI/AboutDialog.h>

#include <UI/CredentialsDialog.h>

#include <UI/DialingDialog.h>
#include <UI/QuestionDialog.h>

#include <UI/ConferenceDialog.h>

#include <UI/SettingsDialog.h>

#include <UI/DeviceNotifies.h>

#include <UI/MiniWindow.h>

#include <Device/Device.h>

#include <CaptureSession/CaptureAudioSession.h>
#include <CaptureSession/CaptureVideoSession.h>
#include <RendererSession/RendererAudioSession.h>
#include <RendererSession/RendererVideoSession.h>

#ifdef _WIN32
#include <Device/DS/DummyGraph.h>
#endif

namespace Client
{

class MainFrame : public IControlActions,
    public IContactListCallback,
    public ListPanelCallback,
    public ContentPanelCallback
{
public:
    MainFrame();
    ~MainFrame();

    void Run(bool minimized);

    wui::system_context &context();

    /// IControlActions impl
    virtual void ActionCall();
    virtual void ActionConference();
    virtual void ActionHangup();

    virtual void ActionDevices();
    virtual void ActionTurnCamera(bool my = true, int64_t actorId = -1, std::string_view actorName = "");

    virtual void ActionTurnMicrophone(bool my = true, int64_t actorId = -1, std::string_view actorName = "");
    virtual void ActionVolumeMicrophone();

    virtual void ActionTurnLoudspeaker();
    virtual void ActionVolumeLoudspeaker();

    virtual void ActionTurnRendererGrid();
    
    virtual void ActionTurnDemonstration(bool my = true, int64_t actorId = -1, std::string_view actorName = "");
    virtual void ActionTurnRemoteControl(bool my = true, int64_t actorId = -1, std::string_view actorName = "", bool enable = true);

    virtual void ActionHand(bool my);

    virtual void ActionTurnListPanel();
    virtual void ActionTurnContentPanel();

    virtual void ActionTurnRecord();

    virtual void ActionFullScreen();

    virtual void ActionMenu();

    /// IContactListCallback impl
    virtual void ContactSelected(int64_t id, std::string_view name);
    virtual void ContactCall(std::string_view name);
    
    virtual void ConferenceSelected(std::string_view tag, std::string_view name);
    virtual void ConferenceConnect(std::string_view tag);
    
    virtual void ContactUnselected();

    /// ListPanelCallback impl
    virtual void ListPanelMembersSelected();
    virtual void ListPanelClosed();
    virtual void ListPanelPinChanged();
    virtual void ListPanelWidthChanged(int32_t width);

    /// ContentPanelCallback impl
    virtual void ContentPanelClosed();
    virtual void ContentPanelPinChanged();
    virtual void ContentPanelWidthChanged(int32_t width);

private:
    std::shared_ptr<wui::window> window;
    std::shared_ptr<wui::message> messageBox;
    std::shared_ptr<wui::menu> mainMenu;
    std::shared_ptr<wui::progress> mainProgress;

    std::shared_ptr<VolumeBox> volumeBox;

    std::shared_ptr<wui::tray_icon> trayIcon;

    std::mutex controllerEventsQueueMutex;
    std::queue<Controller::Event> controllerEventsQueue;

    ActivateHandler activateHandler;

    Storage::Storage storage;
    
    ContactList contactList;
    MemberList memberList;

    Controller::Controller controller;

    Common::TimeMeter timeMeter;

    NetTester::SpeedTester speedTester;
    NetTester::UDPTester udpTester;

    std::vector<Device> cameraDevices;
    std::vector<Device> microphoneDevices;
    std::vector<std::string> audioRendererDevices;

    std::shared_ptr<CaptureSession::CaptureVideoSession> captureVideoSession, demonstrationSession;
    std::shared_ptr<CaptureSession::CaptureAudioSession> captureAudioSession;

    Recorder::Recorder recorder;

    AudioRenderer::AudioRenderer audioRenderer;
    Audio::Resampler resampler;
    Audio::AudioMixer audioMixer;

    std::vector<std::shared_ptr<RendererSession::RendererVideoSession>> renderersVideo;
    std::map<uint32_t, std::shared_ptr<RendererSession::RendererAudioSession>> renderersAudio;

    CPUMeter cpuMeter;

    Ringer ringer;

    MiniWindow miniWindow;

    MainToolBar mainToolBar;
    ListPanel listPanel;
    ContentPanel contentPanel;
    RenderersBox renderersBox;
    TimerBar timerBar;

    BusyBox busyBox;
    std::string busyTitle, busySubTitle;

    AboutDialog aboutDialog;

    CredentialsDialog credentialsDialog;

    DialingDialog dialingDialog;
    QuestionDialog questionDialog;
    ConferenceDialog conferenceDialog;
    SettingsDialog settingsDialog;

#ifdef _WIN32
    DummyGraph dummyGraph;
#endif

    bool showConnectivityResult, useWSMedia;

    bool actionQuested;

    bool online, working;

    bool userForceClose;

    CallParams callParams;

    std::shared_ptr<spdlog::logger> sysLog, errLog;

    void ReceiveEvents(const wui::event &ev);

    bool GetEventFromQueue(Controller::Event &ev);
    void ProcessControllerEvent();

private:
    void Init();

    void UpdateTitle(std::string_view text = "", int32_t progress = 0);

    void WindowControlCallback(wui::window_control control, std::string &tooltip_text, bool &continue_);

    void InitAudio();

    void ShowBusy(std::string_view title);
    void SetBusyProgess(std::string_view sub_title, int32_t value);
    void HideBusy();

    void SetMainProgess(std::string_view sub_title, int32_t value);

    void CheckConnectivity(bool showResult = false);
    void DetermineNetSpeed(bool force = false);

    void SetStanbyMode();
    void SetConferencingMode();

    void SpeedTestCompleted(uint32_t inputSpeed, uint32_t outputSpeed);
    void UDPTestCompleted();

    void ReceiveDeviceNotify(std::string_view name, DeviceNotifyType notifyType, Proto::DeviceType deviceType, uint32_t deviceId, int32_t iData);

    void AudioRendererErrorCallback(uint32_t code, std::string_view msg);

    void VolumeBoxChangeCallback(int32_t value, VolumeBoxMode mode, bool enabled);

    void ActivateCallback();

    void SettingsReadyCallback();

    void TrayIconCallback(wui::tray_icon_action action);

    void CredentialsCloseCallback();

    void MessagesUpdatedCallback(Storage::MessageAction, const Storage::Messages &);

    void AutoRegisterUser(std::string_view name);

private:
    std::shared_ptr<CaptureSession::CaptureVideoSession> GetCaptureVideoSession(Proto::DeviceType deviceType);
    std::shared_ptr<RendererSession::RendererVideoSession> GetRendererVideoSession(uint32_t deviceId);
    std::shared_ptr<RendererSession::RendererAudioSession> GetRendererAudioSession(uint32_t deviceId);

    void UpdateOutputBitrates();

    bool CheckOutputBitrateTooSmall();
    bool CheckInputBitrateTooSmall();

    void KillScreenSaver();

    void Call(std::string_view subscriber);
    void CancelCall();
    void RestoreAfterCall();
    void QuestionCall(bool yes);

    void ConnectToURIConference();
    void ConnectToConference(std::string_view tag, bool connectMembers);
    void DisconnectFromConference();

    void ScheduleConnect(std::string_view tag, std::string_view name, bool forceQuestion);

    void StartCamera();
    void StopCamera();
    
    void StartDemonstration();
    void StopDemonstration();

    void StartMicrophone();
    void StopMicrophone();

    bool IsCameraEnabled();
    bool IsCameraExists();
    bool IsMicrophoneEnabled();
    bool IsMicrophoneExists();
    bool IsDemonstrationEnabled();

    bool IsRecordAllowed();

    void FindCamera();
    void FindMicrophone();

    void ShowAvatar(int64_t clientId, std::string_view name);
    void HideAvatar(int64_t clientId);

    void DeleteVideoRenderer(uint32_t deviceId, int64_t clientId, Proto::DeviceType deviceType);
    void DeleteAudioRenderer(uint32_t deviceId, int64_t clientId);

    void UpdateVideoRenderers();
    void UpdateVideoRendererPosition();

    void UpdateConnectedCameras();
    void UpdateConnectedMicrophones();
    void UpdateConnectedAudioRenderers();

    void UpdateContentPanelLeft();

    void ConfirmClose(bool minimizeAfterNo);

    void ShowHelp();
};

}
