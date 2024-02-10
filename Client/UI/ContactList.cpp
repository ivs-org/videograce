/**
 * ContactList.cpp - Contains contact list impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2015
 */

#include <wui/window/window.hpp>
#include <wui/theme/theme.hpp>
#include <wui/locale/locale.hpp>
#include <wui/system/tools.hpp>
#include <wui/system/clipboard_tools.hpp>
#include <wui/config/config.hpp>

#include <UI/ContactList.h>

#include <Common/BitHelpers.h>
#include <Proto/MemberGrants.h>
#include <Proto/GroupGrants.h>
#include <Proto/ConferenceGrants.h>

#include <License/Grants.h>

#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string.hpp>

#include <webrtc/base/stringutils.h>

#include <resource.h>

namespace Client
{

ContactList::ContactList(Storage::Storage &storage_, Controller::IController &controller_, ConferenceDialog &conferenceDialog_, IContactListCallback &callback_)
    : storage(storage_),
	controller(controller_),
    conferenceDialog(conferenceDialog_),
    callback(callback_),
    parentWindow_(),
    searchInput(std::make_shared<wui::input>()),
    list(std::make_shared<wui::list>()),
    conferencesRolledImg(std::make_shared<wui::image>(IMG_CL_CONFERENCES_ROLLED)), conferencesExpandedImg(std::make_shared<wui::image>(IMG_CL_CONFERENCES_EXPANDED)),
    symConfImg(std::make_shared<wui::image>(IMG_CL_SYM_CONF)), asymConfImg(std::make_shared<wui::image>(IMG_CL_ASYM_CONF)), asymSSConfImg(std::make_shared<wui::image>(IMG_CL_ASYM_SS_CONF)),
    symConfMyImg(std::make_shared<wui::image>(IMG_CL_SYM_CONF_MY)), asymConfMyImg(std::make_shared<wui::image>(IMG_CL_ASYM_CONF_MY)), asymSSConfMyImg(std::make_shared<wui::image>(IMG_CL_ASYM_SS_CONF_MY)),
    groupRolledImg(std::make_shared<wui::image>(IMG_CL_GROUP_ROLLED)), groupExpandedImg(std::make_shared<wui::image>(IMG_CL_GROUP_EXPANDED)),
    accountDeletedImg(std::make_shared<wui::image>(IMG_CL_ACCOUNT_DELETED)), accountOfflineImg(std::make_shared<wui::image>(IMG_CL_ACCOUNT_OFFLINE)), accountOnlineImg(std::make_shared<wui::image>(IMG_CL_ACCOUNT_ONLINE)),
    ownerImg(std::make_shared<wui::image>(IMG_ML_OWNER)), moderatorImg(std::make_shared<wui::image>(IMG_ML_MODERATOR)), ordinaryImg(std::make_shared<wui::image>(IMG_ML_ORDINARY)), readOnlyImg(std::make_shared<wui::image>(IMG_ML_READONLY)),
    popupMenu(std::make_shared<wui::menu>()),
    addButton(std::make_shared<wui::button>(wui::locale("button", "add"), std::bind(&ContactList::Add, this), wui::button_view::image, IMG_TB_CL_ADD, 32, wui::button::tc_tool)),
    deleteButton(std::make_shared<wui::button>(wui::locale("button", "del"), std::bind(&ContactList::Del, this), wui::button_view::image, IMG_TB_CL_DEL, 24, wui::button::tc_tool)),
    editButton(std::make_shared<wui::button>(wui::locale("button", "edit"), std::bind(&ContactList::Edit, this), wui::button_view::image, IMG_TB_CL_EDIT, 24, wui::button::tc_tool)),
    separator(std::make_shared<wui::image>(IMG_TB_CL_SEPARATOR)),
    messageBox(),
    contactDialog(storage, controller, std::bind(&ContactList::ContactDialogCallback, this, std::placeholders::_1, std::placeholders::_2)),
    itemsMutex(),
    items()
{
    storage.SubscribeMessagesReceiver([this](Storage::MessageAction, const Storage::Messages&) { UpdateItems(); });
    storage.SubscribeContactsReceiver([this](const Storage::Contacts&) { UpdateItems(); });
    storage.SubscribeGroupsReceiver([this](const Storage::Groups&) { UpdateItems(); });
    storage.SubscribeConferencesReceiver([this](const Storage::Conferences&) { UpdateItems(); });

    searchInput->set_change_callback(std::bind(&ContactList::SearchChange, this, std::placeholders::_1));

    list->set_item_height_callback([](int32_t, int32_t& h) { h = XBITMAP; });
    list->set_draw_callback(std::bind(&ContactList::DrawItem, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    list->set_item_change_callback(std::bind(&ContactList::ChangeItem, this, std::placeholders::_1));
    list->set_item_activate_callback(std::bind(&ContactList::ActivateItem, this, std::placeholders::_1));
    list->set_item_click_callback([this](wui::list::click_button btn, int32_t nItem, int32_t x, int32_t y) { if (btn == wui::list::click_button::left) ClickItem(nItem); else RightClickItem(nItem, x, y); });
}

ContactList::~ContactList()
{
}

void ContactList::Run(std::weak_ptr<wui::window> parentWindow__)
{
    parentWindow_ = parentWindow__;

    auto parentWindow = parentWindow_.lock();

    if (messageBox)
    {
        messageBox.reset();
    }

    messageBox = std::make_shared<wui::message>(parentWindow->parent().lock() ? parentWindow->parent().lock() : parentWindow);

    parentWindow->add_control(searchInput,  { 0 });
    parentWindow->add_control(list,         { 0 });
    parentWindow->add_control(popupMenu,    { 0 });
    parentWindow->add_control(addButton,    { 0 });
    parentWindow->add_control(deleteButton, { 0 });
    parentWindow->add_control(separator,    { 0 });
    parentWindow->add_control(editButton,   { 0 });
    
    searchInput->update_theme();
    list->update_theme();
    addButton->update_theme();
    editButton->update_theme();
    deleteButton->update_theme();
    separator->update_theme();

    UpdateItems();
}

void ContactList::End()
{
    auto parentWindow = parentWindow_.lock();
    if (parentWindow)
    {
        parentWindow->remove_control(searchInput);
        parentWindow->remove_control(list);
        parentWindow->remove_control(popupMenu);
        parentWindow->remove_control(addButton);
        parentWindow->remove_control(deleteButton);
        parentWindow->remove_control(editButton);
    }
}

void ContactList::UpdateSize(int32_t width, int32_t height)
{
    searchInput->set_position( { 10,               45, width - 10,                    70 },               false);
    list->set_position(        { 10,               80, width - 10,                    height - 10 - 35 }, false);
    addButton->set_position(   { 10,               height - 5 - 36, 10 + 36,          height - 5 },       false);
    deleteButton->set_position({ 20 + 36,          height - 5 - 28, 20 + 36 + 28,     height - 5 },       false);
    separator->set_position(   { 20 + 36 + 28 + 2, height - 5 - 28, 20 + 36 + 28 + 8, height - 5 },       false);
    editButton->set_position(  { 30 + 36 + 28,     height - 5 - 28, 30 + 36 + 28 * 2, height - 5 },       false);
}

ContactList::Selection ContactList::GetSelection()
{
    Selection selection = { SelectionType::Undefined };

    auto nItem = list->selected_item();

    if (nItem == -1)
    {
        return selection;
    }

    auto *item = GetItem(nItem);
    if (item != nullptr)
    {
        switch (item->type)
        {
            case ItemType::Conference:
                selection.type = SelectionType::Conference;
                selection.id = item->tag;
                selection.my = item->my;
            break;
            case ItemType::GroupUser: case ItemType::Contact: case ItemType::ConferenceUser:
                selection.type = SelectionType::Client;
                selection.id = item->name;
            break;
        }
    }

    return selection;
}

void ContactList::SelectUser(int64_t id, std::string_view name)
{
    int32_t pos = 0;
    {
        std::lock_guard<std::mutex> lock(itemsMutex);
        for (auto &c : items)
        {
            if ((c.type == ItemType::Contact ||
                c.type == ItemType::GroupUser ||
                c.type == ItemType::ConferenceUser) && c.id == id)
            {
                return list->select_item(pos);
            }
            ++pos;
        }
    }

    wui::config::set_string("ContactList", "Selected", "null");
    callback.ContactSelected(id, name);
}

void ContactList::SelectConference(std::string_view tag, std::string_view name)
{
    int32_t pos = 0;
    {
        std::lock_guard<std::mutex> lock(itemsMutex);
        for (auto &c : items)
        {
            if (c.type == ItemType::Conference && c.tag == tag)
            {
                list->select_item(pos);
                return;
            }
            ++pos;
        }
    }

    // Conference not in list
    wui::config::set_string("ContactList", "Selected", "null");
    callback.ConferenceSelected(tag, name);
}

/// Impl

void ContactList::ExtractFromConferenceMember(std::string_view str, std::string &tag, int64_t &id)
{
    tag = str.substr(strlen(CONFERENCE_MEMBER));
    auto comma_index = tag.find(",");
    if (comma_index != std::string::npos)
    {
        tag.erase(comma_index);

        std::string id_s(str.substr(strlen(CONFERENCE_MEMBER)));
        id_s.erase(0, comma_index + 1);

        try {
            id = std::stoull(id_s);            
        }
        catch (std::exception &) {}
    }
}

void ContactList::UpdateItems()
{
    std::lock_guard<std::mutex> lock(itemsMutex);

    items.clear();

    MakeGroups();
    MakeConferences();

    list->set_item_count(static_cast<int32_t>(items.size()));

    auto selected = wui::config::get_string("ContactList", "Selected", "null");

    auto isConferenceMember = selected.find(CONFERENCE_MEMBER) != std::string::npos;
    auto isConference = selected.find(CONFERENCE) != std::string::npos;
    auto isUser = selected.find(CONTACT) != std::string::npos;
    auto isConferencesGroup = selected.find(CONFERENCES_GROUP) != std::string::npos;
    auto isGroup = selected.find(GROUP) != std::string::npos;
    
    if (isConference)
    {
        auto conferenceTag = selected.substr(strlen(CONFERENCE));
        if (!conferenceTag.empty())
        {
            int32_t i = 0;
            for (auto &item : items)
            {
                if (item.type == ItemType::Conference && item.tag == conferenceTag)
                {
                    list->select_item(i);
                    break;
                }
                ++i;
            }
        }
    }
    else if (isUser)
    {
        try {
            auto contactId = std::stoull(selected.substr(strlen(CONTACT)));
            if (contactId != 0)
            {
                int32_t i = 0;
                for (auto &item : items)
                {
                    if ((item.type == ItemType::Contact || item.type == ItemType::GroupUser) && item.id == contactId)
                    {
                        list->select_item(i);
                        break;
                    }
                    ++i;
                }
            }
        }
        catch (std::exception &) {}
    }
    else if (isGroup)
    {
        try {
            auto groupId = std::stoull(selected.substr(strlen(GROUP)));
            if (groupId != 0)
            {
                int32_t i = 0;
                for (auto &item : items)
                {
                    if ((item.type == ItemType::Group) && item.id == groupId)
                    {
                        list->select_item(i);
                        break;
                    }
                    ++i;
                }
            }
        }
        catch (std::exception &) {}
    }
    else if (isConferencesGroup)
    {
        int32_t i = 0;
        for (auto &item : items)
        {
            if (item.type == ItemType::ConferencesGroup)
            {
                list->select_item(i);
                break;
            }
            ++i;
        }
    }
    else if (isConferenceMember)
    {
        std::string tag;
        int64_t contactId = 0;
        ExtractFromConferenceMember(selected, tag, contactId);
        if (contactId != 0)
        {
            int32_t i = 0;
            for (auto &item : items)
            {
                if (item.type == ItemType::ConferenceUser && item.id == contactId)
                {
                    list->select_item(i);
                    break;
                }
                ++i;
            }
        }
    }
}

void ContactList::MakeGroups()
{
    std::lock_guard<std::recursive_mutex> lock(storage.GetGroupsMutex());

    for (const auto &group : storage.GetGroups())
    {
        bool rolled = storage.IsGroupRolled(group.id);
        bool my_group = BitIsSet(group.grants, static_cast<uint32_t>(Proto::GroupGrants::Private));

        if (group.parent_id == 0 || !rolled || !storage.IsGroupRolled(group.parent_id))
        {
            items.emplace_back(Item(group.id,
                group.id,
                ItemType::Group,
                group.name,
                "",
                "",
                group.level,
                rolled ? groupRolledImg : groupExpandedImg,
                0,
                my_group, 
                false));
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
                            img = accountOfflineImg;
                        break;
                        case Proto::MemberState::Online: case Proto::MemberState::Conferencing:
                            img = accountOnlineImg;
                        break;
                    }

                    items.emplace_back(Item(contact.id,
                        group.id,
                        my_group ? ItemType::Contact : ItemType::GroupUser,
                        contact.name,
                        contact.number,
                        "",
                        group.level,
                        img,
                        contact.unreaded_count,
                        my_group,
                        false));
                }
            }
        }
    }
}

void ContactList::MakeConferences()
{
    auto selected = wui::config::get_string("ContactList", "Selected", "null");

    bool rolled = wui::config::get_int("User", "ConferencesGroupRolled", 0) != 0 
        && selected.find(CONFERENCE_MEMBER) == std::string::npos;

    items.emplace_back(Item(-1,
        -1,
        ItemType::ConferencesGroup,
        wui::locale("common", "conferences"), "", "", -1,
        rolled ? conferencesRolledImg : conferencesExpandedImg, 0,
        true,
        false));

    if (rolled)
    {
        return;
    }

    std::string selectedConferenceTag;
    int64_t memberId = 0;
    
    if (selected.find(CONFERENCE_MEMBER) != std::string::npos)
    {
        ExtractFromConferenceMember(selected, selectedConferenceTag, memberId);
    }
    else if (selected.find(CONFERENCE) != std::string::npos)
    {
        selectedConferenceTag = selected.substr(strlen(CONFERENCE));
    }

    std::lock_guard<std::recursive_mutex> lock(storage.GetConferencesMutex());

    for (const auto &conference : storage.GetConferences())
    {
        auto my_conf = conference.founder_id == controller.GetMyClientId();

        std::shared_ptr<wui::image> img = my_conf ? symConfMyImg : symConfImg;
        switch (conference.type)
        {
            case Proto::ConferenceType::Symmetric:
                img = my_conf ? symConfMyImg : symConfImg;
            break;
            case Proto::ConferenceType::Asymmetric:
                img = my_conf ? asymConfMyImg : asymConfImg;
            break;
            case Proto::ConferenceType::AsymmetricWithSymmetricSound:
                img = my_conf ? asymSSConfMyImg : asymSSConfImg;
            break;
        }

        items.emplace_back(Item(conference.id,
            -1,
            ItemType::Conference,
            conference.name,
            "",
            conference.tag,
            0,
            img,
            conference.unreaded_count,
            my_conf,
            BitIsSet(conference.grants, static_cast<int32_t>(Proto::ConferenceGrants::Deactivated))));
        if (conference.tag == selectedConferenceTag &&
            wui::config::get_int("User", "ConferenceUsersRolled", 0) != 0)
        {
            std::lock_guard<std::recursive_mutex> lock(storage.GetContactsMutex());

            for (const auto &member : conference.members)
            {
                std::shared_ptr<wui::image> img = ordinaryImg;

                if (BitIsSet(member.grants, static_cast<int32_t>(Proto::MemberGrants::ReadOnly)))
                {
                    img = readOnlyImg;
                }
                else if (BitIsSet(member.grants, static_cast<int32_t>(Proto::MemberGrants::Moderator)))
                {
                    img = moderatorImg;
                }
                else if (BitIsSet(member.grants, static_cast<int32_t>(Proto::MemberGrants::Presenter)))
                {
                    img = ownerImg;
                }

                items.emplace_back(Item(member.id,
                    conference.id,
                    ItemType::ConferenceUser,
                    member.name,
                    member.number,
                    conference.tag,
                    1,
                    img,
                    0,
                    my_conf, 
                    false));
            }
        }
    }
}

const ContactList::Item* ContactList::GetItem(int32_t itemNumber)
{
    if (itemNumber != -1 && itemNumber < static_cast<int32_t>(items.size()))
    {
        return &items[itemNumber];
    }

    return nullptr;
}

void ContactList::ContactDialogCallback(ContactDialogMode mode, const Storage::Contacts &contacts)
{
    for (auto &c : contacts)
    {
        controller.AddContact(c.id);
    }
}

void ContactList::Add()
{
    auto item = GetItem(list->selected_item());
    if (!item || (item->type == ItemType::Group && !item->my) || item->type == ItemType::GroupUser)
    {
        return messageBox->show(wui::locale("message", "need_select_conference_or_contact"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok);
    }

    parentWindow_.lock()->set_focused(searchInput);

    if (item->type == ItemType::Conference || item->type == ItemType::ConferencesGroup || item->type == ItemType::ConferenceUser)
    {
        AddConference();
    }
    else
    {
        AddContact();
    }
}

void ContactList::Edit()
{
    auto item = GetItem(list->selected_item());
    if (!item || item->type == ItemType::ConferencesGroup || item->type == ItemType::Group || item->type == ItemType::GroupUser || item->type == ItemType::Contact)
    {
        return messageBox->show(wui::locale("message", "need_select_conference_to_change"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok);
    }

    if (item->type == ItemType::Conference || item->type == ItemType::ConferenceUser)
    {
        if (!License::Parse(controller.GetGrants()).allowedCreatingConferences)
        {
            return messageBox->show(wui::locale("message", "conference_editing_denied"),
                wui::locale("message", "title_error"),
                wui::message_icon::alert,
                wui::message_button::ok);
        }

        if (!item->my)
        {
            return messageBox->show(wui::locale("message", "cant_update_not_own_conference"),
                wui::locale("message", "title_error"),
                wui::message_icon::alert,
                wui::message_button::ok);
        }

        auto id = item->type == ItemType::Conference ? item->id : item->group_id;
        std::lock_guard<std::recursive_mutex> lock(storage.GetConferencesMutex());
        for (auto &c : storage.GetConferences())
        {
            if (c.id == id)
            {
                conferenceDialog.Run(c, [&](std::string_view tag) {
                    callback.ConferenceConnect(tag);
                });
                break;
            }
        }
    }
}

void ContactList::Del()
{
    auto item = GetItem(list->selected_item());
    if (!item || item->type == ItemType::ConferencesGroup || item->type == ItemType::Group || item->type == ItemType::GroupUser)
    {
        return messageBox->show(wui::locale("message", "need_select_conference_or_contact"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok);
    }

    if (item->type == ItemType::Conference || item->type == ItemType::ConferenceUser)
    {
        auto id = item->type == ItemType::Conference ? item->id : item->group_id;

        if (!item->my)
        {
            messageBox->show(wui::locale("message", "conference_leaving_confirm"),
                wui::locale("message", "title_confirmation"),
                wui::message_icon::question,
                wui::message_button::yes_no, [this, id](wui::message_result result) {
                if (result == wui::message_result::yes)
                {
                    controller.DeleteMeFromConference(id);
                }
            });
        }
        else
        {
            messageBox->show(wui::locale("message", "conference_deletion_confirm"),
                wui::locale("message", "title_confirmation"),
                wui::message_icon::question,
                wui::message_button::yes_no, [this, id](wui::message_result result) {
                if (result == wui::message_result::yes)
                {
                    controller.DeleteConference(id);
                }
            });
        }
    }
    else if (item->type == ItemType::Contact)
    {
        DeleteContact(item->id);
    }
}

void ContactList::AddContact()
{
    auto parentWindow = parentWindow_.lock();
    contactDialog.Run(parentWindow->parent().lock() ? parentWindow->parent().lock() : parentWindow, ContactDialogMode::AddContacts);
}

void ContactList::DeleteContact(int64_t id)
{
    messageBox->show(wui::locale("message", "contact_deletion_confirm"),
        wui::locale("message", "title_confirmation"),
        wui::message_icon::question,
        wui::message_button::yes_no, [this, id](wui::message_result result) {
        if (result == wui::message_result::yes)
        {
            controller.DeleteContact(id);
        }
    });
}

void ContactList::AddConference()
{
    if (!License::Parse(controller.GetGrants()).allowedCreatingConferences)
    {
        return messageBox->show(wui::locale("message", "conference_creating_denied"),
            wui::locale("message", "title_error"),
            wui::message_icon::alert,
            wui::message_button::ok);
    }

    conferenceDialog.Run(Proto::Conference(), [this](std::string_view tag) {
        callback.ConferenceConnect(tag);
    });
}

void ContactList::CopyConferenceLink(std::string_view tag)
{
    auto serverAddress = controller.GetServerAddress();

    std::vector<std::string> vals;
    boost::split(vals, serverAddress, boost::is_any_of(":"));
    if (vals.size() == 2 && (vals[1] == "5060" || vals[1] == "80" || vals[1] == "443"))
    {
        serverAddress = vals[0];
    }

    wui::clipboard_put((controller.IsSecureConnection() ? "https://" : "http://") +
        serverAddress + "/conferences/" + std::string(tag), parentWindow_.lock()->context());

    messageBox->show(wui::locale("message", "conference_link_copied"),
        wui::locale("message", "title_notification"),
        wui::message_icon::information,
        wui::message_button::ok);
}

void ContactList::SearchChange(std::string_view text)
{
    int pos = 0;
    bool finded = false;
    bool searchNumber = text.find_first_not_of("0123456789") == std::string::npos;;

    {
        std::lock_guard<std::mutex> lock(itemsMutex);
        for (const auto &item : items)
        {
            std::string itemStr = !searchNumber ? item.name : item.number;

            finded = boost::ifind_first(itemStr, text);
            if (finded)
            {
                break;
            }
            ++pos;
        }
    }

    if (finded)
    {
        list->select_item(pos);
    }
}

void ContactList::DrawItem(wui::graphic &gr, int32_t nItem, const wui::rect &itemRect, wui::list::item_state state)
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

    int32_t padd = item->type == ItemType::Group ? item->level * XBITMAP : item->level * XBITMAP + XBITMAP;

    auto textColor = wui::theme_color("contact_list", "text");
    auto textFont = wui::theme_font(wui::list::tc, wui::list::tv_font);

    auto nameRect = itemRect;
    nameRect.move(padd + border_width + XBITMAP + 6, (itemRect.height() - gr.measure_text("Qq", textFont).height()) / 2);
    nameRect.right -= padd + border_width + XBITMAP + 6;

    auto name = item->name;
    truncate_line(name, gr, textFont, nameRect.width(), 4);

    gr.draw_text(nameRect, name, textColor, textFont);

    if (storage.GetShowNumbers())
    {
        auto numberRect = nameRect;
        numberRect.left -= (XBITMAP * 2);
        numberRect.right = padd + border_width + 6;

        auto number = item->number;
        truncate_line(number, gr, textFont, numberRect.width(), 4);

        gr.draw_text(numberRect, number, textColor, textFont);
    }

    if (item->unreaded_count != 0)
    {
        auto unreadedBackground = wui::theme_color("contact_list", "unreaded_background");
        auto unreadedColor = wui::theme_color("contact_list", "unreaded_text");
        auto unreadedFont = wui::theme_font("contact_list", "unreaded_font");

        auto unreadedText = std::to_string(item->unreaded_count);

        auto unreadedTextRect = gr.measure_text(unreadedText, unreadedFont);

        auto top = itemRect.top + (itemRect.height() - unreadedTextRect.height()) / 2;
        auto width = unreadedTextRect.width() > unreadedTextRect.height() ? unreadedTextRect.width() + 4 : unreadedTextRect.height();

        wui::rect backgroundRect = { itemRect.right - width - 8, top - 1, itemRect.right - 6, top + unreadedTextRect.height() + 2 };
        gr.draw_rect(backgroundRect, unreadedBackground, unreadedBackground, 1, 8);

        wui::rect textRect = { itemRect.right - width - 8 + ((width - unreadedTextRect.width() + 4) / 2), top, itemRect.right - 6, top + unreadedTextRect.height() };
        gr.draw_text(textRect, unreadedText, unreadedColor, unreadedFont);
    }

    item->image->set_position({ itemRect.left + padd,
        itemRect.top,
        itemRect.left + XBITMAP + padd,
        itemRect.top + XBITMAP });
    item->image->draw(gr, { 0 });

    if (item->deacivated)
    {
        int32_t center = itemRect.top + (XBITMAP / 2);
        gr.draw_rect({ itemRect.left + padd + 5,
                center - 2,
                itemRect.left + XBITMAP + padd - 5,
                center + 2 },
            wui::make_color(234, 10, 40),
            wui::make_color(234, 10, 40),
            1,
            4);
    }
}

void ContactList::ClickItem(int32_t nItem)
{
    if (nItem == -1)
    {
        return callback.ContactUnselected();
    }

    auto *item = GetItem(nItem);
    if (item != nullptr)
    {
        static int64_t prevItemId = item->id;
        if (item->id == prevItemId)
        {
            switch (item->type)
            {
                case ItemType::Group:
                    storage.ChangeGroupRolled(item->id);
                    UpdateItems();
                break;
                case ItemType::ConferencesGroup:
                    wui::config::set_int("User", "ConferencesGroupRolled", wui::config::get_int("User", "ConferencesGroupRolled", 0) != 0 ? 0 : 1);
                    UpdateItems();
                break;
                case ItemType::Conference:
                    wui::config::set_int("User", "ConferenceUsersRolled", wui::config::get_int("User", "ConferenceUsersRolled", 0) != 0 ? 0 : 1);
                    UpdateItems();
                break;
            }
        }
        else if (item->type == ItemType::Conference)
        {
            wui::config::set_string("ContactList", "Selected", CONFERENCE + item->tag);
            UpdateItems();
        }
        prevItemId = item->id;
    }
}

void ContactList::ChangeItem(int32_t nItem)
{
    if (nItem == -1)
    {
        return callback.ContactUnselected();
    }

    auto *item = GetItem(nItem);
    if (item != nullptr)
    {
        switch (item->type)
        {
            case ItemType::GroupUser: case ItemType::Contact:
            {
                auto name = rtc::string_trim(item->name);
                if (!name.empty())
                {
                    callback.ContactSelected(item->id, name);
                    wui::config::set_string("ContactList", "Selected", CONTACT + std::to_string(item->id));
                }
            }
            break;
            case ItemType::ConferenceUser:
            {
                auto name = rtc::string_trim(item->name);
                if (!name.empty())
                {
                    callback.ContactSelected(item->id, name);
                    wui::config::set_string("ContactList", "Selected", CONFERENCE_MEMBER + item->tag + "," + std::to_string(item->id));
                }
            }
            break;
            case ItemType::Conference:
                callback.ConferenceSelected(item->tag, item->name);
                wui::config::set_string("ContactList", "Selected", CONFERENCE + item->tag);
            break;
            case ItemType::ConferencesGroup:
                wui::config::set_string("ContactList", "Selected", CONFERENCES_GROUP);
            break;
            case ItemType::Group:
                wui::config::set_string("ContactList", "Selected", GROUP + std::to_string(item->id));
            break;
        }
    }
}

void ContactList::ActivateItem(int32_t nItem)
{
    if (nItem != -1)
    {
        auto *item = GetItem(nItem);

        if (item != nullptr)
        {
            switch (item->type)
            {
                case ItemType::GroupUser: case ItemType::Contact:
                {
                    auto name = rtc::string_trim(item->name);
                    if (!name.empty())
                    {
                        callback.ContactCall(name);
                    }
                }
                break;
                case ItemType::Conference:
                {
                    callback.ConferenceConnect(item->tag);
                }
                break;
                case ItemType::Group:
                    storage.ChangeGroupRolled(item->id);
                    UpdateItems();
                break;
                case ItemType::ConferencesGroup:
                    wui::config::set_int("User", "ConferencesGroupRolled", wui::config::get_int("User", "ConferencesGroupRolled", 0) != 0 ? 0 : 1);
                    UpdateItems();
                break;
            }
        }
    }
}

void ContactList::RightClickItem(int32_t nItem, int32_t x, int32_t y)
{
    if (nItem != -1)
    {
        auto *item = GetItem(nItem);

        if (item != nullptr)
        {
            switch (item->type)
            {
                case ItemType::Contact:
                    if (item->image != accountOfflineImg && item->id != controller.GetMyClientId())
                    {
                        popupMenu->set_items({
                            { 0, wui::menu_item_state::separator, wui::locale("contact_list", controller.GetState() != Controller::State::Conferencing ? "call_user" : "add_user_to_conference"), "", nullptr, {}, [this, item](int32_t) {
                                auto name = rtc::string_trim(item->name);
                                if (!name.empty())
                                {
                                    callback.ContactCall(name);
                                }
                            } },
                            { 1, wui::menu_item_state::normal, wui::locale("contact_list", "add_user"), "", nullptr, {}, [this](int32_t) {
                                AddContact();
                            } },
                            { 2, wui::menu_item_state::normal, wui::locale("contact_list", "remove_user"), "", nullptr, {}, [this, item](int32_t) { DeleteContact(item->id); } }
                        });
                    }
                    else
                    {
                        popupMenu->set_items({
                            { 0, wui::menu_item_state::normal, wui::locale("contact_list", "add_user"), "", nullptr, {}, [this](int32_t) {
                                AddContact();
                            } },
                            { 1, wui::menu_item_state::normal, wui::locale("contact_list", "remove_user"), "", nullptr, {}, [this, item](int32_t) { DeleteContact(item->id); } }
                        });
                    }
                    popupMenu->show_on_point(x, y);
                break;
                case ItemType::GroupUser:
                    if (item->image != accountOfflineImg && item->id != controller.GetMyClientId())
                    {
                        popupMenu->set_items({
                            { 0, wui::menu_item_state::normal, wui::locale("contact_list", controller.GetState() != Controller::State::Conferencing ? "call_user" : "add_user_to_conference"), "", nullptr, {}, [this, item](int32_t) {
                                auto name = rtc::string_trim(item->name);
                                if (!name.empty())
                                {
                                    callback.ContactCall(name);
                                }
                            } }
                            });
                        popupMenu->show_on_point(x, y);
                    }
                break;
                case ItemType::Group:
                    if (item->my)
                    {
                        popupMenu->set_items({
                            { 0, wui::menu_item_state::normal, wui::locale("contact_list", "add_user"), "", nullptr, {}, [this](int32_t) {
                                AddContact();
                            } }
                        });
                        popupMenu->show_on_point(x, y);
                    }
                break;
                case ItemType::Conference:
                    if (item->my)
                    {
                        popupMenu->set_items({
                            { 0, wui::menu_item_state::normal, wui::locale("contact_list", "start_conference"), "", nullptr, {}, [this, item](int32_t) {
                                callback.ConferenceConnect(item->tag);
                            } },
                            { 1, wui::menu_item_state::separator, wui::locale("contact_list", "copy_conference_link"), "", nullptr, {}, [this, item](int32_t) {
                                CopyConferenceLink(item->tag);
                            } },
                            { 2, wui::menu_item_state::normal, wui::locale("contact_list", "create_conference"), "", nullptr, {}, [this](int32_t) {
                                AddConference();
                            } },
                            { 3, wui::menu_item_state::normal, wui::locale("contact_list", "edit_conference"), "", nullptr, {}, [this, item](int32_t) {
                                if (!License::Parse(controller.GetGrants()).allowedCreatingConferences)
                                {
                                    return messageBox->show(wui::locale("message", "conference_editing_denied"),
                                        wui::locale("message", "title_error"),
                                        wui::message_icon::alert,
                                        wui::message_button::ok);
                                }

                                auto id = item->type == ItemType::Conference ? item->id : item->group_id;
                                std::lock_guard<std::recursive_mutex> lock(storage.GetConferencesMutex());
                                for (auto &c : storage.GetConferences())
                                {
                                    if (c.id == id)
                                    {
                                        conferenceDialog.Run(c, [this](std::string_view tag) {
                                            callback.ConferenceConnect(tag);
                                        });
                                        break;
                                    }
                                }
                            } },
                            { 4, wui::menu_item_state::normal, wui::locale("contact_list", "delete_conference"), "", nullptr, {}, [this, item](int32_t) {
                                messageBox->show(wui::locale("message", "conference_deletion_confirm"),
                                    wui::locale("message", "title_confirmation"),
                                    wui::message_icon::question,
                                    wui::message_button::yes_no, [this, item](wui::message_result result) {
                                    if (result == wui::message_result::yes)
                                    {
                                        controller.DeleteConference(item->id);
                                    }
                                });
                            } }
                        });
                    }
                    else
                    {
                        popupMenu->set_items({
                            { 0, wui::menu_item_state::normal, wui::locale("contact_list", "connect_to_conference"), "", nullptr, {}, [this, item](int32_t) {
                                callback.ConferenceConnect(item->tag);
                            } },
                            { 1, wui::menu_item_state::separator, wui::locale("contact_list", "copy_conference_link"), "", nullptr, {}, [this, item](int32_t) {
                                CopyConferenceLink(item->tag);
                            } },
                            { 2, wui::menu_item_state::normal, wui::locale("contact_list", "create_conference"), "", nullptr, {}, [this](int32_t) {
                                AddConference();
                            } },
                            { 3, wui::menu_item_state::normal, wui::locale("contact_list", "leave_conference"), "", nullptr, {}, [this, item](int32_t) {
                                messageBox->show(wui::locale("message", "conference_leaving_confirm"),
                                    wui::locale("message", "title_confirmation"),
                                    wui::message_icon::question,
                                    wui::message_button::yes_no, [this, item](wui::message_result result) {
                                    if (result == wui::message_result::yes)
                                    {
                                        controller.DeleteMeFromConference(item->id);
                                    }
                                });
                            } }
                        });
                    }
                    popupMenu->show_on_point(x, y);
                break;
                case ItemType::ConferencesGroup:
                    popupMenu->set_items({
                        { 0, wui::menu_item_state::normal, wui::locale("contact_list", "create_conference"), "", nullptr, {}, [this](int32_t) {
                            AddConference();
                        } }
                    });
                    popupMenu->show_on_point(x, y);
                break;
            }
        }
    }
}

}
