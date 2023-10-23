/**
 * MemberList.h - Contains member list header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2015, 2022
 */

#pragma once

#include <wui/control/input.hpp>
#include <wui/control/button.hpp>
#include <wui/control/list.hpp>
#include <wui/control/image.hpp>
#include <wui/control/menu.hpp>

#include <Controller/IController.h>
#include <Controller/IMemberList.h>

#include <UI/ContactDialog.h>

namespace Client
{

class MemberList : public Controller::IMemberList
{
public:
    MemberList(Storage::Storage &storage_, Controller::IController &controller_);

    ~MemberList();

    void Run(std::weak_ptr<wui::window> parentWindow_);
    void End();

    void UpdateSize(int32_t width, int32_t height);
 
    /// IMemberList
    virtual std::recursive_mutex &GetItemsMutex() final;

    virtual void UpdateItem(const Proto::Member& item) final;
    virtual void DeleteItem(int64_t memberId) final;
    virtual void ClearItems() final;

    virtual const Controller::members_t &GetItems() const final;

    virtual bool ExistsMember(int64_t id) final;
    virtual std::string GetMemberName(int64_t id) final;

    virtual uint32_t GetMaximumInputBitrate() final;

    virtual uint32_t GetSpeakersCount() final;

    virtual bool IsMePresenter() final;
    virtual bool IsMeModerator() final;
    virtual bool IsMeReadOnly() final;

    virtual bool IsMeSpeaker() final;
    virtual void TurnMeSpeaker() final;

    virtual void Update() final;

    virtual void Add() final;
    virtual void Kick() final;
    virtual void ToTop() final;
    virtual void MuteAll() final;
    virtual void TurnSpeak() final;
    virtual void Devices() final;
    virtual void TurnCamera() final;
    virtual void TurnMicrophone() final;
    virtual void TurnDemostration() final;

private:
    static const int32_t XBITMAP = 32;

    Storage::Storage &storage;
    Controller::IController &controller;
    
    std::recursive_mutex itemsMutex;
    Controller::members_t items;

    std::weak_ptr<wui::window> parentWindow_;
    
    std::shared_ptr<wui::list> list;
    std::shared_ptr<wui::image> ownerImg, moderatorImg, ordinaryImg, readOnlyImg, deafImg,
        cameraMicrophoneEnabledImg, cameraEnabledImg, microphoneEnabledImg, speakEnabledImg;
    std::shared_ptr<wui::button> addButton, kickButton, toTopButton, muteAllButton, speakButton, devicesButton;
    std::shared_ptr<wui::image> separator0, separator1;
    std::shared_ptr<wui::image> cameraImage, microphoneImage, demonstrationImage;
    std::shared_ptr<wui::menu> devicesMenu;

    ContactDialog contactDialog;

    void DrawItem(wui::graphic &gr, int32_t nItem, const wui::rect &pos, wui::list::item_state state);

    const Proto::Member* GetItem(int32_t itemNumber);

    void ContactDialogCallback(ContactDialogMode mode, const Storage::Contacts &contacts);
};

}
