/**
 * MainToolBar.cpp - Contains main toolbar impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <wui/locale/locale.hpp>
#include <wui/theme/theme.hpp>

#include <wui/control/image.hpp>

#include <wui/config/config.hpp>

#include <wui/common/flag_helpers.hpp>

#include <UI/MainToolBar.h>

#include <resource.h>

namespace Client
{

enum class MyEvent : uint32_t
{
    UpdatePanelPosition = 5000
};

struct ButtonParams
{
    std::string_view tooltip;

#ifdef _WIN32
    int32_t image;
#else
    const std::string image;
#endif

};

enum class Button
{
    Camera,
    Microphone,
    Speaker,
    RendererMode,
    List,
    Content
};

ButtonParams Params(Button button)
{
    switch (button)
    {
        case Button::Camera:
        {
            auto enabled = wui::config::get_int("CaptureDevices", "CameraEnabled", 1) != 0;
            return { wui::locale("toolbar", enabled ? "camera_off" : "camera_on"), enabled ? IMG_TB_CAMERA : IMG_TB_CAMERA_DISABLED };
        }
        break;
        case Button::Microphone:
        {
            auto enabled = wui::config::get_int("CaptureDevices", "MicrophoneEnabled", 1) != 0;
            return { wui::locale("toolbar", enabled ? "microphone_off" : "microphone_on"), enabled ? IMG_TB_MICROPHONE : IMG_TB_MICROPHONE_DISABLED };
        }
        break;
        case Button::Speaker:
        {
            auto enabled = wui::config::get_int("AudioRenderer", "Enabled", 1) != 0;
            return { wui::locale("toolbar", enabled ? "speaker_off" : "speaker_on"), enabled ? IMG_TB_LOUDSPEAKER : IMG_TB_LOUDSPEAKER_DISABLED };
        }
        break;
        case Button::RendererMode:
        {
            auto big = wui::config::get_int("VideoRenderer", "GridType", 1) != 0;
            return { wui::locale("toolbar", big ? "renderer_mode_big" : "renderer_mode_grid"), big ? IMG_TB_BIGGRID : IMG_TB_EVENGRID };
        }
        break;
        case Button::List:
        {
            auto enabled = wui::config::get_int("ListPanel", "Enabled", 1) != 0;
            return { wui::locale("toolbar", enabled ? "list_off" : "list_on"), enabled ? IMG_TB_LISTPANEL : IMG_TB_LISTPANEL_DISABLED };
        }
        break;
        case Button::Content:
        {
            auto enabled = wui::config::get_int("ContentPanel", "Enabled", 1) != 0;
            return { wui::locale("toolbar", enabled ? "content_off" : "content_on"), enabled ? IMG_TB_CONTENTPANEL : IMG_TB_CONTENTPANEL_DISABLED };
        }
        break;
    }

    return { "",
#ifdef _WIN32
        0
#else
        ""
#endif
    };
}

MainToolBar::MainToolBar(std::weak_ptr<wui::window> mainFrame_, IControlActions &receiver)
    : parent(mainFrame_),
    panel(new wui::panel("toolbar_panel")),
    call          (new wui::button(wui::locale("toolbar", "call"),                [&receiver]() { receiver.ActionCall();                  }, wui::button_view::image, IMG_TB_CALL,                        BTN_SIZE, wui::button::tc_tool)),
    conference    (new wui::button(wui::locale("toolbar", "conference"),          [&receiver]() { receiver.ActionConference();            }, wui::button_view::image, IMG_TB_CONFERENCE,                  BTN_SIZE, wui::button::tc_tool)),
    hangup        (new wui::button(wui::locale("toolbar", "hangup"),              [&receiver]() { receiver.ActionHangup();                }, wui::button_view::image, IMG_TB_HANGUP,                      BTN_SIZE, wui::button::tc_tool)),
    separator0    (new wui::image(IMG_TB_SEPARATOR)),
    camera        (new wui::button(Params(Button::Camera).tooltip,                [&receiver]() { receiver.ActionTurnCamera(true);        }, wui::button_view::image, Params(Button::Camera).image,       BTN_SIZE, wui::button::tc_tool)),
    microphone    (new wui::button(Params(Button::Microphone).tooltip,            [&receiver]() { receiver.ActionTurnMicrophone(true);    }, wui::button_view::image, Params(Button::Microphone).image,   BTN_SIZE, wui::button::tc_tool)),
    microphoneVol (new wui::button(wui::locale("toolbar", "gain"),                [&receiver]() { receiver.ActionVolumeMicrophone();      }, wui::button_view::image, IMG_TB_EXPAND,                      EXP_SIZE, wui::button::tc_tool)),
    speaker       (new wui::button(Params(Button::Speaker).tooltip,               [&receiver]() { receiver.ActionTurnLoudspeaker();       }, wui::button_view::image, Params(Button::Speaker).image,      BTN_SIZE, wui::button::tc_tool)),
    speakerVol    (new wui::button(wui::locale("toolbar", "volume"),              [&receiver]() { receiver.ActionVolumeLoudspeaker();     }, wui::button_view::image, IMG_TB_EXPAND,                      EXP_SIZE, wui::button::tc_tool)),
    separator1    (new wui::image(IMG_TB_SEPARATOR)),
    screenCapturer(new wui::button(wui::locale("toolbar", "screen_capturer_off"), [&receiver]() { receiver.ActionTurnDemonstration(true); }, wui::button_view::image, IMG_TB_SCREENCAPTURE_DISABLED,      BTN_SIZE, wui::button::tc_tool)),
    separator2    (new wui::image(IMG_TB_SEPARATOR)),
    rendererMode  (new wui::button(Params(Button::RendererMode).tooltip,          [&receiver]() { receiver.ActionTurnRendererGrid();      }, wui::button_view::image, Params(Button::RendererMode).image, BTN_SIZE, wui::button::tc_tool)),
    separator3    (new wui::image(IMG_TB_SEPARATOR)),
    hand          (new wui::button(wui::locale("toolbar", "hand"),                [&receiver]() { receiver.ActionHand(true);              }, wui::button_view::image, IMG_TB_HAND,                        BTN_SIZE, wui::button::tc_tool)),
    separator4    (new wui::image(IMG_TB_SEPARATOR)),
    list          (new wui::button(Params(Button::List).tooltip,                  [&receiver]() { receiver.ActionTurnListPanel();         }, wui::button_view::image, Params(Button::List).image,         BTN_SIZE, wui::button::tc_tool)),
    content       (new wui::button(Params(Button::Content).tooltip,               [&receiver]() { receiver.ActionTurnContentPanel();      }, wui::button_view::image, Params(Button::Content).image,      BTN_SIZE, wui::button::tc_tool)),
    separator5    (new wui::image(IMG_TB_SEPARATOR)),
    fullscreen    (new wui::button(wui::locale("toolbar", "fullscreen"),          [&receiver]() { receiver.ActionFullScreen();            }, wui::button_view::image, IMG_TB_FULLSCREEN,                  BTN_SIZE, wui::button::tc_tool)),
    menu          (new wui::button(wui::locale("toolbar", "menu"),                [&receiver]() { receiver.ActionMenu();                  }, wui::button_view::image, IMG_TB_MENU,                        BTN_SIZE, wui::button::tc_tool)),
    timer         ([this]() { auto parent_ = parent.lock(); if (parent_) parent_->emit_event(static_cast<int32_t>(MyEvent::UpdatePanelPosition), 0); }),
    showPos(0), delayCount(0),
    normal(true)
{
}

MainToolBar::~MainToolBar()
{
    timer.stop();
}

void MainToolBar::Run()
{
    auto parent_ = parent.lock();
    if (!parent_)
    {
        return;
    }

    auto width = parent_->position().width();

    parent_->add_control(panel, { 0, TOP, width, TOP + SHIFT + BTN_SIZE } );

    auto left = SHIFT, top = TOP + SHIFT, bottom = TOP + SHIFT + BTN_SIZE;

    parent_->add_control(call,           { left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    parent_->add_control(conference,     { left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    parent_->add_control(hangup,         { left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    parent_->add_control(separator0,     { left, top, left + SEP_SIZE, bottom }); left += SEP_SIZE;

    parent_->add_control(camera,         { left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    parent_->add_control(microphone,     { left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    parent_->add_control(microphoneVol,  { left, top, left + EXP_SIZE, bottom }); left += EXP_SIZE;
    parent_->add_control(speaker,        { left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    parent_->add_control(speakerVol,     { left, top, left + EXP_SIZE, bottom }); left += EXP_SIZE;
    parent_->add_control(separator1,     { left, top, left + SEP_SIZE, bottom }); left += SEP_SIZE;

    parent_->add_control(screenCapturer, { left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    parent_->add_control(separator2,     { left, top, left + SEP_SIZE, bottom }); left += SEP_SIZE;

    parent_->add_control(rendererMode,   { left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    parent_->add_control(separator3,     { left, top, left + SEP_SIZE, bottom }); left += SEP_SIZE;

    parent_->add_control(hand,           { left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    parent_->add_control(separator4,     { left, top, left + SEP_SIZE, bottom }); left += SEP_SIZE;

    parent_->add_control(list,           { left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    parent_->add_control(content,        { left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    parent_->add_control(separator5,     { left, top, left + SEP_SIZE, bottom }); left += SEP_SIZE;

    parent_->add_control(fullscreen,     { left, top, left + BTN_SIZE, bottom });

    parent_->add_control(menu,           { width - (SHIFT + BTN_SIZE), top, width - SHIFT, bottom });

    parent_->subscribe(std::bind(&MainToolBar::ReceiveEvents, this, std::placeholders::_1),
        wui::flags_map<wui::event_type>(2, wui::event_type::internal, wui::event_type::mouse));
}

void MainToolBar::End()
{
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->remove_control(panel);
        parent_->remove_control(call);
        parent_->remove_control(conference);
        parent_->remove_control(hangup);
        parent_->remove_control(separator0);
        parent_->remove_control(camera);
        parent_->remove_control(microphone);
        parent_->remove_control(microphoneVol);
        parent_->remove_control(speaker);
        parent_->remove_control(speakerVol);
        parent_->remove_control(separator1);
        parent_->remove_control(screenCapturer);
        parent_->remove_control(separator2);
        parent_->remove_control(rendererMode);
        parent_->remove_control(separator3);
        parent_->remove_control(hand);
        parent_->remove_control(separator4);
        parent_->remove_control(list);
        parent_->remove_control(content);
        parent_->remove_control(separator5);
        parent_->remove_control(fullscreen);
        parent_->remove_control(menu);
    }
}

void MainToolBar::UpdateWidth(int32_t width)
{
    panel->set_position({ 0, TOP, width, TOP + SHIFT + BTN_SIZE });
    menu->set_position({ width - (SHIFT + BTN_SIZE), TOP + SHIFT, width - SHIFT, TOP + SHIFT + BTN_SIZE }, false);
}

void MainToolBar::EnableCamera(bool enabled)
{
    camera->set_image(enabled ? IMG_TB_CAMERA : IMG_TB_CAMERA_DISABLED);
    camera->set_caption(enabled ? wui::locale("toolbar", "camera_off") : wui::locale("toolbar", "camera_on"));
}

void MainToolBar::EnableMicrophone(bool enabled)
{
    microphone->set_image(enabled ? IMG_TB_MICROPHONE : IMG_TB_MICROPHONE_DISABLED);
    microphone->set_caption(enabled ? wui::locale("toolbar", "microphone_off") : wui::locale("toolbar", "microphone_on"));
}

void MainToolBar::EnableLoudspeaker(bool enabled)
{
    speaker->set_image(enabled ? IMG_TB_LOUDSPEAKER : IMG_TB_LOUDSPEAKER_DISABLED);
    speaker->set_caption(enabled ? wui::locale("toolbar", "speaker_off") : wui::locale("toolbar", "speaker_on"));
}

void MainToolBar::EnableScreenCapturer(bool enabled)
{
    screenCapturer->set_image(enabled ? IMG_TB_SCREENCAPTURE : IMG_TB_SCREENCAPTURE_DISABLED);
    screenCapturer->set_caption(enabled ? wui::locale("toolbar", "screen_capturer_off") : wui::locale("toolbar", "screen_capturer_on"));
}

void MainToolBar::ChangeRendererMode(bool big)
{
    rendererMode->set_image(big ? IMG_TB_BIGGRID : IMG_TB_EVENGRID);
    rendererMode->set_caption(big ? wui::locale("toolbar", "renderer_mode_big") : wui::locale("toolbar", "renderer_mode_grid"));
}

void MainToolBar::EnableList(bool enabled)
{
    list->set_image(enabled ? IMG_TB_LISTPANEL : IMG_TB_LISTPANEL_DISABLED);
    list->set_caption(enabled ? wui::locale("toolbar", "list_off") : wui::locale("toolbar", "list_on"));
}

void MainToolBar::EnableContent(bool enabled)
{
    content->set_image(enabled ? IMG_TB_CONTENTPANEL : IMG_TB_CONTENTPANEL_DISABLED);
    content->set_caption(enabled ? wui::locale("toolbar", "content_off") : wui::locale("toolbar", "content_on"));
}

void MainToolBar::Normalize()
{
    if (normal)
    {
        return;
    }

    normal = true;

    timer.stop();

    auto parent_ = parent.lock();
    if (!parent_)
    {
        return;
    }

    auto width = parent_->position().width();

    panel->set_position({ 0, TOP, width, TOP + SHIFT + BTN_SIZE });

    auto left = SHIFT, top = TOP + SHIFT, bottom = TOP + SHIFT + BTN_SIZE;

    call->set_position          ({ left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    conference->set_position    ({ left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    hangup->set_position        ({ left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    separator0->set_position    ({ left, top, left + SEP_SIZE, bottom }); left += SEP_SIZE;

    camera->set_position        ({ left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    microphone->set_position    ({ left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    microphoneVol->set_position ({ left, top, left + EXP_SIZE, bottom }); left += EXP_SIZE;
    speaker->set_position       ({ left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    speakerVol->set_position    ({ left, top, left + EXP_SIZE, bottom }); left += EXP_SIZE;
    separator1->set_position    ({ left, top, left + SEP_SIZE, bottom }); left += SEP_SIZE;

    screenCapturer->set_position({ left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    separator2->set_position    ({ left, top, left + SEP_SIZE, bottom }); left += SEP_SIZE;

    rendererMode->set_position  ({ left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    separator3->set_position    ({ left, top, left + SEP_SIZE, bottom }); left += SEP_SIZE;

    hand->set_position          ({ left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    separator4->set_position    ({ left, top, left + SEP_SIZE, bottom }); left += SEP_SIZE;

    list->set_position          ({ left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    content->set_position       ({ left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    separator5->set_position    ({ left, top, left + SEP_SIZE, bottom }); left += SEP_SIZE;

    fullscreen->set_position    ({ left, top, left + BTN_SIZE, bottom });

    menu->set_position          ({ width - (SHIFT + BTN_SIZE), top, width - SHIFT, bottom });
}

void MainToolBar::FullScreen()
{
    if (!normal)
    {
        return;
    }

    normal = false;

    showPos = 0;
    delayCount = 0;

    UpdatePanelPosition(true);

    panel->set_topmost(true);
    call->set_topmost(true);
    conference->set_topmost(true);
    hangup->set_topmost(true);
    separator0->set_topmost(true);
    camera->set_topmost(true);
    microphone->set_topmost(true);
    microphoneVol->set_topmost(true);
    speaker->set_topmost(true);
    speakerVol->set_topmost(true);
    separator1->set_topmost(true);
    screenCapturer->set_topmost(true);
    separator2->set_topmost(true);
    rendererMode->set_topmost(true);
    separator3->set_topmost(true);
    hand->set_topmost(true);
    separator4->set_topmost(true);
    list->set_topmost(true);
    content->set_topmost(true);
    separator5->set_topmost(true);
    fullscreen->set_topmost(true);
    menu->set_topmost(true);

    timer.start(100);
}

int32_t MainToolBar::Bottom() const
{
    return normal ? TOP + panel->position().height() : 0;
}

std::shared_ptr<wui::i_control> MainToolBar::GetMicrophoneGainControl() const
{
    return microphoneVol;
}

std::shared_ptr<wui::i_control> MainToolBar::GetSpeakerVolumeControl() const
{
    return speakerVol;
}

std::shared_ptr<wui::i_control> MainToolBar::GetMenuControl() const
{
    return menu;
}

void MainToolBar::UpdatePanelPosition(bool show)
{
    auto width = 13 * (SHIFT + BTN_SIZE) + 7 * 6;
    auto height = 2 * SHIFT + BTN_SIZE;

    if (!show)
    {
        if (showPos > height)
        {
            return;
        }

        ++delayCount;

        if (delayCount < 10)
        {
            return;
        }

        showPos += 4;
    }
    else
    {      
        delayCount = 0;
        showPos = 0;
    }

    auto parent_ = parent.lock();
    if (!parent_)
    {
        return;
    }

    auto parentPos = parent_->position();

    auto left = (parentPos.width() - width) / 2;
    auto top = parentPos.height() - height;

    top += showPos;

    panel->set_position({ left, top, left + width, top + height });

    left += SHIFT;
    top += SHIFT;
    auto bottom = top + SHIFT + BTN_SIZE;

    call->set_position({ left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    conference->set_position({ left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    hangup->set_position({ left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    separator0->set_position({ left, top, left + SEP_SIZE, bottom }); left += SEP_SIZE;

    camera->set_position({ left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    microphone->set_position({ left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    microphoneVol->set_position({ left, top, left + EXP_SIZE, bottom }); left += EXP_SIZE;
    speaker->set_position({ left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    speakerVol->set_position({ left, top, left + EXP_SIZE, bottom }); left += EXP_SIZE;
    separator1->set_position({ left, top, left + SEP_SIZE, bottom }); left += SEP_SIZE;

    screenCapturer->set_position({ left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    separator2->set_position({ left, top, left + SEP_SIZE, bottom }); left += SEP_SIZE;

    rendererMode->set_position({ left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    separator3->set_position({ left, top, left + SEP_SIZE, bottom }); left += SEP_SIZE;

    hand->set_position({ left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    separator4->set_position({ left, top, left + SEP_SIZE, bottom }); left += SEP_SIZE;

    list->set_position({ left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    content->set_position({ left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;
    separator5->set_position({ left, top, left + SEP_SIZE, bottom }); left += SEP_SIZE;

    fullscreen->set_position({ left, top, left + BTN_SIZE, bottom }); left += BTN_SIZE;

    menu->set_position({ left, top, left + BTN_SIZE, bottom });
}

void MainToolBar::ReceiveEvents(const wui::event &ev)
{
    switch (ev.type)
    {
        case wui::event_type::mouse:
            if (!normal)
            {
                if (ev.mouse_event_.type == wui::mouse_event_type::move)
                {
                    static int32_t x = 0, y = 0;

                    if (ev.mouse_event_.x == x && ev.mouse_event_.y == y)
                    {
                        return;
                    }
                    x = ev.mouse_event_.x;
                    y = ev.mouse_event_.y;
                }
                UpdatePanelPosition(true);
            }
        break;
        case wui::event_type::internal:
            if (!normal && ev.internal_event_.x == static_cast<int32_t>(MyEvent::UpdatePanelPosition))
            {
                UpdatePanelPosition(false);
            }
        break;
    }
}

}
