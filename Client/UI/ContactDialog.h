/**
 * ContactDialog.h - Contains add contact dialog header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#pragma once

#include <wui/window/window.hpp>
#include <wui/control/text.hpp>
#include <wui/control/input.hpp>
#include <wui/control/image.hpp>
#include <wui/control/list.hpp>
#include <wui/control/button.hpp>
#include <wui/control/message.hpp>

#include <string>
#include <memory>
#include <functional>

#include <Controller/IController.h>

namespace Client
{

enum class ContactDialogMode
{
    AddContacts,
    ConnectToConference,
    InviteToConference,
    ForwardMessage
};

class ContactDialog
{
public:
    ContactDialog(Storage::Storage &storage_, Controller::IController &controller, std::function<void(ContactDialogMode, const Storage::Contacts &)> callback);
    ~ContactDialog();

    void Run(std::weak_ptr<wui::window> parentWindow_, ContactDialogMode mode);

private:
    static const int32_t WND_WIDTH = 400, WND_HEIGHT = 550;
    static const int32_t XBITMAP = 32;

    Storage::Storage &storage;
    Controller::IController &controller;
    std::function<void(ContactDialogMode, const Storage::Contacts &)> callback;

    ContactDialogMode mode;

    std::shared_ptr<wui::window> window;
    std::shared_ptr<wui::input> input;
    std::shared_ptr<wui::button> searchButton;
    std::shared_ptr<wui::list> list;
    std::shared_ptr<wui::button> addButton, closeButton;

    std::shared_ptr<wui::image> checkOnImage, checkOffImage;

    std::shared_ptr<wui::image> groupRolledImg, groupExpandedImg, accountDeletedImg, onlineImg, offlineImg;

    std::shared_ptr<wui::message> messageBox;

    enum class ItemType
    {
        Undefined,
        Group,
        Contact        
    };
    struct Item
    {
        int64_t id, parent_id;

        ItemType type;
        std::string name, number;
        std::string tag;

        int32_t level;

        std::shared_ptr<wui::image> image;

        bool checked;

        Item()
            : id(0), parent_id(-1), type(ItemType::Undefined), name(), number(), tag(), level(0), image(), checked(false) {}

        Item(int64_t id_, int64_t parent_id_, ItemType type_, std::string_view name_, std::string_view number_, std::string_view tag_, int32_t level_, std::shared_ptr<wui::image> image_, bool checked_)
            : id(id_), parent_id(parent_id_), type(type_), name(name_), number(number_), tag(tag_), level(level_), image(image_), checked(checked_) {}
    };

    std::mutex itemsMutex;
    std::vector<Item> items;
    bool added;
    
    void Search();
    void Add();

    void MakeCurrentContactListItems();

    void DrawItem(wui::graphic &gr, int32_t nItem, const wui::rect &pos, wui::list::item_state state);

    void ClickItem(int32_t nItem, int32_t x, int32_t y);

    const Item *GetItem(int32_t nItem);

    void CheckChildren(int64_t parentID, bool checked);

    void ContactListCallback(const Storage::Contacts&);
};

}
