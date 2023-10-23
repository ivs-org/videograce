/**
 * MemberList.cpp - Contains member list impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2015
 */

#include <wui/window/window.hpp>
#include <wui/theme/theme.hpp>
#include <wui/locale/locale.hpp>
#include <wui/system/tools.hpp>

#include <UI/MemberList.h>

#include <wui/config/config.hpp>
#include <Common/BitHelpers.h>
#include <Proto/MemberGrants.h>

#include <License/Grants.h>

#include <resource.h>

namespace Client
{

MemberList::MemberList(Storage::Storage &storage_, Controller::IController &controller_)
    : storage(storage_), controller(controller_),
    itemsMutex(), items(),
    parentWindow_(),
    list(new wui::list()),
    ownerImg(new wui::image(IMG_ML_OWNER)), moderatorImg(new wui::image(IMG_ML_MODERATOR)), ordinaryImg(new wui::image(IMG_ML_ORDINARY)), readOnlyImg(new wui::image(IMG_ML_READONLY)), deafImg(new wui::image(IMG_ML_DEAF)),
    cameraMicrophoneEnabledImg(new wui::image(IMG_ML_CAMERA_MICROPHONE_ENABLED)), cameraEnabledImg(new wui::image(IMG_ML_CAMERA_ENABLED)), microphoneEnabledImg(new wui::image(IMG_ML_MICROPHONE_ENABLED)), speakEnabledImg(new wui::image(IMG_ML_SPEAK_ENABLED)),
    addButton(new wui::button(wui::locale("member_list", "add_members"), std::bind(&MemberList::Add, this), wui::button_view::image, IMG_TB_ML_ADD, 24, wui::button::tc_tool)),
    kickButton(new wui::button(wui::locale("member_list", "kick"), std::bind(&MemberList::Kick, this), wui::button_view::image, IMG_TB_ML_DEL, 24, wui::button::tc_tool)),
    toTopButton(new wui::button(wui::locale("member_list", "to_top"), std::bind(&MemberList::ToTop, this), wui::button_view::image, IMG_TB_ML_UP, 24, wui::button::tc_tool)),
    muteAllButton(new wui::button(wui::locale("member_list", "mute_all"), std::bind(&MemberList::MuteAll, this), wui::button_view::image, IMG_TB_ML_MUTE_ALL, 24, wui::button::tc_tool)),
    speakButton(new wui::button(wui::locale("member_list", "turn_speak"), std::bind(&MemberList::TurnSpeak, this), wui::button_view::image, IMG_TB_ML_SPEAK, 24, wui::button::tc_tool)),
    devicesButton(new wui::button(wui::locale("member_list", "devices"), std::bind(&MemberList::Devices, this), wui::button_view::image, IMG_TB_ML_DEVICES, 24, wui::button::tc_tool)),
    cameraImage(new wui::image(IMG_TB_ML_CAMERA)), microphoneImage(new wui::image(IMG_TB_ML_MICROPHONE)), demonstrationImage(new wui::image(IMG_TB_ML_SCREENCAPTURE)),
    separator0(new wui::image(IMG_TB_ML_SEPARATOR)),
    separator1(new wui::image(IMG_TB_ML_SEPARATOR)),
    devicesMenu(new wui::menu()),
    contactDialog(storage, controller, std::bind(&MemberList::ContactDialogCallback, this, std::placeholders::_1, std::placeholders::_2))
{
    list->set_item_height_callback([](int32_t, int32_t& h) { h = XBITMAP; });
    list->set_draw_callback(std::bind(&MemberList::DrawItem, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

    devicesMenu->set_items({
        { 1, wui::menu_item_state::normal, wui::locale("member_list", "turn_camera"), "", cameraImage, {}, [this](int32_t) { TurnCamera(); } },
        { 2, wui::menu_item_state::normal, wui::locale("member_list", "turn_microphone"), "", microphoneImage, {}, [this](int32_t) { TurnMicrophone(); } },
        { 3, wui::menu_item_state::normal, wui::locale("member_list", "turn_screen_capt"), "", demonstrationImage, {}, [this](int32_t) { TurnDemostration(); } }
        });
}

MemberList::~MemberList()
{
}

void MemberList::Run(std::weak_ptr<wui::window> parentWindow__)
{
    parentWindow_ = parentWindow__;

    auto parentWindow = parentWindow_.lock();
    if (parentWindow)
    {
        auto parentPos = parentWindow->position();
        auto width = parentPos.width(), height = parentPos.height();

        parentWindow->add_control(list, { 10, 45, width - 10, height - 10 - 35 });
        
        parentWindow->add_control(addButton, { 10, height - 5 - 36, 10 + 36, height - 5 });
        parentWindow->add_control(kickButton, { 20 + 36, height - 5 - 28, 20 + 36 + 28, height - 5 });
        parentWindow->add_control(separator0, { 20 + 36 + 28 + 2, height - 5 - 28, 20 + 36 + 28 + 8, height - 5 });
        parentWindow->add_control(toTopButton, { 30 + 36 + 28, height - 5 - 28, 30 + 36 + 28 * 2, height - 5 });
        parentWindow->add_control(muteAllButton, { 40 + 36 + 28 * 2, height - 5 - 28, 40 + 36 + 28 * 3, height - 5 });
        parentWindow->add_control(speakButton, { 50 + 36 + 28 * 3, height - 5 - 28, 50 + 36 + 28 * 4, height - 5 });
        parentWindow->add_control(separator1, { 50 + 36 + 28 * 4 + 2, height - 5 - 28,  50 + 36 + 28 * 4 + 8, height - 5 });
        parentWindow->add_control(devicesButton, { 60 + 36 + 28 * 4, height - 5 - 28, 60 + 36 + 28 * 5, height - 5 });

        parentWindow->add_control(devicesMenu, { 0 });
    }

    list->update_theme();
    addButton->update_theme();
    kickButton->update_theme();
    toTopButton->update_theme();
    muteAllButton->update_theme();
    speakButton->update_theme();
    devicesButton->update_theme();
    separator0->update_theme();
    separator1->update_theme();
    devicesMenu->update_theme();
    cameraImage->update_theme();
    microphoneImage->update_theme();
    demonstrationImage->update_theme();

    std::lock_guard<std::recursive_mutex> lock(itemsMutex);
    Update();
}

void MemberList::End()
{
    auto parentWindow = parentWindow_.lock();
    if (parentWindow)
    {
        parentWindow->remove_control(list);
        parentWindow->remove_control(addButton);
        parentWindow->remove_control(kickButton);
        parentWindow->remove_control(separator0);
        parentWindow->remove_control(toTopButton);
        parentWindow->remove_control(muteAllButton);
        parentWindow->remove_control(speakButton);
        parentWindow->remove_control(separator1);
        parentWindow->remove_control(devicesButton);
        parentWindow->remove_control(devicesMenu);
    }
}

void MemberList::UpdateSize(int32_t width, int32_t height)
{
    list->set_position({ 10, 45, width - 10, height - 10 - 35 }, false);
    addButton->set_position({ 10, height - 5 - 36, 10 + 36, height - 5 }, false);
    kickButton->set_position({ 20 + 36, height - 5 - 28, 20 + 36 + 28, height - 5 }, false);
    separator0->set_position({ 20 + 36 + 28 + 2, height - 5 - 28, 20 + 36 + 28 + 8, height - 5 }, false);
    toTopButton->set_position({ 30 + 36 + 28, height - 5 - 28, 30 + 36 + 28 * 2, height - 5 }, false);
    muteAllButton->set_position({ 40 + 36 + 28 * 2, height - 5 - 28, 40 + 36 + 28 * 3, height - 5 }, false);
    speakButton->set_position({ 50 + 36 + 28 * 3, height - 5 - 28, 50 + 36 + 28 * 4, height - 5 }, false);
    separator1->set_position({ 50 + 36 + 28 * 4 + 2, height - 5 - 28,  50 + 36 + 28 * 4 + 8, height - 5 }, false);
    devicesButton->set_position({ 60 + 36 + 28 * 4, height - 5 - 28, 60 + 36 + 28 * 5, height - 5 }, false);
}

/// IMemberList

std::recursive_mutex &MemberList::GetItemsMutex()
{
    return itemsMutex;
}

void MemberList::UpdateItem(const Proto::Member& item)
{
    std::lock_guard<std::recursive_mutex> lock(itemsMutex);

    auto it = std::find(items.begin(), items.end(), item.id);
    if (it != items.end())
    {
        *it = item;
    }
    else
    {
        items.push_back(item);
    }

    if (items.size() < 2)
    {
        return;
    }

    bool swapp = true;
    while (swapp)
    {
        swapp = false;
        for (auto i = 0; i != items.size() - 1; ++i)
        {
            if (items[i].order > items[i + 1].order)
            {
                std::swap(items[i], items[i + 1]);
                swapp = true;
            }
        }
    }

    Update();
}

void MemberList::DeleteItem(int64_t memberId)
{
    std::lock_guard<std::recursive_mutex> lock(itemsMutex);

    auto it = std::find(items.begin(), items.end(), memberId);
    if (it != items.end())
    {
        items.erase(it);
    }

    Update();
}

void MemberList::ClearItems()
{
    std::lock_guard<std::recursive_mutex> lock(itemsMutex);
    items.clear();

    Update();
}

const Controller::members_t &MemberList::GetItems() const
{
    return items;
}

bool MemberList::ExistsMember(int64_t id)
{
    return std::find(items.begin(), items.end(), id) != items.end();
}

std::string MemberList::GetMemberName(int64_t id)
{
    auto it = std::find(items.begin(), items.end(), id);
    if (it != items.end())
    {
        return it->name;
    }

    return "";
}

uint32_t MemberList::GetMaximumInputBitrate()
{
    uint32_t minBitrate = 1024 * 100;

    for (const auto &member : items)
    {
        if (member.id != controller.GetMyClientId())
        {
            uint32_t val = member.max_input_bitrate;
            if (val < minBitrate)
            {
                minBitrate = val;
            }
        }
    }

    return minBitrate;
}

uint32_t MemberList::GetSpeakersCount()
{
    uint32_t speakersCount = 0;

    for (const auto &member : items)
    {
        if (member.id != controller.GetMyClientId() && BitIsSet(member.grants, static_cast<int32_t>(Proto::MemberGrants::Speaker)))
        {
            ++speakersCount;
        }
    }

    return speakersCount;
}

bool MemberList::IsMePresenter()
{
    for (const auto &member : items)
    {
        if (member.id == controller.GetMyClientId() && BitIsSet(member.grants, static_cast<int32_t>(Proto::MemberGrants::Presenter)))
        {
            return true;
        }
    }
    return false;
}

bool MemberList::IsMeModerator()
{
    for (const auto &member : items)
    {
        if (member.id == controller.GetMyClientId() && BitIsSet(member.grants, static_cast<int32_t>(Proto::MemberGrants::Moderator)))
        {
            return true;
        }
    }
    return false;
}

bool MemberList::IsMeReadOnly()
{
    for (const auto &member : items)
    {
        if (member.id == controller.GetMyClientId() && BitIsSet(member.grants, static_cast<int32_t>(Proto::MemberGrants::ReadOnly)))
        {
            return true;
        }
    }
    return false;
}

bool MemberList::IsMeSpeaker()
{
    for (const auto &member : items)
    {
        if (member.id == controller.GetMyClientId())
        {
            return BitIsSet(member.grants, static_cast<int32_t>(Proto::MemberGrants::Speaker));
        }
    }
    return false;
}

void MemberList::TurnMeSpeaker()
{
    for (auto &member : items)
    {
        if (member.id == controller.GetMyClientId())
        {
            auto nowSpeaker = BitIsSet(member.grants, static_cast<int32_t>(Proto::MemberGrants::Speaker));
            if (nowSpeaker)
            {
                ClearBit(member.grants, static_cast<int32_t>(Proto::MemberGrants::Speaker));
            }
            else
            {
                SetBit(member.grants, static_cast<int32_t>(Proto::MemberGrants::Speaker));
            }
        }
    }
}

void MemberList::Update()
{
    std::lock_guard<std::recursive_mutex> lock(itemsMutex);
    list->set_item_count(static_cast<int32_t>(items.size()));
}

/// Impl

const Proto::Member* MemberList::GetItem(int32_t itemNumber)
{
    if (itemNumber < static_cast<int32_t>(items.size()))
    {
        return &items[itemNumber];
    }

    return nullptr;
}

void MemberList::DrawItem(wui::graphic &gr, int32_t nItem, const wui::rect &itemRect, wui::list::item_state state)
{
    std::lock_guard<std::recursive_mutex> lock(itemsMutex);

    auto item = GetItem(nItem);
    if (!item)
    {
        return;
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

    auto name = item->name;
    truncate_line(name, gr, font, textRect.width(), 4);

    gr.draw_text(textRect, name, textColor, font);

    wui::rect imgRect = { itemRect.left,
        itemRect.top,
        itemRect.left + XBITMAP,
        itemRect.top + XBITMAP };

    if (BitIsSet(item->grants, static_cast<int32_t>(Proto::MemberGrants::Deaf)))
    {
        deafImg->set_position(imgRect);
        deafImg->draw(gr, { 0 });
    }
    else if (BitIsSet(item->grants, static_cast<int32_t>(Proto::MemberGrants::ReadOnly)))
    {
        readOnlyImg->set_position(imgRect);
        readOnlyImg->draw(gr, { 0 });
    }
    else if (BitIsSet(item->grants, static_cast<int32_t>(Proto::MemberGrants::Moderator)))
    {
        moderatorImg->set_position(imgRect);
        moderatorImg->draw(gr, { 0 });
    }
    else if (BitIsSet(item->grants, static_cast<int32_t>(Proto::MemberGrants::Presenter)))
    {
        ownerImg->set_position(imgRect);
        ownerImg->draw(gr, { 0 });
    }
    else
    {
        ordinaryImg->set_position(imgRect);
        ordinaryImg->draw(gr, { 0 });
    }

    if (item->has_camera && item->has_microphone)
    {
        cameraMicrophoneEnabledImg->set_position({ itemRect.right - 32 * 2, itemRect.top, itemRect.right - 32 * 2 + XBITMAP, itemRect.top + XBITMAP });
        cameraMicrophoneEnabledImg->draw(gr, { 0 });
    }
    else if (item->has_camera)
    {
        cameraEnabledImg->set_position({ itemRect.right - 32 * 2, itemRect.top, itemRect.right - 32 * 2 + XBITMAP, itemRect.top + XBITMAP });
        cameraEnabledImg->draw(gr, { 0 });
    }
    else if (item->has_microphone)
    {
        microphoneEnabledImg->set_position({ itemRect.right - 32 * 2, itemRect.top, itemRect.right - 32 * 2 + XBITMAP, itemRect.top + XBITMAP });
        microphoneEnabledImg->draw(gr, { 0 });
    }
    if (BitIsSet(item->grants, static_cast<int32_t>(Proto::MemberGrants::Speaker)))
    {
        speakEnabledImg->set_position({ itemRect.right - 32, itemRect.top, itemRect.right - 32 + XBITMAP, itemRect.top + XBITMAP });
        speakEnabledImg->draw(gr, { 0 });
    }
}

void MemberList::Add()
{
    contactDialog.Run(parentWindow_, ContactDialogMode::ConnectToConference);
}

void MemberList::Kick()
{
    std::lock_guard<std::recursive_mutex> lock(itemsMutex);
    auto *item = GetItem(list->selected_item());
    if (item)
    {
        controller.SendMemberAction({ item->id }, Proto::MEMBER_ACTION::Action::DisconnectFromConference);
    }
}

void MemberList::ToTop()
{
    std::lock_guard<std::recursive_mutex> lock(itemsMutex);
    auto *item = GetItem(list->selected_item());
    if (item)
    {
        controller.SendMemberAction({ item->id }, Proto::MEMBER_ACTION::Action::MoveToTop);
    }
}

void MemberList::MuteAll()
{
    std::lock_guard<std::recursive_mutex> lock(itemsMutex);

    std::vector<int64_t> ids;

    for (auto &m : items)
    {
        ids.emplace_back(m.id);
    }

    if (!ids.empty())
    {
        controller.SendMemberAction(ids, Proto::MEMBER_ACTION::Action::MuteMicrophone);
    }
}

void MemberList::TurnSpeak()
{
    std::lock_guard<std::recursive_mutex> lock(itemsMutex);
    auto *item = GetItem(list->selected_item());
    if (item)
    {
        controller.SendMemberAction({ item->id }, Proto::MEMBER_ACTION::Action::TurnSpeaker);
    }
}

void MemberList::Devices()
{
    devicesMenu->show_on_control(devicesButton, 0);
}

void MemberList::TurnCamera()
{
    std::lock_guard<std::recursive_mutex> lock(itemsMutex);
    auto *item = GetItem(list->selected_item());
    if (item)
    {
        controller.SendMemberAction({ item->id }, Proto::MEMBER_ACTION::Action::TurnCamera);
    }
}

void MemberList::TurnMicrophone()
{
    std::lock_guard<std::recursive_mutex> lock(itemsMutex);
    auto *item = GetItem(list->selected_item());
    if (item)
    {
        controller.SendMemberAction({ item->id }, Proto::MEMBER_ACTION::Action::TurnMicrophone);
    }
}

void MemberList::TurnDemostration()
{
    std::lock_guard<std::recursive_mutex> lock(itemsMutex);
    auto *item = GetItem(list->selected_item());
    if (item)
    {
        controller.SendMemberAction({ item->id }, Proto::MEMBER_ACTION::Action::TurnDemonstration);
    }
}

void MemberList::ContactDialogCallback(ContactDialogMode mode, const Storage::Contacts &contacts)
{
    for (auto &c : contacts)
    {
        controller.SendConnectToConference(controller.GetCurrentConference().tag, c.id, -1, static_cast<uint32_t>(Proto::SEND_CONNECT_TO_CONFERENCE::Flag::AddMember));
    }
}

}
