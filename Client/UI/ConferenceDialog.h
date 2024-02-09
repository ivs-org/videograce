/**
 * ConferenceDialog.h - Contains conference dialog header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#pragma once

#include <wui/window/window.hpp>
#include <wui/control/text.hpp>
#include <wui/control/input.hpp>
#include <wui/control/select.hpp>
#include <wui/control/list.hpp>
#include <wui/control/button.hpp>
#include <wui/control/message.hpp>

#include <Controller/IController.h>
#include <Storage/Storage.h>

#include <UI/ContactDialog.h>

#include <string>
#include <memory>
#include <functional>

namespace Client
{

class ConferenceDialog
{
public:
    ConferenceDialog(std::weak_ptr<wui::window> parentWindow_, Controller::IController &controller_, Storage::Storage &storage_);
    ~ConferenceDialog();

    void Run(const Proto::Conference &editedConf_ = Proto::Conference(), std::function<void(std::string_view )> readyCallback = [](std::string_view ) {});

private:
    static constexpr int32_t WND_WIDTH = 530, WND_HEIGHT = 600,
        BTN_SIZE = 32, XBITMAP = 32,
        MEMBER_SWITCHER_COLUMN = 75, MEMBER_SWITCHER = 38;

    std::weak_ptr<wui::window> parentWindow_;
    Controller::IController &controller;
    Storage::Storage &storage;
    Proto::Conference editedConf;

    std::shared_ptr<wui::window> window;
    std::shared_ptr<wui::button> baseSheet, membersSheet, optionsSheet;
    std::shared_ptr<wui::text> nameText;
    std::shared_ptr<wui::input> nameInput;
    std::shared_ptr<wui::text> tagText;
    std::shared_ptr<wui::input> tagInput;
    std::shared_ptr<wui::text> typeText;
    std::shared_ptr<wui::select> typeSelect;
    std::shared_ptr<wui::text> descriptionText;
    std::shared_ptr<wui::input> descriptionInput;
    std::shared_ptr<wui::text> linkText;
    std::shared_ptr<wui::input> linkInput;
    std::shared_ptr<wui::button> openConfLinkButton;
    std::shared_ptr<wui::button> deactiveConfCheck;

    std::shared_ptr<wui::button> addMemberButton;
    std::shared_ptr<wui::button> deleteMemberButton;
    std::shared_ptr<wui::button> upMemberButton;
    std::shared_ptr<wui::button> downMemberButton;
    std::shared_ptr<wui::list> membersList;
    
    std::shared_ptr<wui::button> disableMicrophoneIfNoSpeakCheck;
    std::shared_ptr<wui::button> disableCameraIfNoSpeakCheck;
    std::shared_ptr<wui::button> enableCameraOnConnectCheck;
    std::shared_ptr<wui::button> enableMicrophoneOnConnectCheck;
    std::shared_ptr<wui::button> denyTurnSpeakCheck;
    std::shared_ptr<wui::button> denyTurnMicrophoneCheck;
    std::shared_ptr<wui::button> denyTurnCameraCheck;
    std::shared_ptr<wui::button> dontAskTurnDevices;
    std::shared_ptr<wui::button> disableSpeakerChangeCheck;
    std::shared_ptr<wui::button> denyRecordCheck;
    std::shared_ptr<wui::button> autoConnectCheck;
    std::shared_ptr<wui::button> denySelfConnectCheck;
    
    std::shared_ptr<wui::button> updateButton, startButton, closeButton;
    std::shared_ptr<wui::message> messageBox;

    std::shared_ptr<wui::image> switchOnImage, switchOffImage;

    std::shared_ptr<wui::image> groupRolledImg, groupExpandedImg,
        ownerImg, moderatorImg, ordinaryImg, readOnlyImg;

    bool startPushed;

    std::function<void(std::string_view )> readyCallback;

    enum class CurrentSheet
    {
        Base,
        Members,
        Options
    };
    CurrentSheet currentSheet;

    ContactDialog contactDialog;

    void ShowBase();
    void ShowMembers();
    void ShowOptions();

    void MakeURL(std::string_view tag);

    void AddMember();
    void DeleteMember();
    void UpMember();
    void DownMember();

    void DenyTurnSpeak();
    void DisableMicrophoneIfNoSpeak();
    void DisableCameraIfNoSpeak();
    void EnableCameraOnConnect();
    void EnableMicrophoneOnConnect();
    void DisableSpeakerChange();
    void DenyTurnMicrophone();
    void DenyTurnCamera();
    void DontAskTurnDevices();
    void DenyRecordChange();
    void AutoConnectChange();
    void DenySelfConnectChange();
    void DeactivateChange();

    void Update();
    void Start();
    void Close();

    void UpdateMemberList();

    Proto::Member* GetMemberItem(int32_t itemNumber);

    void DrawMemberItem(wui::graphic &gr, int32_t nItem, const wui::rect &pos, wui::list::item_state state);
    void ClickMemberItem(int32_t nItem, int32_t xPos);

    void ConferenceUpdateCallback(const Proto::CONFERENCE_UPDATE_RESPONSE::Command &response);
    void ContactDialogCallback(ContactDialogMode mode, const Storage::Contacts &contacts);
};

}
