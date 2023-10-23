/**
 * ContactDialog.cpp - Contains question dialog impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <UI/ContactDialog.h>

#include <wui/locale/locale.hpp>
#include <wui/theme/theme.hpp>
#include <wui/system/tools.hpp>

#include <Common/BitHelpers.h>
#include <Proto/MemberGrants.h>
#include <Proto/GroupGrants.h>

#include <boost/algorithm/string/find.hpp>

#include <resource.h>

namespace Client
{

ContactDialog::ContactDialog(Storage::Storage &storage_, Controller::IController &controller_, std::function<void(ContactDialogMode, const Storage::Contacts &)> callback_)
    : storage(storage_), controller(controller_), callback(callback_),
    mode(ContactDialogMode::AddContacts),
    window(new wui::window()),
    input(),
    searchButton(),
    list(),
    addButton(),
    closeButton(),
    checkOnImage(), checkOffImage(),
    groupRolledImg(), groupExpandedImg(), accountDeletedImg(), onlineImg(), offlineImg(),
    messageBox(),
    itemsMutex(), items(),
    added(false)
{
}

ContactDialog::~ContactDialog()
{
}

void ContactDialog::Run(std::weak_ptr<wui::window> parentWindow_, ContactDialogMode mode_)
{
    mode = mode_;

    items.clear();
    added = false;

    std::string modeName = "";
    switch (mode)
    {
        case ContactDialogMode::AddContacts:
            modeName = "add_contacts";
        break;
        case ContactDialogMode::ConnectToConference:
            modeName = "connect_to_conference";
        break;
        case ContactDialogMode::InviteToConference:
            modeName = "invite_to_conference";
        break;
        case ContactDialogMode::ForwardMessage:
            modeName = "forward_message";
        break;
    }

    input = std::shared_ptr<wui::input>(new wui::input());
    searchButton = std::shared_ptr<wui::button>(new wui::button(wui::locale("button", "search"), std::bind(&ContactDialog::Search, this)));
    
    list = std::shared_ptr<wui::list>(new wui::list());
    list->set_item_height_callback([](int32_t, int32_t& h) { h = XBITMAP; });
    list->set_draw_callback(std::bind(&ContactDialog::DrawItem, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    list->set_item_activate_callback([this](int32_t) { Add(); });
    list->set_item_click_callback([this](wui::list::click_button btn, int32_t nItem, int32_t x, int32_t y) { if (btn == wui::list::click_button::left) ClickItem(nItem, x, y); });

    addButton = std::shared_ptr<wui::button>(new wui::button(wui::locale("button", "add"), std::bind(&ContactDialog::Add, this), "green_button"));
    closeButton = std::shared_ptr<wui::button>(new wui::button(wui::locale("button", "close"), [this]() { items.clear(); window->destroy(); }));
    checkOnImage = std::shared_ptr<wui::image>(new wui::image(IMG_CHECK_ON));
    checkOffImage = std::shared_ptr<wui::image>(new wui::image(IMG_CHECK_OFF));
    groupRolledImg = std::shared_ptr<wui::image>(new wui::image(IMG_CL_GROUP_ROLLED));
    groupExpandedImg = std::shared_ptr<wui::image>(new wui::image(IMG_CL_GROUP_EXPANDED));
    accountDeletedImg = std::shared_ptr<wui::image>(new wui::image(IMG_CL_ACCOUNT_DELETED));
    onlineImg = std::shared_ptr<wui::image>(new wui::image(IMG_CL_ACCOUNT_ONLINE));
    offlineImg = std::shared_ptr<wui::image>(new wui::image(IMG_CL_ACCOUNT_OFFLINE));

    messageBox = std::shared_ptr<wui::message>(new wui::message(window));

    window->set_transient_for(parentWindow_.lock());

    window->add_control(input, { 10, 60, WND_WIDTH - 120, 85 });
    window->add_control(searchButton, { WND_WIDTH - 110, 60, WND_WIDTH - 10, 85 });
    window->add_control(list, { 10, 95, WND_WIDTH - 10, WND_HEIGHT - 50 });
    window->add_control(addButton, { WND_WIDTH - 230, WND_HEIGHT - 40, WND_WIDTH - 130, WND_HEIGHT - 10 });
    window->add_control(closeButton, { WND_WIDTH - 110, WND_HEIGHT - 40, WND_WIDTH - 10, WND_HEIGHT - 10 });

    window->set_default_push_control(addButton);

    window->init(wui::locale("contact_dialog", modeName), { -1, -1, WND_WIDTH, WND_HEIGHT }, wui::window_style::dialog, [this]() {
        messageBox.reset();
        checkOnImage.reset();
        checkOffImage.reset();
        groupRolledImg.reset();
        groupExpandedImg.reset();
        accountDeletedImg.reset();
        offlineImg.reset();
        onlineImg.reset();
        closeButton.reset();
        addButton.reset();
        list.reset();
        searchButton.reset();
        input.reset();

        Storage::Contacts selectedContacts;
        if (added)
        {
            std::lock_guard<std::mutex> lock(itemsMutex);
            for (auto &item : items)
            {
                if (item.type == ItemType::Contact && item.checked)
                {
                    selectedContacts.emplace_back(Proto::Member(item.id, item.name));
                }
            }
        }
        callback(mode, selectedContacts);
    });

    if (mode != ContactDialogMode::AddContacts)
    {
        MakeItems();
    }
}

void ContactDialog::Search()
{
    switch (mode)
    {
        case ContactDialogMode::AddContacts:
            items.clear();
            controller.SearchContact(input->text());
            MakeItems();
        break;
        case ContactDialogMode::ConnectToConference:
        case ContactDialogMode::InviteToConference:
        case ContactDialogMode::ForwardMessage:
        {
            int pos = 0;
            bool finded = false; 
            std::lock_guard<std::mutex> lock(itemsMutex);
            for (const auto &item : items)
            {
                finded = boost::ifind_first(item.name, input->text());
                if (finded)
                {
                    break;
                }
                ++pos;
            }
            if (finded)
            {
                list->select_item(pos);
            }
        }
        break;
    }
}

void ContactDialog::Add()
{
    bool hasSelected = false;
    {
        std::lock_guard<std::mutex> lock(itemsMutex);
        for (auto &item : items)
        {
            if (item.checked)
            {
                hasSelected = true;
                break;
            }
        }

        if (!hasSelected)
        {
            auto selectedItem = list->selected_item();
            if (selectedItem < items.size())
            {
                if (items[selectedItem].type != ItemType::Group)
                {
                    items[selectedItem].checked = true;
                    hasSelected = true;
                }
            }
        }
    }

    if (!hasSelected)
    {
        return messageBox->show(wui::locale("message", "no_contacts_selected"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok);
    }

    added = true;
    window->destroy();
}

void ContactDialog::MakeItems()
{
    std::lock_guard<std::mutex> lock(itemsMutex);

    items.clear();

    switch (mode)
    {
        case ContactDialogMode::AddContacts:
            MakeFindedContactListItems();
        break;
        case ContactDialogMode::ConnectToConference:
        case ContactDialogMode::InviteToConference:
        case ContactDialogMode::ForwardMessage:
            MakeCurrentContactListItems();
        break;
    }

    list->set_item_count(static_cast<int32_t>(items.size()));
}

void ContactDialog::MakeFindedContactListItems()
{
    auto &contacts = controller.GetFindedContects();

    for (auto &contact : contacts)
    {
        items.emplace_back(Item(contact.id, -1,
            ItemType::Contact,
            contact.name, contact.number, "", -1, 
            contact.state != Proto::MemberState::Offline ? onlineImg : offlineImg, false));
    }
}

void ContactDialog::MakeCurrentContactListItems()
{
    std::lock_guard<std::recursive_mutex> lock(storage.GetGroupsMutex());

    for (const auto &group : storage.GetGroups())
    {
        bool rolled = storage.IsGroupRolled(group.id);

        if (group.parent_id == 0 || !rolled || !storage.IsGroupRolled(group.parent_id))
        {
            items.emplace_back(Item(group.id, group.parent_id, ItemType::Group, group.name, "", "", group.level, rolled ? groupRolledImg : groupExpandedImg, false));
        }

        if (!rolled)
        {
            std::lock_guard<std::recursive_mutex> lock(storage.GetContactsMutex());

            for (const auto &contact : storage.GetContacts())
            {
                if (std::find(contact.groups.begin(), contact.groups.end(), group.id) != contact.groups.end())
                {
                    std::shared_ptr<wui::image> img = accountDeletedImg;
                    switch (contact.state)
                    {
                    case Proto::MemberState::Offline:
                        img = offlineImg;
                        break;
                    case Proto::MemberState::Online: case Proto::MemberState::Conferencing:
                        img = onlineImg;
                        break;
                    }

                    items.emplace_back(Item(contact.id, group.id,
                        ItemType::Contact,
                        contact.name, contact.number, "", group.level, img, false));
                }
            }
        }
    }
}

void ContactDialog::DrawItem(wui::graphic &gr, int32_t nItem, const wui::rect &itemRect, wui::list::item_state state)
{
    std::lock_guard<std::mutex> lock(itemsMutex);
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

    int32_t padd = item->type == ItemType::Group ? item->level * XBITMAP : item->level * XBITMAP + XBITMAP;

    auto textRect = itemRect;
    textRect.move(padd + border_width + XBITMAP + 6, (itemRect.height() - gr.measure_text("Qq", font).height()) / 2);
    textRect.right -= padd + border_width + XBITMAP + 6;

    auto name = item->name;
    truncate_line(name, gr, font, textRect.width(), 4);

    gr.draw_text(textRect, name, textColor, font);

    item->image->set_position({ itemRect.left + padd,
        itemRect.top,
        itemRect.left + XBITMAP + padd,
        itemRect.top + XBITMAP });
    item->image->draw(gr, { 0 });

    if (mode != ContactDialogMode::ForwardMessage)
    {
        std::shared_ptr<wui::image> checkImage = item->checked ? checkOnImage : checkOffImage;
        checkImage->set_position({ itemRect.right - checkImage->width() - 20,
            itemRect.top + 4,
            itemRect.right - 20,
            itemRect.top + 4 + checkImage->height() });
        checkImage->draw(gr, { 0 });
    }
}

void ContactDialog::CheckChildren(int64_t parentID, bool checked)
{
    for (auto &i : items)
    {
        if (i.parent_id == parentID && i.id != parentID)
        {
            i.checked = checked;

            if (i.type == ItemType::Group)
            {
                CheckChildren(i.id, checked);
            }
        }
    }
}

void ContactDialog::ClickItem(int32_t nItem, int32_t x, int32_t y)
{
    auto item = GetItem(nItem);
    if (item)
    {
        auto listPos = list->position();
        if (x > listPos.right - checkOnImage->width() - 20 && x <= listPos.right - 20)
        {
            std::lock_guard<std::mutex> lock(itemsMutex);
            items[nItem].checked = !items[nItem].checked;
            if (item->type == ItemType::Group)
            {
                CheckChildren(item->id, item->checked);
            }
            list->set_item_count(static_cast<int32_t>(items.size()));
        }
        else
        {
            static int64_t prevItemId = item->id;
            if (item->id == prevItemId && item->type == ItemType::Group)
            {
                storage.ChangeGroupRolled(item->id);

                std::lock_guard<std::mutex> lock(itemsMutex);
                items.clear();
                MakeCurrentContactListItems();
                list->set_item_count(static_cast<int32_t>(items.size()));
            }
            prevItemId = item->id;
        }
    }
}

const ContactDialog::Item *ContactDialog::GetItem(int32_t nItem)
{
    if (nItem != -1 && nItem < static_cast<int32_t>(items.size()))
    {
        return &items[nItem];
    }

    return nullptr;
}

}
