/**
 * ContactList.h - Contains contact list header
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

#include <UI/ContactDialog.h>
#include <UI/ConferenceDialog.h>

namespace Client
{

class IContactListCallback
{
public:
    virtual void ContactSelected(int64_t id, std::string_view name) = 0;
    virtual void ContactCall(std::string_view name) = 0;

    virtual void ConferenceSelected(std::string_view tag, std::string_view name) = 0;
    virtual void ConferenceConnect(std::string_view tag) = 0;
    
    virtual void ContactUnselected() = 0;

protected:
    ~IContactListCallback() {}
};

class ContactList
{
public:
    ContactList(Storage::Storage &storage_, Controller::IController &controller_, ConferenceDialog &conferenceDialog_, IContactListCallback &callback_);

    ~ContactList();

    void Run(std::weak_ptr<wui::window> parentWindow_);
    void End();

    void UpdateSize(int32_t width, int32_t height);

    enum class SelectionType
    {
        Undefined,
        Client,
        Conference
    };
    struct Selection
    {
        SelectionType type;
        std::string id;
        bool my;
    };

    Selection GetSelection();

    void SelectUser(int64_t id, std::string_view name);
    void SelectConference(std::string_view tag, std::string_view name);

private:
    static const int32_t XBITMAP = 32;

    Storage::Storage &storage;
    Controller::IController &controller;
    ConferenceDialog &conferenceDialog;
    IContactListCallback &callback;

    std::weak_ptr<wui::window> parentWindow_;
    
    std::shared_ptr<wui::input> searchInput;
    std::shared_ptr<wui::list> list;
    std::shared_ptr<wui::image> conferencesRolledImg, conferencesExpandedImg,
        symConfImg, asymConfImg, asymSSConfImg,
        symConfMyImg, asymConfMyImg, asymSSConfMyImg,
        groupRolledImg, groupExpandedImg,
        accountDeletedImg, accountOfflineImg, accountOnlineImg,
        ownerImg, moderatorImg, ordinaryImg, readOnlyImg;
    std::shared_ptr<wui::menu> popupMenu;
    std::shared_ptr<wui::button> addButton, deleteButton, editButton;
    std::shared_ptr<wui::image> separator;

    std::shared_ptr<wui::message> messageBox;

    ContactDialog contactDialog;

    enum class ItemType
    {
        Undefined,
        Group,
        GroupUser,
        Contact,
        ConferencesGroup,
        Conference,
        ConferenceUser
    };
    struct Item
    {
        int64_t id, group_id;

        ItemType type;
        std::string name, number;
        std::string tag;

        int32_t level;

        std::shared_ptr<wui::image> image;

        int32_t unreaded_count;

        bool my;

        Item()
            : id(0), group_id(-1), type(ItemType::Undefined), name(), number(), tag(), level(0), image(), unreaded_count(0), my(false) {}

        Item(int64_t id_, int64_t group_id_, ItemType type_, std::string_view name_, std::string_view number_, std::string_view tag_, int32_t level_, std::shared_ptr<wui::image> image_, int32_t unreaded_count_, bool my_)
            : id(id_), group_id(group_id_), type(type_), name(name_), number(number_), tag(tag_), level(level_), image(image_), unreaded_count(unreaded_count_), my(my_) {}
    };

    std::mutex itemsMutex;
    std::vector<Item> items;

    void UpdateItems();
    void MakeGroups();
    void MakeConferences();

    void SearchChange(std::string_view text);

    void DrawItem(wui::graphic &gr, int32_t nItem, const wui::rect &pos, wui::list::item_state state);
    void ClickItem(int32_t nItem);
    void ChangeItem(int32_t nItem);
    void ActivateItem(int32_t nItem);
    void RightClickItem(int32_t nItem, int32_t x, int32_t y);

    const Item* GetItem(int32_t itemNumber);

    void ContactDialogCallback(ContactDialogMode mode, const Storage::Contacts &contacts);

    void Add();
    void Edit();
    void Del();

    void AddContact();
    void DeleteContact(int64_t id);

    void AddConference();

    void CopyConferenceLink(std::string_view tag);

    void ExtractFromConferenceMember(std::string_view str, std::string &tag, int64_t &id);

    static constexpr const char * CONTACT = "contact:";    
    static constexpr const char * CONFERENCE = "conference:";
    static constexpr const char * CONFERENCE_MEMBER = "conference_member:";
    static constexpr const char * GROUP = "group:";
    static constexpr const char * CONFERENCES_GROUP = "conferences_group";
};

}
