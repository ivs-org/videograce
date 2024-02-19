/**
 * MainToolBar.h - Contains main toolbar header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#pragma once

#include <wui/window/window.hpp>
#include <wui/control/panel.hpp>
#include <wui/control/button.hpp>
#include <wui/control/image.hpp>

#include <mt/timer.h>

#include <UI/IControlActions.h>

namespace Client
{

class MainToolBar
{
public:
    MainToolBar(std::weak_ptr<wui::window> mainFrame, IControlActions &receiver);
    ~MainToolBar();

    void Run();
    void End();

    void UpdateWidth(int32_t width);

    void EnableCamera(bool enabled);
    void EnableMicrophone(bool enabled);
    void EnableLoudspeaker(bool enabled);
    void EnableScreenCapturer(bool enabled);
    void ChangeRendererMode(bool big);
    void EnableList(bool enabled);
    void EnableContent(bool enabled);

    void Normalize();
    void FullScreen();

    int32_t Bottom() const;

    std::shared_ptr<wui::i_control> GetMicrophoneGainControl() const;
    std::shared_ptr<wui::i_control> GetSpeakerVolumeControl() const;
    std::shared_ptr<wui::i_control> GetMenuControl() const;

private:
    static constexpr int32_t TOP = 30, SHIFT = 2, BTN_SIZE = 48, EXP_SIZE = 16, SEP_SIZE = 5;
    
    std::weak_ptr<wui::window> parent;

    std::shared_ptr<wui::panel> panel;
    std::shared_ptr<wui::button> call, conference, hangup;
    std::shared_ptr<wui::image> separator0;
    std::shared_ptr<wui::button> camera;
    std::shared_ptr<wui::button> microphone, microphoneVol;
    std::shared_ptr<wui::button> speaker, speakerVol;
    std::shared_ptr<wui::image> separator1;
    std::shared_ptr<wui::button> screenCapturer;
    std::shared_ptr<wui::image> separator2;
    std::shared_ptr<wui::button> rendererMode;
    std::shared_ptr<wui::image> separator3;
    std::shared_ptr<wui::button> hand;
    std::shared_ptr<wui::image> separator4;
    std::shared_ptr<wui::button> list, content;
    std::shared_ptr<wui::image> separator5;
    std::shared_ptr<wui::button> fullscreen;
    std::shared_ptr<wui::button> menu;

    mt::timer timer;
    int32_t delayCount;

    bool normal;

    void SetFullScreenPosition();
    void Show();
    void Hide();

    void ReceiveEvents(const wui::event &ev);
};

}
