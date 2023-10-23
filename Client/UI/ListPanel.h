/**
 * ListPanel.h - Contains the list panel header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018, 2022
 */

#pragma once

#include <wui/window/window.hpp>
#include <wui/control/splitter.hpp>

#include <string>
#include <cstdint>

#include <UI/ContactList.h>
#include <UI/MemberList.h>

namespace Client
{

class ListPanelCallback
{
public:
	virtual void ListPanelMembersSelected() = 0;
	virtual void ListPanelClosed() = 0;
	virtual void ListPanelPinChanged() = 0;
    virtual void ListPanelWidthChanged(int32_t width) = 0;

protected:
	~ListPanelCallback() {}
};

class ListPanel
{
public:
    ListPanel(std::weak_ptr<wui::window> mainFrame, ContactList &contactList_, MemberList &memberList_, ListPanelCallback &callback_);
    ~ListPanel();

	/// Interface
    void Run();
    void End();
    bool IsShowed() const;

    void UpdateTop(int32_t top);
    void UpdateWidth(int32_t width);
    void UpdateHeight(int32_t height);

    void Pin();
    void Unpin();
    bool Pinned() const;

    void ShowContacts();
    void ShowMembers();

    void ActivateMembers();
    void DeactivateMembers();

    int32_t Right() const;

    enum class PanelMode
    {
        ContactList,
        MemberList
    };

    PanelMode GetPanelMode() const;

private:
    std::weak_ptr<wui::window> mainFrame;
    ContactList &contactList;
    MemberList &memberList;
    ListPanelCallback &callback;

    std::shared_ptr<wui::window> window;
    std::shared_ptr<wui::button> contactsSheet, membersSheet;

    std::shared_ptr<wui::splitter> splitter;

    PanelMode panelMode;

    bool pinned;

    void ReceiveEvents(const wui::event &ev);

    void SplitterCallback(int32_t x, int32_t);
};

}
