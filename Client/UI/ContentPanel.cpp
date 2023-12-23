/**
 * ContentPanel.cpp - Contains the content panel impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <UI/ContentPanel.h>

#include <wui/locale/locale.hpp>
#include <wui/theme/theme.hpp>
#include <wui/system/clipboard_tools.hpp>
#include <wui/event/internal_event.hpp>
#include <wui/common/flag_helpers.hpp>

#include <wui/config/config.hpp>
#include <Common/Process.h>
#include <Common/HumanTime.h>
#include <Common/JSONSymbolsScreener.h>

#include <boost/algorithm/string.hpp>

#include <sstream>
#include <nlohmann/json.hpp>

#include <resource.h>

namespace Client
{

enum class CPEvent : uint32_t
{
    AddMessage = 11000,
    UpdateMessages = 11001,
    SendReaded = 11002
};

ContentPanel::ContentPanel(std::weak_ptr<wui::window> mainFrame_, Storage::Storage &storage_, Controller::IController &controller_, ContentPanelCallback &callback_)
    : mainFrame(mainFrame_),
    storage(storage_),
    controller(controller_),
    callback(callback_),
    window(new wui::window()),
    splitter(new wui::splitter(wui::splitter_orientation::vertical, std::bind(&ContentPanel::SplitterCallback, this, std::placeholders::_1, std::placeholders::_2))),
    titlePanel(new wui::panel("content_title_panel")),
    title(new wui::text("", wui::hori_alignment::left, wui::vert_alignment::top, "content_title_text")),
    list(new wui::list("content_box")),
    sendButton(new wui::button(wui::locale("content_panel", "send_message"), std::bind(&ContentPanel::SendMsg, this), wui::button_view::image, IMG_TB_SEND, 25, wui::button::tc_tool)),
    input(new wui::input()),
    popupMenu(new wui::menu()),
    msgCreatedImg(new wui::image(IMG_MSG_CREATED)), msgSendedImg(new wui::image(IMG_MSG_SENDED)), msgDeliveredImg(new wui::image(IMG_MSG_DELIVERED)), msgReadedImg(new wui::image(IMG_MSG_READED)),
    replyImg(new wui::image(IMG_REPLY)),
    replyAuthor(new wui::text("", wui::hori_alignment::left, wui::vert_alignment::top, "author_text")), replyText(new wui::text()),
    replyCloseButton(new wui::button(wui::locale("content_panel", "close_reply"), std::bind(&ContentPanel::CloseReply, this), wui::button_view::image, wui::theme_image(wui::window::ti_close), 24, wui::button::tc_tool)),
    memGr(),
    statusImageWidth(0),
    pinned(true), standbyMode(true),
    userId(0),
    userName(),
    conferenceTag(), conferenceName(),
    posts(),
    replyMessage()
{
    storage.SubscribeMessagesReceiver(std::bind(&ContentPanel::MessagesUpdatedCallback, this, std::placeholders::_1, std::placeholders::_2));

    window->subscribe(std::bind(&ContentPanel::ReceiveEvents, this, std::placeholders::_1), wui::event_type::internal);

    window->set_min_size(240, 240);

    splitter->hide();

    list->set_item_count(0);
    list->set_item_height_callback(std::bind(&ContentPanel::ListItemHeightCallback, this, std::placeholders::_1, std::placeholders::_2));
    list->set_draw_callback(std::bind(&ContentPanel::DrawListItem, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    list->set_scroll_callback(std::bind(&ContentPanel::ListScrollCallback, this, std::placeholders::_1, std::placeholders::_2));
    list->set_item_click_callback([this](wui::list::click_button btn, int32_t nItem, int32_t x, int32_t y) { if (btn == wui::list::click_button::right) RightClickItem(nItem, x, y); });

    window->set_default_push_control(sendButton);
}

ContentPanel::~ContentPanel()
{
}

/// Interface

void ContentPanel::Run()
{
    window->add_control(titlePanel, { 0 });
    window->add_control(title, { 0 });
    window->add_control(list, { 0 });
    window->add_control(sendButton, { 0 });
    window->add_control(input, { 0 });
    window->add_control(popupMenu, { 0 });
    window->add_control(replyImg, { 0 });
    window->add_control(replyAuthor, { 0 });
    window->add_control(replyText, { 0 });
    window->add_control(replyCloseButton, { 0 });

    replyImg->hide();
    replyCloseButton->hide();

    if (pinned)
    {
        auto mainFrame_ = mainFrame.lock();
        if (mainFrame_)
        {
            mainFrame_->add_control(splitter, { 0 });
            if (standbyMode)
            {
                splitter->hide();
            }
            else
            {
                splitter->show();
            }

            mainFrame_->add_control(window, { 0 });
            mainFrame_->move_to_back(window);

            UpdateSize(mainFrame_->position().width(), mainFrame_->position().height());
        }
    }

    window->init("",
        { 0 },
        pinned ? wui::flags_map<wui::window_style>(2, wui::window_style::pinned, wui::window_style::border_left) : wui::window_style::pinned,
        [this]() { 
            if (!window->context().valid() || !pinned)
            {
                callback.ContentPanelClosed();
                splitter->hide(); 
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
            callback.ContentPanelPinChanged();
        }
    });

    if (!memGr)
    {
#ifdef _WIN32
        memGr = std::unique_ptr<wui::graphic>(new wui::graphic(wui::system_context{ window->context().hwnd }));
#elif __linux__
        memGr = std::unique_ptr<wui::graphic>(new wui::graphic(window->context()));
#endif
        memGr->init(window->position(), 0);
    }

    statusImageWidth = msgReadedImg->width();

    titlePanel->update_theme();
    list->update_theme();
    input->update_theme();
    sendButton->update_theme();

    popupMenu->update_theme();

    msgCreatedImg->update_theme();
    msgSendedImg->update_theme();
    msgDeliveredImg->update_theme();
    msgReadedImg->update_theme();

    replyImg->update_theme();
    replyAuthor->update_theme();
    replyText->update_theme();
    replyCloseButton->update_theme();

    list->scroll_to_end();
}

void ContentPanel::End()
{
    window->destroy();
}

bool ContentPanel::IsShowed() const
{
    return pinned && window->context().valid();
}

void ContentPanel::SetStandbyMode()
{
    standbyMode = true;
    splitter->hide();
}

void ContentPanel::SetConferencingMode()
{
    standbyMode = false;
    if (pinned && window->context().valid())
    {
        auto parentWindow_ = mainFrame.lock();
        if (parentWindow_)
        {
            parentWindow_->add_control(window, { 0 });
            parentWindow_->move_to_back(window);

            splitter->show();

            UpdateSize(parentWindow_->position().width(), parentWindow_->position().height());
        }
    }
}

void ContentPanel::UpdateLeft(int32_t left)
{
    if (pinned)
    {
        auto windowPos = window->position();
        window->set_position({ left, windowPos.top, windowPos.right, windowPos.bottom });
    }
}

void ContentPanel::UpdateTop(int32_t top)
{
    if (pinned)
    {
        auto windowPos = window->position();
        window->set_position({ windowPos.left, top, windowPos.right, windowPos.bottom });
    }
}

void ContentPanel::UpdateSize(int32_t width, int32_t height)
{
    if (pinned)
    {
        auto panelWidth = wui::config::get_int("ContentPanel", "Width", 300);
        window->set_position({ !standbyMode ? width - panelWidth : window->position().left, window->position().top, width, height });
    }
}

void ContentPanel::ScrollToEnd()
{
    list->scroll_to_end();
}

void ContentPanel::Pin()
{
    if (pinned)
    {
        return;
    }

    pinned = true;

    window->set_style(wui::flags_map<wui::window_style>(2, wui::window_style::pinned, wui::window_style::border_left));

    auto parentWindow_ = mainFrame.lock();
    if (parentWindow_)
    {
        parentWindow_->add_control(window, { 0 });
        parentWindow_->move_to_back(window);

        if (standbyMode)
        {
            splitter->hide();
        }
        else
        {
            splitter->show();
        }

        UpdateSize(parentWindow_->position().width(), parentWindow_->position().height());
    }
}

void ContentPanel::Unpin()
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

bool ContentPanel::Pinned() const
{
    return pinned;
}

int32_t ContentPanel::Left() const
{
    if (window->context().valid() && pinned)
    {
        return window->position().left - 5;
    }
    else
    {
        auto parentWindow_ = mainFrame.lock();
        if (parentWindow_)
        {
            return parentWindow_->position().width();
        }
    }

    return 0;
}

void ContentPanel::SetUser(const int64_t id, std::string_view name)
{
    if (id == 0 || userId == id)
    {
        return;
    }

    posts.clear();

    userId = id;
    userName = name;
    conferenceTag.clear();
    conferenceName.clear();

    title->set_text(userName);

    int32_t count = 0;
    {
        std::lock_guard<std::recursive_mutex> lock(storage.MessagesMutex());
        count = static_cast<int32_t>(storage.LoadMessages(0, userId, conferenceTag, MESSAGE_LIMIT));
    }

    list->set_item_count(count);
    list->scroll_to_end();
    
    if (count != 0)
    {
        window->emit_event(static_cast<int32_t>(CPEvent::SendReaded), 0);
    }
}

void ContentPanel::SetConference(std::string_view tag, std::string_view name)
{
    if (tag.empty() || conferenceTag == tag)
    {
        return;
    }

    posts.clear();
    
    userId = 0;
    userName.clear();
    conferenceTag = tag;
    conferenceName = name;

    title->set_text(name);

    int32_t count = 0;
    {
        std::lock_guard<std::recursive_mutex> lock(storage.MessagesMutex());
        count = static_cast<int32_t>(storage.LoadMessages(0, userId, conferenceTag, MESSAGE_LIMIT));
    }

    list->set_item_count(count);
    list->scroll_to_end();

    if (count != 0)
    {
        window->emit_event(static_cast<int32_t>(CPEvent::SendReaded), 0);
    }
}

void ParseMessageJSON(std::string_view input, std::string &text, std::string *replyAuthor = nullptr, std::string *reply = nullptr)
{
    try
    {
        auto j = nlohmann::json::parse(input);

        if (j.is_discarded())
        {
            text = input;
            return;
        }

        auto obj = j.get<nlohmann::json::object_t>();
        auto type = obj.at("type").get<std::string>();
        if (type == "simple")
        {
            text = obj.at("message").get<std::string>();
        }
        else if (type == "reply")
        {
            text = obj.at("message").get<std::string>();
            if (replyAuthor)
            {
                *replyAuthor = obj.at("author_name").get<std::string>();
            }
            if (reply)
            {
                *reply = obj.at("reply_text").get<std::string>();
            }
        }
        else if (type == "system_notice")
        {
            auto eventType = obj.at("event").get<std::string>();
            if (eventType == "added_to_conference")
            {
                text = wui::locale("content_panel", "added_to_conference");
            }
            else if (eventType == "created_conference")
            {
                text = wui::locale("content_panel", "created_conference");
            }
            else if (eventType == "invoice")
            {
                text = wui::locale("content_panel", "invoice");
            }
            else if (eventType == "call_start")
            {
                text = wui::locale("content_panel", "call_start");
            }
            else if (eventType == "call_ended")
            {
                text = wui::locale("content_panel", "call_ended") + " " + Common::toHumanDuration(obj.at("duration").get<uint64_t>());
            }
            else if (eventType == "missed_call")
            {
                text = wui::locale("content_panel", "missed_call");
            }
            else if (eventType == "subscriber_offline")
            {
                text = wui::locale("content_panel", "subscriber_offline");
            }
            else if (eventType == "subscriber_reject_call")
            {
                text = wui::locale("content_panel", "subscriber_reject_call");
            }
            else if (eventType == "subscriber_answer_timeout")
            {
                text = wui::locale("content_panel", "subscriber_answer_timeout");
            }
            else if (eventType == "you_reject_call")
            {
                text = wui::locale("content_panel", "you_reject_call");
            }
            else if (eventType == "subscriber_busy")
            {
                text = wui::locale("content_panel", "subscriber_busy");
            }
        }
    }
    catch (...)
    {
        text = input;
    }
}

void SplitLines(std::string_view source_, std::vector<std::string> &lines, int32_t &maxWidth, int32_t &lineHeight, wui::graphic &gr)
{
    std::stringstream source(source_.data());
    std::string line;
    int32_t maxLine = 0;

    auto font = wui::theme_font("content_box", "message_font");

    auto lineHeightMeasure = gr.measure_text("Q`,", font);
    lineHeight = lineHeightMeasure.height();
#ifndef _WIN32
    lineHeight *= 1.3;
#endif

    while (std::getline(source, line, '\n'))
    {
        auto lineSize = gr.measure_text(line, font);
        
        if (lineSize.width() <= maxWidth)
        {
            lines.emplace_back(line);

            if (lineSize.width() > maxLine)
            {
                maxLine = lineSize.width();
            }
        }
        else
        {
            std::vector<std::string> strs;
            boost::split(strs, line, boost::is_any_of(" "));

            while (!strs.empty())
            {
                std::string newLine;
                lineSize = { 0 };

                while (!strs.empty() && lineSize.width() <= maxWidth)
                {
                    newLine += strs[0] + " ";
                    lineSize = gr.measure_text(newLine, font);

                    strs.erase(strs.begin());
                }

                if (lineSize.width() > maxLine)
                {
                    maxLine = lineSize.width();
                }

                lines.emplace_back(newLine);
            }
        }
    }

    maxWidth = maxLine;
}

void ContentPanel::SendMsg()
{
    if (input->text().empty())
    {
        return;
    }

    std::string text;

    if (replyMessage.guid.empty())
    {
        text = "{"
            "\"type\":\"simple\","
            "\"message\":\"" + Common::JSON::Screen(input->text()) + "\""
            "}";
    }
    else
    {
        std::string replyText;
        ParseMessageJSON(replyMessage.text, replyText);

        text = "{"
            "\"type\":\"reply\","
            "\"guid\":\"" + replyMessage.guid + "\","
            "\"author_id\":" + std::to_string(replyMessage.author_id) + ","
            "\"author_name\":\"" + replyMessage.author_name + "\","
            "\"dt\":" + std::to_string(replyMessage.dt) + ","
            "\"reply_text\":\"" + Common::JSON::Screen(replyText) + "\","
            "\"message\":\"" + Common::JSON::Screen(input->text()) + "\""
        "}";

        CloseReply();
    }

    auto message = Proto::Message(Storage::GUID(),
        time(0),
        Proto::MessageType::TextMessage,
        controller.GetMyClientId(), controller.GetMyClientName(),
        controller.GetMyClientId(), controller.GetMyClientName(),
        userId, userName,
        conferenceTag, conferenceName,
        Proto::MessageStatus::Created,
        text,
        0, Proto::CallResult::Undefined,
        "", "", "");

    storage.AddMessage(message);

    controller.DeliveryMessages(Storage::Messages{ message });

    input->set_text("");
}

void ContentPanel::SendReaded()
{
    if (Common::IsForegroundProcess())
    {
        Storage::Messages notices;
        Storage::Messages conferenceReadeds;
        {
            std::lock_guard<std::recursive_mutex> lock(storage.MessagesMutex());
            for (auto &msg : storage.GetMessages())
            {
                if (((msg.sender_id != controller.GetMyClientId() && msg.type != Proto::MessageType::ServiceMessage) || msg.type == Proto::MessageType::ServiceMessage)
                    && msg.status < Proto::MessageStatus::Readed)
                {
                    if (msg.conference_tag.empty())
                    {
                        Proto::Message notice;
                        notice.guid = msg.guid;
                        notice.status = Proto::MessageStatus::Readed;
                        notices.emplace_back(notice);
                    }
                    else if (msg.conference_tag == conferenceTag)
                    {
                        Proto::Message conferenceReaded;
                        conferenceReaded.guid = msg.guid;
                        conferenceReaded.status = Proto::MessageStatus::Readed;
                        conferenceReadeds.emplace_back(conferenceReaded);
                    }
                }
            }
        }

        if (!notices.empty())
        {
            storage.UpdateMessages(notices);
            controller.DeliveryMessages(notices);
        }

        if (!conferenceReadeds.empty())
        {
            storage.UpdateMessages(conferenceReadeds);
        }
    }
}

void ContentPanel::ReceiveEvents(const wui::event &ev)
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
                    if (windowPos.is_null())
                    {
                        return;
                    }

                    auto prevWidth = list->position().width();

                    splitter->set_position({ windowPos.left - 5, windowPos.top, windowPos.left, windowPos.bottom }, false);
                    splitter->set_margins(wui::config::get_int("ListPanel", "Width", 290) + (wui::config::get_int("RenderersBox", "Showed", 0) != 0 ? 200 : 0), windowPos.right - 250);

                    titlePanel->set_position({ 10, 45, ev.internal_event_.x - 10, 70 }, false);
                    title->set_position({ 15, 50, ev.internal_event_.x - 5, 65 }, false);
                    list->set_position({ 10, 80, ev.internal_event_.x - 10, ev.internal_event_.y - 20 - 25 - (replyMessage.guid.empty() ? 0 : 40) }, false);
                    input->set_position({ 10, ev.internal_event_.y - 10 - 25, ev.internal_event_.x - 10 - 25 - 10, ev.internal_event_.y - 10 }, false);
                    sendButton->set_position({ ev.internal_event_.x - 10 - 25, ev.internal_event_.y - 10 - 25, ev.internal_event_.x - 10, ev.internal_event_.y - 10 }, false);
                    
                    replyImg->set_position({ 10, ev.internal_event_.y - 10 - 25 - 40, 10 + 25, ev.internal_event_.y - 10 - 40 }, false);
                    replyAuthor->set_position({ 10 + 35, ev.internal_event_.y - 15 - 25 - 40, ev.internal_event_.x - 10 - 35, ev.internal_event_.y - 15 - 25 - 40 + 15 }, false);
                    replyText->set_position({ 10 + 35, ev.internal_event_.y - 15 - 25 - 40 + 20, ev.internal_event_.x - 10 - 35, ev.internal_event_.y - 15 - 25 - 40 + 20 + 15 }, false);
                    replyCloseButton->set_position({ ev.internal_event_.x - 10 - 24, ev.internal_event_.y - 10 - 24 - 40, ev.internal_event_.x - 10, ev.internal_event_.y - 10 - 40 }, false);

                    if (prevWidth != list->position().width())
                    {
                        posts.clear();
                    }
                }
                break;
                case wui::internal_event_type::user_emitted:
                {
                    switch (static_cast<CPEvent>(ev.internal_event_.x))
                    {
                        case CPEvent::AddMessage:
                        {
                            int32_t count = 0;
                            {
                                std::lock_guard<std::recursive_mutex> lock(storage.MessagesMutex());
                                count = static_cast<int32_t>(storage.GetMessages().size());
                            }

                            list->set_item_count(count);
                            list->scroll_to_end();

                            if (count != 0)
                            {
                                SendReaded();
                            }
                        }
                        break;
                        case CPEvent::UpdateMessages:
                        {
                            int32_t count = 0;
                            {
                                std::lock_guard<std::recursive_mutex> lock(storage.MessagesMutex());
                                count = static_cast<int32_t>(storage.GetMessages().size());
                            }

                            auto prevItemCount = list->get_item_count();
                            list->set_item_count(count);
                            if (prevItemCount != count)
                            {
                                list->scroll_to_end();

                                SendReaded();
                            }
                        }
                        break;
                        case CPEvent::SendReaded:
                            SendReaded();
                        break;
                    }
                }
                break;
            }
        }
        break;
    }
}

void ContentPanel::SplitterCallback(int32_t x, int32_t)
{
    if (pinned)
    {
        auto mainFrame_ = mainFrame.lock();
        if (mainFrame_)
        {
            wui::config::set_int("ContentPanel", "Width", mainFrame_->position().width() - x);

            auto windowPos = window->position();
            window->set_position({ x, windowPos.top, windowPos.right, windowPos.bottom });
        }

        callback.ContentPanelWidthChanged(x);
    }
}
void ContentPanel::FormatPost(std::string_view source, FormattedPost &post)
{
    if (list->position().width() <= 0 || !memGr)
    {
        return;
    }

    int32_t maxWidth = ((list->position().width() * 3) / 4) - 10;

    std::string text, reply;
    int32_t replyWidth = maxWidth - 10;

    ParseMessageJSON(source, text, &post.replyAuthor, &reply);
    if (!reply.empty())
    {
        SplitLines(reply, post.replys, replyWidth, post.lineHeight, *memGr.get());
    }
    SplitLines(text, post.lines, maxWidth, post.lineHeight, *memGr.get());

    if (!reply.empty() && replyWidth + 10 > maxWidth)
    {
        maxWidth = replyWidth + 10;
    }

    auto timeRect = memGr->measure_text(Common::toHumanTime(post.dt, wui::get_locale()), wui::theme_font("content_box", "time_font"));
    post.size = { 0, 0,
        maxWidth > timeRect.width() + statusImageWidth + 10 ? maxWidth : timeRect.width() + statusImageWidth + 10,
        (post.author.empty() ? 0 : static_cast<int32_t>(2 * post.lineHeight)) +
        static_cast<int32_t>(post.lines.size() * post.lineHeight) + 
        (!post.replys.empty() ? static_cast<int32_t>(post.replys.size() * post.lineHeight + 2 * post.lineHeight) : 0) +
        timeRect.height() + 15 };
}

void ContentPanel::ListItemHeightCallback(int32_t nItem, int32_t &height)
{
    if (nItem < 0)
    {
        return;
    }

    {
        auto post = std::find(posts.begin(), posts.end(), nItem);
        if (post != posts.end())
        {
            height = post->size.height() + 10;
            return;
        }
    }

    std::lock_guard<std::recursive_mutex> lock(storage.MessagesMutex());
    auto &messages = storage.GetMessages();
    if (static_cast<int32_t>(messages.size()) < nItem + 1)
    {
        return;
    }

    auto &msg = messages[messages.size() - nItem - 1];

    FormattedPost post = { 0 };
    post.author = (msg.type == Proto::MessageType::TextMessage && !conferenceTag.empty() && msg.author_id != controller.GetMyClientId()) ? msg.author_name : "";

    FormatPost(msg.text, post);
                     
    post.pos = nItem;
    post.type = msg.type;
    post.dt = msg.dt;

    posts.emplace_back(post);

    height = post.size.height() + 10;
}

void ContentPanel::DrawListItem(wui::graphic &gr, int32_t nItem, const wui::rect &itemRect, wui::list::item_state state)
{
    if (nItem < 0 || !memGr)
    {
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(storage.MessagesMutex());
    auto &messages = storage.GetMessages();
    if (static_cast<int32_t>(messages.size()) < nItem + 1)
    {
        return;
    }

    auto &msg = messages[messages.size() - nItem - 1];

    bool my = msg.author_id == controller.GetMyClientId();

    auto post = std::find(posts.begin(), posts.end(), nItem);
    if (post != posts.end())
    {
        wui::rect messageRect = { my ? itemRect.width() - post->size.width() - 20 : 10, 
            itemRect.top,
            my ? itemRect.width() - 10 : post->size.width() + 40,
            itemRect.bottom };

        auto messageBackground = wui::theme_color("content_box", my ? "my_message" : "someone_message");
        gr.draw_rect(messageRect, messageBackground, messageBackground, 1, 8);

        auto textColor = wui::theme_color("content_box", "text");
        auto textFont = wui::theme_font("content_box", "message_font");

        int32_t lineNumber = 0;

        if (!post->author.empty())
        {
            auto authorColor = wui::theme_color("author_text", "color");
            auto authorFont = wui::theme_font("author_text", "font");
            gr.draw_text({ messageRect.left + 5, static_cast<int32_t>(messageRect.top + 5), messageRect.right - 5, messageRect.bottom }, post->author, authorColor, authorFont);
            
            lineNumber += 2;
        }

        if (!post->replys.empty())
        {
            auto authorColor = wui::theme_color("author_text", "color");
            auto authorFont = wui::theme_font("author_text", "font");
            gr.draw_text({ messageRect.left + 15, static_cast<int32_t>(messageRect.top + 5 + (post->lineHeight * lineNumber++)), messageRect.right - 5, messageRect.bottom }, post->replyAuthor, authorColor, authorFont);

            for (auto &line : post->replys)
            {
                gr.draw_text({ messageRect.left + 15, static_cast<int32_t>(messageRect.top + 10 + (post->lineHeight * lineNumber++)), messageRect.right - 5, messageRect.bottom }, line, textColor, textFont);
            }

            gr.draw_line({ messageRect.left + 5, messageRect.top + 5 + (!post->author.empty() ? static_cast<int32_t>(post->lineHeight * 2) : 0), messageRect.left + 5, static_cast<int32_t>(messageRect.top + 10 + (post->lineHeight * lineNumber)) }, authorColor, 2);
                
            ++lineNumber;
        }

        for (auto &line : post->lines)
        {
            gr.draw_text({ messageRect.left + 5, static_cast<int32_t>(messageRect.top + 5 + (post->lineHeight * lineNumber++)), messageRect.right - 5, messageRect.bottom }, line, textColor, textFont);
        }

        auto humanDt = Common::toHumanTime(post->dt, wui::get_locale());
        auto timeRect = memGr->measure_text(humanDt, wui::theme_font("content_box", "time_font"));
        gr.draw_text({ messageRect.right - 5 - timeRect.width() - (my ? statusImageWidth + 5 : 0),
            messageRect.bottom - 5 - timeRect.height(),
            messageRect.right - (my ? statusImageWidth + 5 : 0),
            messageRect.bottom },
            Common::toHumanTime(msg.dt, wui::get_locale()),
            wui::theme_color("content_box", "time"),
            wui::theme_font("content_box", "time_font"));

        if (my)
        {
            std::shared_ptr<wui::image> img;

            switch (msg.status)
            {
                case Proto::MessageStatus::Created: img = msgCreatedImg; break;
                case Proto::MessageStatus::Sended: img = msgSendedImg; break;
                case Proto::MessageStatus::Delivered: img = msgDeliveredImg; break;
                case Proto::MessageStatus::Readed: img = msgReadedImg; break;
            }

            img->set_position({ messageRect.right - 10 - statusImageWidth,
                messageRect.bottom - statusImageWidth - 5,
                messageRect.right - 5,
                messageRect.bottom - 5 });
            img->draw(gr, { 0 });        
        }
    }
}

void ContentPanel::ListScrollCallback(wui::scroll_state ss, int32_t)
{
    if (ss == wui::scroll_state::up_end)
    {
        std::lock_guard<std::recursive_mutex> lock(storage.MessagesMutex());

        auto prevCount = static_cast<int32_t>(storage.GetMessages().size());
        auto loadedCount = static_cast<int32_t>(storage.LoadNextMessages(MESSAGE_LIMIT));
        if (loadedCount != 0)
        {
            posts.clear();

            list->set_item_count(prevCount + loadedCount);
            list->select_item(loadedCount - 1);

            SendReaded();
        }
    }
}

void ContentPanel::MessagesUpdatedCallback(Storage::MessageAction ma, const Storage::Messages &messages)
{
    for (const auto &message : messages)
    {
        if ((message.subscriber_id != 0 && message.subscriber_id == userId) ||
            (!message.conference_tag.empty() && message.conference_tag == conferenceTag) ||
            message.status == Proto::MessageStatus::Readed)
        {
            window->emit_event(static_cast<int32_t>(ma == Storage::MessageAction::Updated ? CPEvent::UpdateMessages : CPEvent::AddMessage), 0);
            break;
        }
    }
}

void ContentPanel::RightClickItem(int32_t nItem, int32_t x, int32_t y)
{
    if (nItem < 0 || !memGr)
    {
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(storage.MessagesMutex());
    auto &messages = storage.GetMessages();
    if (static_cast<int32_t>(messages.size()) < nItem + 1)
    {
        return;
    }

    auto &msg = messages[messages.size() - nItem - 1];

    bool my = msg.author_id == controller.GetMyClientId();

    if (!my)
    {
        popupMenu->set_items({
            { 0, wui::menu_item_state::normal, wui::locale("content_panel", "reply"), "", nullptr, {}, [this, &msg](int32_t) { ReplyMessage(msg); } },
            { 1, wui::menu_item_state::normal, wui::locale("content_panel", "copy"), "", nullptr, {}, [this, &msg](int32_t) { CopyMessage(msg); } }
        });
    }
    else
    {
        popupMenu->set_items({
            { 0, wui::menu_item_state::normal, wui::locale("content_panel", "reply"), "", nullptr, {}, [this, &msg](int32_t) { ReplyMessage(msg); } },
            { 1, wui::menu_item_state::separator, wui::locale("content_panel", "copy"), "", nullptr, {}, [this, &msg](int32_t) { CopyMessage(msg); } },
            { 2, wui::menu_item_state::normal, wui::locale("content_panel", "delete"), "", nullptr, {}, [this, &msg](int32_t) { DeleteMessage(msg); } }
        });
    }

    popupMenu->show_on_point(x, y);
}

void ContentPanel::ReplyMessage(const Proto::Message &msg)
{
    auto position = window->position();
    list->set_position({ 10, 80, position.width() - 10, position.height() - 20 - 25 - 40 }, true);

    replyImg->set_position({ 10, position.height() - 10 - 25 - 40, 10 + 25, position.height() - 10 - 40 }, false);
    replyAuthor->set_position({ 10 + 35, position.height() - 15 - 25 - 40, position.width() - 10 - 35, position.height() - 15 - 25 - 40 + 15 }, false);
    replyText->set_position({ 10 + 35, position.height() - 15 - 25 - 40 + 20, position.width() - 10 - 35, position.height() - 15 - 25 - 40 + 20 + 15 }, false);
    replyCloseButton->set_position({ position.width() - 10 - 24, position.height() - 10 - 24 - 40, position.width() - 10, position.height() - 10 - 40 }, false);

    replyImg->show();
    replyAuthor->show();
    replyText->show();
    replyCloseButton->show();

    replyAuthor->set_text(msg.author_name);

    std::string text;
    ParseMessageJSON(msg.text, text);
    replyText->set_text(text);

    replyMessage = msg;
}

void ContentPanel::CloseReply()
{
    replyImg->hide();
    replyAuthor->hide();
    replyText->hide();
    replyCloseButton->hide();

    auto position = window->position();
    list->set_position({ 10, 80, position.width() - 10, position.height() - 20 - 25 }, true);

    replyMessage.Clear();
}

void ContentPanel::CopyMessage(const Proto::Message &msg)
{
    std::string text;
    ParseMessageJSON(msg.text, text);
    wui::clipboard_put(text, window->context());
}

void ContentPanel::DeleteMessage(const Proto::Message &msg)
{
    /*Storage::Messages notices;

    Proto::Message notice;
    notice.guid = msg.guid;
    notice.status = Proto::msDeleted;
    notices.emplace_back(notice);

    if (!notices.empty())
    {
        storage.UpdateMessages(notices);
        controller.DeliveryMessages(notices);
    }*/

    // todo
}

}
