/**
 * ContentPanel.h - Contains the content panel header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#pragma once

#include <wui/window/window.hpp>
#include <wui/control/splitter.hpp>
#include <wui/control/input.hpp>
#include <wui/control/button.hpp>
#include <wui/control/list.hpp>
#include <wui/control/image.hpp>
#include <wui/control/menu.hpp>
#include <wui/control/text.hpp>
#include <wui/control/panel.hpp>

#include <Controller/IController.h>

#include <string>
#include <cstdint>

namespace Client
{

class ContentPanelCallback
{
public:
	virtual void ContentPanelClosed() = 0;
	virtual void ContentPanelPinChanged() = 0;
    virtual void ContentPanelWidthChanged(int32_t width) = 0;

protected:
	~ContentPanelCallback() {}
};

struct FormattedPost
{
    int32_t pos;

    Proto::MessageType type;

    time_t dt;

    std::vector<std::string> replys, lines;
    wui::rect size;
    int32_t lineHeight;

    std::string author, replyAuthor;

    inline bool operator==(uint32_t pos_)
    {
        return pos == pos_;
    }
};

class ContentPanel
{
public:
    ContentPanel(std::weak_ptr<wui::window> mainFrame, Storage::Storage &storage_, Controller::IController &controller_, ContentPanelCallback &callback_);
    ~ContentPanel();

	/// Interface
    void Run();
    void End();
    bool IsShowed() const;

    void SetStandbyMode();
    void SetConferencingMode();

    void UpdateLeft(int32_t left);
    void UpdateTop(int32_t top);
    void UpdateSize(int32_t width, int32_t height);
    void ScrollToEnd();

    void Pin();
    void Unpin();
    bool Pinned() const;

    int32_t Left() const;

    void SetUser(const int64_t id, std::string_view name);
    void SetConference(std::string_view tag, std::string_view name);

private:
    std::weak_ptr<wui::window> mainFrame;
    Storage::Storage &storage;
    Controller::IController &controller;
    ContentPanelCallback &callback;

    std::shared_ptr<wui::window> window;
    
    std::shared_ptr<wui::splitter> splitter;

    std::shared_ptr<wui::panel> titlePanel;
    std::shared_ptr<wui::text> title;
    std::shared_ptr<wui::list> list;
    std::shared_ptr<wui::button> sendButton;
    std::shared_ptr<wui::input> input;
    
    std::shared_ptr<wui::menu> popupMenu;
    
    std::shared_ptr<wui::image> msgCreatedImg, msgSendedImg, msgDeliveredImg, msgReadedImg;
    
    std::shared_ptr<wui::image> replyImg;
    std::shared_ptr<wui::text> replyAuthor, replyText;
    std::shared_ptr<wui::button> replyCloseButton;

    std::unique_ptr<wui::graphic> memGr;

    int32_t statusImageWidth;

    bool pinned, standbyMode;

    int64_t userId;
    std::string userName;
    std::string conferenceTag, conferenceName;

    std::vector<FormattedPost> posts;

    Proto::Message replyMessage;

    static const int MESSAGE_LIMIT = 20;

    void SendMsg();

    void SendReaded();

    void ReceiveEvents(const wui::event &ev);

    void SplitterCallback(int32_t x, int32_t);

    void ListItemHeightCallback(int32_t nItem, int32_t &height);

    void DrawListItem(wui::graphic &gr, int32_t nItem, const wui::rect &pos, wui::list::item_state state);

    void ListScrollCallback(wui::scroll_state, int32_t);

    void MessagesUpdatedCallback(Storage::MessageAction, const Storage::Messages &messages);

    void FormatPost(std::string_view source_, FormattedPost &post);

    void RightClickItem(int32_t nItem, int32_t x, int32_t y);

    void ReplyMessage(const Proto::Message &msg);
    void CopyMessage(const Proto::Message &msg);
    void DeleteMessage(const Proto::Message &msg);

    void CloseReply();
};

}
