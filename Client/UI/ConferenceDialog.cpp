/**
 * ConferenceDialog.cpp - Contains conference dialog impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <wui/locale/locale.hpp>
#include <wui/theme/theme.hpp>
#include <wui/system/tools.hpp>
#include <wui/system/uri_tools.hpp>

#include <UI/ConferenceDialog.h>

#include <Common/BitHelpers.h>
#include <Proto/ConferenceGrants.h>
#include <Proto/MemberGrants.h>
#include <Proto/CmdConferenceUpdateResponse.h>

#include <boost/algorithm/string.hpp>

#include <random>

#include <resource.h>

namespace Client
{

std::string RandomString(std::size_t length)
{
    static const std::string alphabet = "aceimnorstuvwxz"; //"abcdefghijklmnopqrstuvwxyz";
    static std::default_random_engine rng((uint32_t)std::time(nullptr));
    static std::uniform_int_distribution<std::size_t> distribution(0, alphabet.size() - 1);

    std::string str;
    while (str.size() < length) str += alphabet[distribution(rng)];
    return str;
}

ConferenceDialog::ConferenceDialog(std::weak_ptr<wui::window> parentWindow__, Controller::IController &controller_, Storage::Storage &storage_)
    : parentWindow_(parentWindow__),
    controller(controller_),
    storage(storage_),
    editedConf(),
    window(new wui::window()),
    baseSheet(), membersSheet(), optionsSheet(),
    nameText(), nameInput(),
    tagText(), tagInput(),
    typeText(), typeSelect(),
    descriptionText(), descriptionInput(),
    linkText(), linkInput(),
    openConfLinkButton(),
    deactiveConfCheck(),

    addMemberButton(),
    deleteMemberButton(),
    upMemberButton(),
    downMemberButton(),
    membersList(),
    
    disableMicrophoneIfNoSpeakCheck(),
    disableCameraIfNoSpeakCheck(),
    enableCameraOnConnectCheck(),
    enableMicrophoneOnConnectCheck(),
    denyTurnSpeakCheck(),
    denyTurnMicrophoneCheck(),
    denyTurnCameraCheck(),
    dontAskTurnDevices(),
    disableSpeakerChangeCheck(),
    denyRecordCheck(),
    autoConnectCheck(),
    denySelfConnectCheck(),

    updateButton(), startButton(),
    closeButton(),
    messageBox(),

    switchOnImage(),
    switchOffImage(),

    groupRolledImg(), groupExpandedImg(),
    ownerImg(), moderatorImg(), ordinaryImg(), readOnlyImg(),

    startPushed(false),

    readyCallback(),

    currentSheet(CurrentSheet::Base),
    contactDialog(storage, controller, std::bind(&ConferenceDialog::ContactDialogCallback, this, std::placeholders::_1, std::placeholders::_2))
{
}

ConferenceDialog::~ConferenceDialog()
{
}

void ConferenceDialog::Run(const Proto::Conference &editedConf_, std::function<void(std::string_view )> readyCallback_)
{
    window->set_transient_for(parentWindow_.lock());
    baseSheet = std::make_shared<wui::button>(wui::locale("conference_dialog", "base_sheet"), std::bind(&ConferenceDialog::ShowBase, this), wui::button_view::sheet);
    membersSheet = std::make_shared<wui::button>(wui::locale("conference_dialog", "members_sheet"), std::bind(&ConferenceDialog::ShowMembers, this), wui::button_view::sheet);
    optionsSheet = std::make_shared<wui::button>(wui::locale("conference_dialog", "options_sheet"), std::bind(&ConferenceDialog::ShowOptions, this), wui::button_view::sheet);
    nameText = std::make_shared<wui::text>(wui::locale("conference_dialog", "name"));
    nameInput = std::make_shared<wui::input>();
    tagText = std::make_shared<wui::text>(wui::locale("conference_dialog", "tag"));
    tagInput = std::make_shared<wui::input>();
    tagInput->set_change_callback(std::bind(&ConferenceDialog::MakeURL, this, std::placeholders::_1));
    typeText = std::make_shared<wui::text>(wui::locale("conference_dialog", "conference_type"));
    typeSelect = std::make_shared<wui::select>();
    typeSelect->set_items({
            { 1, wui::locale("conference", "symmetric") },
            { 2, wui::locale("conference", "asymmetric") },
            { 3, wui::locale("conference", "asymmetric_sym_sound") }
        });
    descriptionText = std::make_shared<wui::text>(wui::locale("conference_dialog", "description"));
    descriptionInput = std::make_shared<wui::input>();
    linkText = std::make_shared<wui::text>(wui::locale("conference_dialog", "link"));
    linkInput = std::make_shared<wui::input>("", wui::input_view::readonly);
    openConfLinkButton = std::make_shared<wui::button>(wui::locale("conference_dialog", "open_conf_link"), [this]() { wui::open_uri(linkInput->text()); });
    deactiveConfCheck = std::make_shared<wui::button>(wui::locale("conference_dialog", "deactivate_conference"), std::bind(&ConferenceDialog::DeactivateChange, this), wui::button_view::switcher);

    membersList = std::make_shared<wui::list>();
    membersList->set_item_height_callback([](int32_t, int32_t& h) { h = XBITMAP; });
    membersList->set_draw_callback(std::bind(&ConferenceDialog::DrawMemberItem, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    membersList->set_item_click_callback([this](wui::list::click_button btn, int32_t nItem, int32_t x, int32_t) { if (btn == wui::list::click_button::left) ClickMemberItem(nItem, x - membersList->position().left); });
    membersList->update_columns({
        { WND_WIDTH - 20 - MEMBER_SWITCHER_COLUMN * 3, wui::locale("common", "name") },
        { MEMBER_SWITCHER_COLUMN, wui::locale("conference", "owner") },
        { MEMBER_SWITCHER_COLUMN, wui::locale("conference", "moderator") },
        { MEMBER_SWITCHER_COLUMN, wui::locale("conference", "readonly") }
        });
    addMemberButton = std::make_shared<wui::button>(wui::locale("conference_dialog", "add_member"), std::bind(&ConferenceDialog::AddMember, this), wui::button_view::image, IMG_MC_ADD_TO_MEMBERS, BTN_SIZE, wui::button::tc_tool);
    deleteMemberButton = std::make_shared<wui::button>(wui::locale("conference_dialog", "del_member"), std::bind(&ConferenceDialog::DeleteMember, this), wui::button_view::image, IMG_MC_DEL_MEMBER, BTN_SIZE, wui::button::tc_tool);
    upMemberButton = std::make_shared<wui::button>(wui::locale("conference_dialog", "up_member"), std::bind(&ConferenceDialog::UpMember, this), wui::button_view::image, IMG_MC_UP_MEMBER, BTN_SIZE, wui::button::tc_tool);
    downMemberButton = std::make_shared<wui::button>(wui::locale("conference_dialog", "down_member"), std::bind(&ConferenceDialog::DownMember, this), wui::button_view::image, IMG_MC_DOWN_MEMBER, BTN_SIZE, wui::button::tc_tool);

    disableMicrophoneIfNoSpeakCheck = std::make_shared<wui::button>(wui::locale("conference_dialog", "disable_microphone_if_no_speak"), std::bind(&ConferenceDialog::DisableMicrophoneIfNoSpeak, this), wui::button_view::switcher);
    disableCameraIfNoSpeakCheck = std::make_shared<wui::button>(wui::locale("conference_dialog", "disable_camera_if_no_speak"), std::bind(&ConferenceDialog::DisableCameraIfNoSpeak, this), wui::button_view::switcher);
    
    enableCameraOnConnectCheck = std::make_shared<wui::button>(wui::locale("conference_dialog", "enable_camera_on_connect"), std::bind(&ConferenceDialog::EnableCameraOnConnect, this), wui::button_view::switcher);
    enableMicrophoneOnConnectCheck = std::make_shared<wui::button>(wui::locale("conference_dialog", "enable_microphone_on_connect"), std::bind(&ConferenceDialog::EnableMicrophoneOnConnect, this), wui::button_view::switcher);
    
    denyTurnSpeakCheck = std::make_shared<wui::button>(wui::locale("conference_dialog", "deny_turn_speak"), std::bind(&ConferenceDialog::DenyTurnSpeak, this), wui::button_view::switcher);
    denyTurnMicrophoneCheck = std::make_shared<wui::button>(wui::locale("conference_dialog", "deny_turn_microphone"), std::bind(&ConferenceDialog::DenyTurnMicrophone, this), wui::button_view::switcher);
    denyTurnCameraCheck = std::make_shared<wui::button>(wui::locale("conference_dialog", "deny_turn_camera"), std::bind(&ConferenceDialog::DenyTurnCamera, this), wui::button_view::switcher);

    dontAskTurnDevices = std::make_shared<wui::button>(wui::locale("conference_dialog", "dont_ask_turn_devices"), std::bind(&ConferenceDialog::DontAskTurnDevices, this), wui::button_view::switcher);
    disableSpeakerChangeCheck = std::make_shared<wui::button>(wui::locale("conference_dialog", "disable_speaker_change"), std::bind(&ConferenceDialog::DisableSpeakerChange, this), wui::button_view::switcher);

    denyRecordCheck = std::make_shared<wui::button>(wui::locale("conference_dialog", "deny_record"), std::bind(&ConferenceDialog::DenyRecordChange, this), wui::button_view::switcher);

    autoConnectCheck = std::make_shared<wui::button>(wui::locale("conference_dialog", "auto_connect_on_start"), std::bind(&ConferenceDialog::AutoConnectChange, this), wui::button_view::switcher);
    denySelfConnectCheck = std::make_shared<wui::button>(wui::locale("conference_dialog", "deny_self_connect"), std::bind(&ConferenceDialog::DenySelfConnectChange, this), wui::button_view::switcher);

    updateButton = std::make_shared<wui::button>(wui::locale("button", editedConf_.tag.empty() ? "create" : "change"), std::bind(&ConferenceDialog::Update, this), "green_button");
    startButton = std::make_shared<wui::button>(wui::locale("button", "start"), std::bind(&ConferenceDialog::Start, this));
    closeButton = std::make_shared<wui::button>(wui::locale("button", "close"), std::bind(&ConferenceDialog::Close, this));
    messageBox = std::make_shared<wui::message>(window);

    switchOnImage = std::make_shared<wui::image>(wui::theme_image(wui::button::ti_switcher_on));
    switchOffImage = std::make_shared<wui::image>(wui::theme_image(wui::button::ti_switcher_off));

    groupRolledImg = std::make_shared<wui::image>(IMG_CL_GROUP_ROLLED); groupExpandedImg = std::make_shared<wui::image>(IMG_CL_GROUP_EXPANDED);
    ownerImg = std::make_shared<wui::image>(IMG_ML_OWNER); moderatorImg = std::make_shared<wui::image>(IMG_ML_MODERATOR); ordinaryImg = std::make_shared<wui::image>(IMG_ML_ORDINARY); readOnlyImg = std::make_shared<wui::image>(IMG_ML_READONLY);

    currentSheet = CurrentSheet::Base;

    editedConf = editedConf_;
    readyCallback = readyCallback_;

    startPushed = false;

    bool newConference = editedConf.tag.empty();

    if (newConference)
    {
        editedConf.tag = RandomString(6);
        editedConf.founder_id = controller.GetMyClientId();
        editedConf.founder = controller.GetMyClientName();

        int32_t grants = 0;
        SetBit(grants, static_cast<int32_t>(Proto::MemberGrants::Presenter));
        editedConf.members.emplace_back(Proto::Member(controller.GetMyClientId(), Proto::MemberState::Undefined, "", controller.GetMyClientName(), "", {}, grants));
    }

    const auto SHEET_WIDTH = (WND_WIDTH - 20 - 10 * 3) / 3;

    window->add_control(baseSheet,    { 10, 30, SHEET_WIDTH, 55 });
    window->add_control(membersSheet, { 10 * 2 + SHEET_WIDTH, 30, 10 * 2 + SHEET_WIDTH * 2, 55 });
    window->add_control(optionsSheet, { 10 * 3 + SHEET_WIDTH * 2, 30, 10 * 3 + SHEET_WIDTH * 3, 55 });

    ShowBase();
        
    window->add_control(updateButton, { 10, WND_HEIGHT - 35, 130, WND_HEIGHT - 10 });
    window->add_control(startButton,  { 140, WND_HEIGHT - 35, 260, WND_HEIGHT - 10 });
    window->add_control(closeButton,  { WND_WIDTH - 110, WND_HEIGHT - 35, WND_WIDTH - 10, WND_HEIGHT - 10 });

    window->set_default_push_control(startButton);

    typeSelect->update_theme();
    addMemberButton->update_theme();
    deleteMemberButton->update_theme();
    upMemberButton->update_theme();
    downMemberButton->update_theme();
    
    switchOnImage->change_image(wui::theme_image(wui::button::ti_switcher_on));
    switchOffImage->change_image(wui::theme_image(wui::button::ti_switcher_off));

    nameInput->set_text(editedConf.name);
    tagInput->set_text(editedConf.tag);
    typeSelect->select_item_id(editedConf.type != Proto::ConferenceType::Undefined ? static_cast<int32_t>(editedConf.type) : 1);
    descriptionInput->set_text(editedConf.descr);
    deactiveConfCheck->turn(BitIsSet(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::Deactivated)));
    disableMicrophoneIfNoSpeakCheck->turn(BitIsSet(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::DisableMicrophoneIfNoSpeak)));
    disableCameraIfNoSpeakCheck->turn(BitIsSet(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::DisableCameraIfNoSpeak)));
    enableCameraOnConnectCheck->turn(BitIsSet(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::EnableCameraOnConnect)));
    enableMicrophoneOnConnectCheck->turn(BitIsSet(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::EnableMicrophoneOnConnect)));
    denyTurnSpeakCheck->turn(BitIsSet(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::DenyTurnSpeak)));
    denyTurnMicrophoneCheck->turn(BitIsSet(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::DenyTurnMicrophone)));
    denyTurnCameraCheck->turn(BitIsSet(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::DenyTurnCamera)));
    dontAskTurnDevices->turn(BitIsSet(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::DontAskTurnDevices)));
    disableSpeakerChangeCheck->turn(BitIsSet(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::DisableSpeakerChange)));
    denyRecordCheck->turn(BitIsSet(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::DenyRecord)));
    autoConnectCheck->turn(BitIsSet(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::AutoConnect)));
    denySelfConnectCheck->turn(BitIsSet(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::DenySelfConnectMembers)));

    UpdateMemberList();

    window->init(wui::locale("conference_dialog", newConference ? "title_new" : "title_edit"), { -1, -1, WND_WIDTH, WND_HEIGHT }, wui::window_style::dialog,
        [this]() {
        readOnlyImg.reset(); ordinaryImg.reset(); moderatorImg.reset(); ownerImg.reset();
        groupExpandedImg.reset(); groupRolledImg.reset();

        switchOffImage.reset(); switchOnImage.reset();

        messageBox.reset();
        closeButton.reset();
        updateButton.reset();
        startButton.reset();

        dontAskTurnDevices.reset();
        denyTurnCameraCheck.reset();
        denyTurnMicrophoneCheck.reset();
        enableCameraOnConnectCheck.reset();
        enableMicrophoneOnConnectCheck.reset();
        disableSpeakerChangeCheck.reset();
        disableCameraIfNoSpeakCheck.reset();
        disableMicrophoneIfNoSpeakCheck.reset();
        denyTurnSpeakCheck.reset();
        denyRecordCheck.reset();
        autoConnectCheck.reset();
        denySelfConnectCheck.reset();
        
        membersList.reset();
        downMemberButton.reset();
        upMemberButton.reset();
        deleteMemberButton.reset();
        addMemberButton.reset();
        
        deactiveConfCheck.reset();
        openConfLinkButton.reset();
        linkInput.reset();
        linkText.reset();
        descriptionInput.reset();
        descriptionText.reset();
        typeSelect.reset();
        typeText.reset();
        tagInput.reset();
        tagText.reset();
        nameInput.reset();
        nameText.reset();

        optionsSheet.reset();
        membersSheet.reset();
        baseSheet.reset();

        controller.SetConferenceUpdateHandler(nullptr);
    });

    controller.SetConferenceUpdateHandler(std::bind(&ConferenceDialog::ConferenceUpdateCallback, this, std::placeholders::_1));
}

void ConferenceDialog::ShowBase()
{
    window->disable_draw();

    baseSheet->turn(true);
    membersSheet->turn(false);
    optionsSheet->turn(false);

    window->remove_control(addMemberButton);
    window->remove_control(deleteMemberButton);
    window->remove_control(upMemberButton);
    window->remove_control(downMemberButton);
    window->remove_control(membersList);

    window->remove_control(disableMicrophoneIfNoSpeakCheck);
    window->remove_control(disableCameraIfNoSpeakCheck);
    window->remove_control(enableCameraOnConnectCheck);
    window->remove_control(enableMicrophoneOnConnectCheck);
    window->remove_control(denyTurnSpeakCheck);
    window->remove_control(denyTurnMicrophoneCheck);
    window->remove_control(denyTurnCameraCheck);
    window->remove_control(dontAskTurnDevices);
    window->remove_control(disableSpeakerChangeCheck);
    window->remove_control(denyRecordCheck);
    window->remove_control(autoConnectCheck);
    window->remove_control(denySelfConnectCheck);

    wui::rect pos = { 10, 80, WND_WIDTH - 10, 95 };
    window->add_control(nameText, pos);
    wui::line_up_top_bottom(pos, 25, 5);
    window->add_control(nameInput, pos);
    wui::line_up_top_bottom(pos, 15, 5);
    window->add_control(tagText, pos);
    wui::line_up_top_bottom(pos, 25, 5);
    window->add_control(tagInput, pos);
    wui::line_up_top_bottom(pos, 15, 5);
    window->add_control(typeText, pos);
    wui::line_up_top_bottom(pos, 25, 5);
    window->add_control(typeSelect, pos);
    wui::line_up_top_bottom(pos, 15, 5);
    window->add_control(descriptionText, pos);
    wui::line_up_top_bottom(pos, 25, 5);
    window->add_control(descriptionInput, pos);
    wui::line_up_top_bottom(pos, 15, 15);
    window->add_control(linkText, pos);
    wui::line_up_top_bottom(pos, 25, 5);
    window->add_control(linkInput, { pos.left, pos.top, pos.right - 150, pos.bottom });
    window->add_control(openConfLinkButton, { pos.right - 140, pos.top, pos.right, pos.bottom });
    wui::line_up_top_bottom(pos, 35, 5);
    window->add_control(deactiveConfCheck, pos);

    //window->set_focused(nameInput);

    window->enable_draw();
    window->redraw(window->parent().lock()
        ? window->position()
        : wui::rect{ 0, 0, window->position().width(), window->position().height() }, true);
}

void ConferenceDialog::ShowMembers()
{
    window->disable_draw();

    baseSheet->turn(false);
    membersSheet->turn(true);
    optionsSheet->turn(false);

    window->remove_control(nameText);
    window->remove_control(nameInput);
    window->remove_control(tagText);
    window->remove_control(tagInput);
    window->remove_control(typeText);
    window->remove_control(typeSelect);
    window->remove_control(descriptionText);
    window->remove_control(descriptionInput);
    window->remove_control(linkText);
    window->remove_control(linkInput);
    window->remove_control(openConfLinkButton);
    window->remove_control(deactiveConfCheck);

    window->remove_control(disableMicrophoneIfNoSpeakCheck);
    window->remove_control(disableCameraIfNoSpeakCheck);
    window->remove_control(enableCameraOnConnectCheck);
    window->remove_control(enableMicrophoneOnConnectCheck);
    window->remove_control(denyTurnSpeakCheck);
    window->remove_control(denyTurnMicrophoneCheck);
    window->remove_control(denyTurnCameraCheck);
    window->remove_control(dontAskTurnDevices);
    window->remove_control(disableSpeakerChangeCheck);
    window->remove_control(denyRecordCheck);
    window->remove_control(autoConnectCheck);
    window->remove_control(denySelfConnectCheck);

    wui::rect pos = { 10, 80, 10 + 32, 80 + 32 };
    window->add_control(addMemberButton, pos);
    wui::line_up_left_right(pos, 32, 4);
    window->add_control(deleteMemberButton, pos);
    wui::line_up_left_right(pos, 32, 4);
    window->add_control(upMemberButton, pos);
    wui::line_up_left_right(pos, 32, 4);
    window->add_control(downMemberButton, pos);
    window->add_control(membersList, { 10, 80 + 42, WND_WIDTH - 10, WND_HEIGHT - 45 });

    window->enable_draw();
    window->redraw(window->parent().lock()
        ? window->position()
        : wui::rect{ 0, 0, window->position().width(), window->position().height() }, true);
}

void ConferenceDialog::ShowOptions()
{
    window->disable_draw();

    baseSheet->turn(false);
    membersSheet->turn(false);
    optionsSheet->turn(true);

    window->remove_control(nameText);
    window->remove_control(nameInput);
    window->remove_control(tagText);
    window->remove_control(tagInput);
    window->remove_control(typeText);
    window->remove_control(typeSelect);
    window->remove_control(descriptionText);
    window->remove_control(descriptionInput);
    window->remove_control(linkText);
    window->remove_control(linkInput);
    window->remove_control(openConfLinkButton);
    window->remove_control(deactiveConfCheck);

    window->remove_control(addMemberButton);
    window->remove_control(deleteMemberButton);
    window->remove_control(upMemberButton);
    window->remove_control(downMemberButton);
    window->remove_control(membersList);

    wui::rect pos = { 10, 80, WND_WIDTH - 10, 80 + 15 };
    window->add_control(disableMicrophoneIfNoSpeakCheck, pos);
    wui::line_up_top_bottom(pos, 15, 10);
    window->add_control(disableCameraIfNoSpeakCheck, pos);
    wui::line_up_top_bottom(pos, 15, 20);

    window->add_control(enableCameraOnConnectCheck, pos);
    wui::line_up_top_bottom(pos, 15, 10);
    window->add_control(enableMicrophoneOnConnectCheck, pos);
    wui::line_up_top_bottom(pos, 15, 20);

    window->add_control(denyTurnSpeakCheck, pos);
    wui::line_up_top_bottom(pos, 15, 10);
    window->add_control(denyTurnMicrophoneCheck, pos);
    wui::line_up_top_bottom(pos, 15, 10);
    window->add_control(denyTurnCameraCheck, pos);
    wui::line_up_top_bottom(pos, 15, 20);

    window->add_control(dontAskTurnDevices, pos);
    wui::line_up_top_bottom(pos, 15, 10);
    window->add_control(disableSpeakerChangeCheck, pos);
    wui::line_up_top_bottom(pos, 15, 10);
    window->add_control(denyRecordCheck, pos);
    wui::line_up_top_bottom(pos, 15, 20);

    window->add_control(autoConnectCheck, pos);
    wui::line_up_top_bottom(pos, 15, 10);
    window->add_control(denySelfConnectCheck, pos);

    window->enable_draw();
    window->redraw(window->parent().lock()
        ? window->position()
        : wui::rect{ 0, 0, window->position().width(), window->position().height() }, true);
}

void ConferenceDialog::MakeURL(std::string_view tag)
{
    if (!tag.empty())
    {
        auto serverAddress = controller.GetServerAddress();

        std::vector<std::string> vals;
        boost::split(vals, serverAddress, boost::is_any_of(":"));
        if (vals.size() == 2 && (vals[1] == "5060" || vals[1] == "80" || vals[1] == "443"))
        {
            serverAddress = vals[0];
        }

        linkInput->set_text((controller.IsSecureConnection() ? "https://" : "http://") +
            serverAddress + "/conferences/" + std::string(tag));
    }
    else
    {
        linkInput->set_text("");
    }
}

void ConferenceDialog::AddMember()
{
    contactDialog.Run(window, ContactDialogMode::InviteToConference);
}

void ConferenceDialog::DeleteMember()
{
    auto item = GetMemberItem(membersList->selected_item());
    if (!item)
    {
        return;
    }

    auto it = std::find(editedConf.members.begin(), editedConf.members.end(), item->id);
    if (it != editedConf.members.end())
    {
        if (it->id == controller.GetMyClientId())
        {
            return messageBox->show(wui::locale("message", "cant_remove_owner_from_conference"),
                wui::locale("message", "title_error"),
                wui::message_icon::alert,
                wui::message_button::ok);
        }

        editedConf.members.erase(it);

        UpdateMemberList();
    }
}

void ConferenceDialog::UpMember()
{
    auto memberNumber = membersList->selected_item();
    if (memberNumber == -1)
    {
        return;
    }

    int32_t newPos = memberNumber > 0 ? memberNumber - 1 : static_cast<int32_t>(editedConf.members.size()) - 1;
    std::swap(editedConf.members[memberNumber], editedConf.members[newPos]);

    membersList->select_item(newPos);

    UpdateMemberList();
}

void ConferenceDialog::DownMember()
{
    auto memberNumber = membersList->selected_item();
    if (memberNumber == -1)
    {
        return;
    }

    int32_t newPos = memberNumber != editedConf.members.size() - 1 ? memberNumber + 1 : 0;
    std::swap(editedConf.members[memberNumber], editedConf.members[newPos]);

    membersList->select_item(newPos);

    UpdateMemberList();
}

void ConferenceDialog::DenyTurnSpeak()
{
    if (denyTurnSpeakCheck->turned())
    {
        SetBit(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::DenyTurnSpeak));
    }
    else
    {
        ClearBit(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::DenyTurnSpeak));
    }
}

void ConferenceDialog::DisableMicrophoneIfNoSpeak()
{
    if (disableMicrophoneIfNoSpeakCheck->turned())
    {
        SetBit(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::DisableMicrophoneIfNoSpeak));
    }
    else
    {
        ClearBit(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::DisableMicrophoneIfNoSpeak));
    }
}

void ConferenceDialog::DisableCameraIfNoSpeak()
{
    if (disableCameraIfNoSpeakCheck->turned())
    {
        SetBit(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::DisableCameraIfNoSpeak));
    }
    else
    {
        ClearBit(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::DisableCameraIfNoSpeak));
    }
}

void ConferenceDialog::EnableCameraOnConnect()
{
    if (enableCameraOnConnectCheck->turned())
    {
        SetBit(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::EnableCameraOnConnect));
    }
    else
    {
        ClearBit(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::EnableCameraOnConnect));
    }
}

void ConferenceDialog::EnableMicrophoneOnConnect()
{
    if (enableMicrophoneOnConnectCheck->turned())
    {
        SetBit(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::EnableMicrophoneOnConnect));
    }
    else
    {
        ClearBit(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::EnableMicrophoneOnConnect));
    }
}

void ConferenceDialog::DisableSpeakerChange()
{
    if (disableSpeakerChangeCheck->turned())
    {
        SetBit(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::DisableSpeakerChange));
    }
    else
    {
        ClearBit(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::DisableSpeakerChange));
    }
}

void ConferenceDialog::DenyTurnMicrophone()
{
    if (denyTurnMicrophoneCheck->turned())
    {
        SetBit(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::DenyTurnMicrophone));
    }
    else
    {
        ClearBit(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::DenyTurnMicrophone));
    }
}

void ConferenceDialog::DenyTurnCamera()
{
    if (denyTurnCameraCheck->turned())
    {
        SetBit(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::DenyTurnCamera));
    }
    else
    {
        ClearBit(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::DenyTurnCamera));
    }
}

void ConferenceDialog::DontAskTurnDevices()
{
    if (dontAskTurnDevices->turned())
    {
        SetBit(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::DontAskTurnDevices));
    }
    else
    {
        ClearBit(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::DontAskTurnDevices));
    }
}

void ConferenceDialog::DenyRecordChange()
{
    if (denyRecordCheck->turned())
    {
        SetBit(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::DenyRecord));
    }
    else
    {
        ClearBit(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::DenyRecord));
    }
}

void ConferenceDialog::AutoConnectChange()
{
    if (autoConnectCheck->turned())
    {
        SetBit(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::AutoConnect));
    }
    else
    {
        ClearBit(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::AutoConnect));
    }
}

void ConferenceDialog::DenySelfConnectChange()
{
    if (denySelfConnectCheck->turned())
    {
        SetBit(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::DenySelfConnectMembers));
    }
    else
    {
        ClearBit(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::DenySelfConnectMembers));
    }
}

void ConferenceDialog::DeactivateChange()
{
    if (deactiveConfCheck->turned())
    {
        SetBit(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::Deactivated));
    }
    else
    {
        ClearBit(editedConf.grants, static_cast<int32_t>(Proto::ConferenceGrants::Deactivated));
    }
}

void ConferenceDialog::Update()
{
    if (nameInput->text().empty())
    {
        return messageBox->show(wui::locale("message", "conference_name_empty"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok, [this](wui::message_result) { ShowBase(); window->set_focused(nameInput); });
    }
    if (tagInput->text().empty())
    {
        return messageBox->show(wui::locale("message", "conference_tag_empty"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok, [this](wui::message_result) { ShowBase(); window->set_focused(tagInput); });
    }
    if (tagInput->text().find_first_not_of("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz") != std::string::npos)
    {
        return messageBox->show(wui::locale("message", "conference_tag_incorrect"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok, [this](wui::message_result) { ShowBase(); window->set_focused(tagInput); });
    }

    editedConf.name = nameInput->text();
    editedConf.tag = tagInput->text();
    editedConf.type = static_cast<Proto::ConferenceType>(typeSelect->selected_item().id);
    editedConf.descr = descriptionInput->text();

    Proto::CONFERENCE_UPDATE_RESPONSE::Command result;

    if (editedConf.id == 0)
    {
        controller.CreateConference(editedConf);
    }
    else
    {
        controller.EditConference(editedConf);
    }
}

void ConferenceDialog::Start()
{
    startPushed = true;
    Update();
}

void ConferenceDialog::Close()
{
    window->destroy();
}

void ConferenceDialog::UpdateMemberList()
{
    membersList->set_item_count(static_cast<int32_t>(editedConf.members.size()));
}

Proto::Member* ConferenceDialog::GetMemberItem(int32_t itemNumber)
{
    if (itemNumber != -1 && itemNumber < static_cast<int32_t>(editedConf.members.size()))
    {
        return &editedConf.members[itemNumber];
    }

    return nullptr;
}

void ConferenceDialog::DrawMemberItem(wui::graphic &gr, int32_t nItem, const wui::rect &itemRect_, wui::list::item_state state)
{
    auto item = GetMemberItem(nItem);
    if (!item)
    {
        return;
    }

    auto border_width = wui::theme_dimension(wui::list::tc, wui::list::tv_border_width);

    auto itemRect = itemRect_;

    if (itemRect.bottom > membersList->position().bottom - border_width)
    {
        itemRect.bottom = membersList->position().bottom - border_width;
    }

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

    auto textRect = itemRect_;
    textRect.move(border_width + XBITMAP + 6, (itemRect_.height() - gr.measure_text("Qq", font).height()) / 2);
    textRect.right -= border_width + XBITMAP + 6 + (WND_WIDTH / 2 - 265);

    auto name = item->name;
    truncate_line(name, gr, font, textRect.width(), 4);

    gr.draw_text(textRect, name, textColor, font);

    std::shared_ptr<wui::image> img = ordinaryImg;

    auto switcherLeft = membersList->position().width() - MEMBER_SWITCHER_COLUMN;
    auto switcherSpan = (MEMBER_SWITCHER_COLUMN - MEMBER_SWITCHER) / 2;

    if (BitIsSet(item->grants, static_cast<int32_t>(Proto::MemberGrants::ReadOnly)))
    {
        img = readOnlyImg;
        
        switchOnImage->set_position({ switcherLeft + switcherSpan, itemRect.top + 4, switcherLeft + switcherSpan + MEMBER_SWITCHER, itemRect.top + 28 });
        switchOnImage->draw(gr, { 0 });
    }
    else
    {
        switchOffImage->set_position({ switcherLeft + switcherSpan, itemRect.top + 4, switcherLeft + switcherSpan + MEMBER_SWITCHER, itemRect.top + 28 });
        switchOffImage->draw(gr, { 0 });
    }

    switcherLeft -= MEMBER_SWITCHER_COLUMN;

    if (BitIsSet(item->grants, static_cast<int32_t>(Proto::MemberGrants::Moderator)))
    {
        img = moderatorImg;

        switchOnImage->set_position({ switcherLeft + switcherSpan, itemRect.top + 4, switcherLeft + switcherSpan + MEMBER_SWITCHER, itemRect.top + 28 });
        switchOnImage->draw(gr, { 0 });
    }
    else
    {
        switchOffImage->set_position({ switcherLeft + switcherSpan, itemRect.top + 4, switcherLeft + switcherSpan + MEMBER_SWITCHER, itemRect.top + 28 });
        switchOffImage->draw(gr, { 0 });
    }

    switcherLeft -= MEMBER_SWITCHER_COLUMN;

    if (BitIsSet(item->grants, static_cast<int32_t>(Proto::MemberGrants::Presenter)))
    {
        img = ownerImg;

        switchOnImage->set_position({ switcherLeft + switcherSpan, itemRect.top + 4, switcherLeft + switcherSpan + MEMBER_SWITCHER, itemRect.top + 28 });
        switchOnImage->draw(gr, { 0 });
    }
    else
    {
        switchOffImage->set_position({ switcherLeft + switcherSpan, itemRect.top + 4, switcherLeft + switcherSpan + MEMBER_SWITCHER, itemRect.top + 28 });
        switchOffImage->draw(gr, { 0 });
    }

    img->set_position({ itemRect.left,
        itemRect.top,
        itemRect.left + XBITMAP,
        itemRect.top + XBITMAP });
    img->draw(gr, { 0 });
}

void ConferenceDialog::ClickMemberItem(int32_t nItem, int32_t xPos)
{
    auto firstSwitcherPos = membersList->position().width() - MEMBER_SWITCHER_COLUMN * 3;
    if (xPos < firstSwitcherPos)
    {
        return;
    }

    auto item = GetMemberItem(nItem);
    if (!item)
    {
        return;
    }

    auto switcherNumber = (xPos - firstSwitcherPos) / MEMBER_SWITCHER_COLUMN;
    switch (switcherNumber)
    {
        case 0:
            if (BitIsSet(item->grants, static_cast<int32_t>(Proto::MemberGrants::Presenter)))
            {
                ClearBit(item->grants, static_cast<int32_t>(Proto::MemberGrants::Presenter));
            }
            else
            {
                SetBit(item->grants, static_cast<int32_t>(Proto::MemberGrants::Presenter));
                ClearBit(item->grants, static_cast<int32_t>(Proto::MemberGrants::Moderator));
                ClearBit(item->grants, static_cast<int32_t>(Proto::MemberGrants::ReadOnly));
            }
        break;
        case 1:
            if (BitIsSet(item->grants, static_cast<int32_t>(Proto::MemberGrants::Moderator)))
            {
                ClearBit(item->grants, static_cast<int32_t>(Proto::MemberGrants::Moderator));
            }
            else
            {
                SetBit(item->grants, static_cast<int32_t>(Proto::MemberGrants::Moderator));
                ClearBit(item->grants, static_cast<int32_t>(Proto::MemberGrants::Presenter));
                ClearBit(item->grants, static_cast<int32_t>(Proto::MemberGrants::ReadOnly));
            }
        break;
        case 2:
            if (BitIsSet(item->grants, static_cast<int32_t>(Proto::MemberGrants::ReadOnly)))
            {
                ClearBit(item->grants, static_cast<int32_t>(Proto::MemberGrants::ReadOnly));
            }
            else
            {
                SetBit(item->grants, static_cast<int32_t>(Proto::MemberGrants::ReadOnly));
                ClearBit(item->grants, static_cast<int32_t>(Proto::MemberGrants::Presenter));
                ClearBit(item->grants, static_cast<int32_t>(Proto::MemberGrants::Moderator));
            }
        break;
    }

    UpdateMemberList();
}

void ConferenceDialog::ConferenceUpdateCallback(const Proto::CONFERENCE_UPDATE_RESPONSE::Command& response)
{
    if (response.result == Proto::CONFERENCE_UPDATE_RESPONSE::Result::DuplicateTag)
    {
        return messageBox->show(wui::locale("message", "conference_tag_duplicate"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok, [this](wui::message_result) { ShowBase(); window->set_focused(tagInput); });
    }
    else if (response.result == Proto::CONFERENCE_UPDATE_RESPONSE::Result::OK)
    {
        bool newConfCreated = editedConf.id == 0;

        editedConf.id = response.id;
        editedConf.tag = tagInput->text();

        updateButton->set_caption(wui::locale("button", "change"));

        if (startPushed)
        {
            window->destroy();

            if (readyCallback)
            {
                readyCallback(editedConf.tag);
            }
        }
        else
        {
            messageBox->show(wui::locale("message", newConfCreated ? "conference_created" : "conference_updated"),
                wui::locale("message", "title_confirmation"),
                wui::message_icon::information,
                wui::message_button::ok);
        }
    }
    else
    {
        return messageBox->show(wui::locale("message", "unspecified_error_or_not_response"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok);
    }
}

void ConferenceDialog::ContactDialogCallback(ContactDialogMode mode, const Storage::Contacts &contacts)
{
    for (auto &c : contacts)
    {
        if (std::find(editedConf.members.begin(), editedConf.members.end(), c.id) == editedConf.members.end())
        {
            editedConf.members.emplace_back(Proto::Member(c.id, Proto::MemberState::Undefined, "", c.name, "", {}, 0));
        }
    }

    UpdateMemberList();
}

}
