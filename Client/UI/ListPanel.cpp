/**
 * ListPanel.cpp - Contains the list panel impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018, 2022
 */

#include <UI/ListPanel.h>

#include <wui/locale/locale.hpp>
#include <wui/common/flag_helpers.hpp>
#include <wui/config/config.hpp>

#include <resource.h>

namespace Client
{

ListPanel::ListPanel(std::weak_ptr<wui::window> mainFrame_, ContactList &contactList_, MemberList &memberList_, ListPanelCallback &callback_)
    : mainFrame(mainFrame_),
    contactList(contactList_), memberList(memberList_), callback(callback_),
    panelMode(PanelMode::ContactList),
    pinned(true),
    window(std::make_shared<wui::window>()),
    contactsSheet(std::make_shared<wui::button>(wui::locale("list_panel", "contacts"), [this]() { ShowContacts(); }, wui::button_view::sheet)),
    membersSheet(std::make_shared<wui::button>(wui::locale("list_panel", "members"), [this]() { ShowMembers(); }, wui::button_view::sheet)),
    splitter(std::make_shared<wui::splitter>(wui::splitter_orientation::vertical, std::bind(&ListPanel::SplitterCallback, this, std::placeholders::_1, std::placeholders::_2)))
{
    window->subscribe(std::bind(&ListPanel::ReceiveEvents, this, std::placeholders::_1), wui::event_type::internal);

    window->set_min_size(240, 240);

    contactsSheet->turn(true);
    membersSheet->disable();
}

ListPanel::~ListPanel()
{
    splitter.reset();
}

/// Interface

void ListPanel::Run()
{
    window->add_control(contactsSheet, { 10, 2, 110, 28 });
    window->add_control(membersSheet, { 120, 2, 220, 28 });

    if (panelMode == PanelMode::ContactList)
    {
        contactList.Run(window);
    }
    else if (panelMode == PanelMode::MemberList)
    {
        memberList.Run(window);
    }
  
    if (pinned)
    {
        auto mainFrame_ = mainFrame.lock();
        if (mainFrame_)
        {
            mainFrame_->add_control(window, { 0 });
            mainFrame_->move_to_back(window);

            mainFrame_->add_control(splitter, { 0 });
            splitter->show();

            UpdateHeight(mainFrame_->position().height());
        }
    }

    contactList.UpdateParent();

    window->init("",
        { 0 },
        wui::window_style::pinned,
        [this]() { 
            if (!window->context().valid() || !pinned)
            {
                callback.ListPanelClosed();
                if (splitter) splitter->hide();
            }
        });

    window->set_control_callback([this](wui::window_control control, std::string &tooltipText, bool &) {
        if (control == wui::window_control::pin)
        {
            if (pinned)
            {
                tooltipText = wui::locale("window", "pin");
                Unpin();
            }
            else
            {
                tooltipText = wui::locale("window", "unpin");
                Pin();
            }
            callback.ListPanelPinChanged();
        }
    });
}

void ListPanel::End()
{
    window->destroy();
}

bool ListPanel::IsShowed() const
{
    return pinned && window->context().valid();
}

void ListPanel::UpdateTop(int32_t top)
{
    if (pinned)
    {
        window->set_position({ 0, top, window->position().right, window->position().bottom });
    }
}

void ListPanel::UpdateWidth(int32_t width)
{
    if (pinned)
    {
        wui::config::set_int("ListPanel", "Width", width);
        window->set_position({ 0, window->position().top, width, window->position().bottom });
    }
}

void ListPanel::UpdateHeight(int32_t height)
{
	if (pinned && window->position().bottom != height)
	{
        auto width = wui::config::get_int("ListPanel", "Width", 300);
        window->set_position({ 0, window->position().top, width, height });
	}
}

void ListPanel::Pin()
{
    if (pinned)
    {
        return;
    }

    pinned = true;

    auto parentWindow_ = mainFrame.lock();
    if (parentWindow_)
    {
        parentWindow_->add_control(window, { 0 });
        parentWindow_->move_to_back(window);

        splitter->show();
        
        UpdateHeight(parentWindow_->position().height());
    }

    contactList.UpdateParent();
}

void ListPanel::Unpin()
{
    if (!pinned)
    {
        return;
    }

    pinned = false;

    auto parentWindow_ = mainFrame.lock();
    if (parentWindow_)
    {
        parentWindow_->remove_control(window);
        splitter->hide();
    }

    Run();
}

bool ListPanel::Pinned() const
{
    return pinned;
}

void ListPanel::ShowContacts()
{
    if (panelMode == PanelMode::ContactList)
    {
        return;
    }

    panelMode = PanelMode::ContactList;

    contactsSheet->turn(true);
    membersSheet->turn(false);

    if (!window->context().valid())
    {
        return;
    }

    memberList.End();
    contactList.Run(window);

    auto controlsRight = pinned ? 8 : 0;
    auto windowPos = window->position();
    contactList.UpdateSize(windowPos.width() + controlsRight, windowPos.height());
}

void ListPanel::ShowMembers()
{
    if (panelMode == PanelMode::MemberList)
    {
        return;
    }

    panelMode = PanelMode::MemberList;

    contactsSheet->turn(false);
    membersSheet->turn(true);

    if (!window->context().valid())
    {
        return;
    }

    memberList.Run(window);
    contactList.End();

    auto controlsRight = pinned ? 8 : 0;
    auto windowPos = window->position();
    memberList.UpdateSize(windowPos.width() + controlsRight, windowPos.height());
}

void ListPanel::ActivateMembers()
{
    if (pinned && window->context().valid())
    {
        auto parentWindow_ = mainFrame.lock();
        if (parentWindow_)
        {
            parentWindow_->move_to_back(window);
        }
    }

    ShowMembers();
    membersSheet->enable();
}

void ListPanel::DeactivateMembers()
{
    ShowContacts();
    membersSheet->disable();
}

int32_t ListPanel::Right() const
{
    if (pinned)
    {
        return window->context().valid() && window->position().width() != 0 ? window->position().width() + 5 : 0;
    }
    return 0;
}

ListPanel::PanelMode ListPanel::GetPanelMode() const
{
    return panelMode;
}

void ListPanel::ReceiveEvents(const wui::event &ev)
{
    switch (ev.type)
    {
        case wui::event_type::internal:
        {
            switch (ev.internal_event_.type)
            {
                case wui::internal_event_type::size_changed:
                {
                    auto windowPos = window->position();
                    splitter->set_position({ ev.internal_event_.x, windowPos.top, ev.internal_event_.x + 5, windowPos.bottom }, false);
                    auto mainFrame_ = mainFrame.lock();
                    if (mainFrame_)
                    {
                        splitter->set_margins(290, mainFrame_->position().width() - (wui::config::get_int("RenderersBox", "Showed", 0) != 0 ? 200 : 0) - wui::config::get_int("ContentPanel", "Width", 290));
                    }

                    auto controlsRight = pinned ? 8 : 0;

                    contactList.UpdateSize(ev.internal_event_.x + controlsRight, ev.internal_event_.y);
                    memberList.UpdateSize(ev.internal_event_.x + controlsRight, ev.internal_event_.y);
                }
                break;
            }
        }
        break;
    }
}

void ListPanel::SplitterCallback(int32_t x, int32_t)
{
    UpdateWidth(x);
    callback.ListPanelWidthChanged(x);
}

}
