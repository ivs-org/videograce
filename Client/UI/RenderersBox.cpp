/**
 * RenderersBox.cpp - Contains video renderers box impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <UI/RenderersBox.h>

#include <wui/config/config.hpp>
#include <wui/locale/locale.hpp>

#include <wui/system/wm_tools.hpp>

namespace Client
{

RenderersBox::RenderersBox(std::weak_ptr<wui::window> mainFrame_,
    MiniWindow &miniWindow_,
    std::vector<std::shared_ptr<RendererSession::RendererVideoSession>> &renderersVideo_,
    Controller::IController& controller_)
    : parent(mainFrame_), miniWindow(miniWindow_), controller(controller_),
    sourceRenderers(renderersVideo_),
    showedRenderers(),
    demonstrationWindows(),
    position{},
    gridType(GridType::MainUser),
    timer_(std::bind(&RenderersBox::Redraw, this))
{
}

RenderersBox::~RenderersBox()
{
    End();
}

void RenderersBox::Run(const wui::rect &pos)
{
    position = pos;

    Update();

    timer_.start(40);

    wui::config::set_int("RenderersBox", "Showed", 1);
}

void RenderersBox::End()
{
    timer_.stop();

    auto parent_ = parent.lock();
    if (parent_)
    {
        for (auto &rvs : showedRenderers)
        {
            parent_->remove_control(rvs->GetControl());
        }
    }

    demonstrationWindows.clear();

    position = { 0 };

    wui::config::set_int("RenderersBox", "Showed", 0);
}

bool RenderersBox::IsShowed() const
{
    return !position.is_null();
}

void RenderersBox::Update()
{
    auto parent_ = parent.lock();
    if (!parent_)
    {
        return;
    }

    if (wui::config::get_int("User", "UseDemonstrationWindow", 1) == 0)
    {
        showedRenderers = sourceRenderers;
    }
    else
    {
        auto screenSize = wui::get_screen_size(parent_->context());
        screenSize.right *= 0.9;
        screenSize.bottom *= 0.9;

        showedRenderers.clear();

        for (auto &rvs : sourceRenderers)
        {
            if (rvs->GetDeviceType() != Proto::DeviceType::Demonstration ||
                (rvs->GetDeviceType() == Proto::DeviceType::Demonstration && rvs->GetMy()))
            {
                showedRenderers.emplace_back(rvs);
            }
            else
            {
                auto deviceId = rvs->GetDeviceId();
                auto dw = demonstrationWindows.find(deviceId);
                if (dw == demonstrationWindows.end())
                {
                    demonstrationWindows[deviceId] = std::make_shared<DemonstrationWindow>(*rvs, controller, screenSize);
                }
            }
        }
    }

    Reorder();
}

void RenderersBox::Reorder()
{
    auto parent_ = parent.lock();
    if (!parent_)
    {
        return;
    }

    if (showedRenderers.size() == 2)
    {
        RendererSession::RendererVideoSessionPtr_t my, remote;
        for (auto &rvs : showedRenderers)
        {
            if (rvs->GetMy())
            {
                my = rvs;
            }
            else
            {
                remote = rvs;
            }
        }

        if (!remote)
        {
            remote = showedRenderers.front();
        }

        if (!my)
        {
            my = *std::find_if(showedRenderers.begin(), showedRenderers.end(), [remote](const RendererSession::RendererVideoSessionPtr_t &p) { return p != remote; });
        }

        auto pos = GetRendererPos(0, remote->GetResolution());
        parent_->add_control(remote->GetControl(), pos);
        remote->GetControl()->set_position(pos, false);

        pos = GetRendererPos(1, my->GetResolution());
        parent_->add_control(my->GetControl(), pos);
        my->GetControl()->set_position(pos, false);
        parent_->bring_to_front(my->GetControl());

        miniWindow.ShowRenderer(remote->GetControl());

        return;
    }

    uint32_t number = 0;
    for (auto &rvs : showedRenderers)
    {
        auto pos = GetRendererPos(number++, rvs->GetResolution());

        parent_->add_control(rvs->GetControl(), pos);
        rvs->GetControl()->set_position(pos, false);
    }

    if (!showedRenderers.empty())
    {
        miniWindow.ShowRenderer(showedRenderers.front()->GetControl());
    }
}

void RenderersBox::DeleteRenderer(uint32_t deviceId)
{
    auto dw = demonstrationWindows.find(deviceId);
    if (dw != demonstrationWindows.end())
    {
        demonstrationWindows.erase(dw);
    }
}

void RenderersBox::SetGridType(GridType gridType_)
{
    gridType = gridType_;
    Reorder();
}

GridType RenderersBox::GetGridType() const
{
    return gridType;
}

int32_t GetClientRendererPos(int64_t clientId, std::vector<std::shared_ptr<RendererSession::RendererVideoSession>> &renderersVideo)
{
    uint32_t cnt = 0;
    for (const auto &rvs : renderersVideo)
    {
        if (rvs->GetClientId() == clientId)
        {
            return cnt;
        }

        ++cnt;
    }

    return -1;
}

void RenderersBox::AccentuateSpeakerRenderer(int64_t clientId)
{
    auto rendPos = GetClientRendererPos(clientId, showedRenderers);

    if (showedRenderers.size() < 2 || rendPos == -1)
    {
        return;
    }

    auto firstIt = showedRenderers.begin();
    for (int i = 0;; ++i)
    {
        if ((*(firstIt + i))->Started())
        {
            std::iter_swap(firstIt + rendPos, firstIt + i);
            break;
        }
    }

    Reorder();
}

void RenderersBox::SetPosition(const wui::rect &pos)
{
    wui::config::set_int("RenderersBox", "Width", pos.width());
    position = pos;

    Reorder();

    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->redraw(position, true);
    }
}

void RenderersBox::EnableRC(int64_t clientId, bool yes)
{
    for (auto& dw : demonstrationWindows)
    {
        if (dw.second->GetClientId() == clientId)
        {
            dw.second->EnableRC(yes);
        }
    }
}

wui::rect RenderersBox::GetRendererPos(uint32_t pos, Video::Resolution resolution)
{
    switch (gridType)
    {
        case GridType::Even:
            return GetRendererEvenGridPos(pos, resolution);
        break;
        case GridType::MainUser:
            return GetRendererBigGridPos(pos, resolution);
        break;
    }

    return { 0 };
}

wui::rect RenderersBox::FitRenderer(Video::Resolution resolution, int32_t left, int32_t top, int32_t width, int32_t height)
{
    wui::rect out = { 0 };

    if (resolution == 0)
    {
        return out;
    }

    Video::ResolutionValues rv = Video::GetValues(resolution);

    int32_t X = width,
        Y = height,
        W = rv.width,
        H = rv.height;

    if ((static_cast<double>(H) / W) > 0.57)
    {
        W = static_cast<int32_t>(static_cast<double>(H) / 0.56);
    }

    int32_t x = 0, y = 0;

    x = X;
    y = H * X / W;
    if (y > Y)
    {
        y = Y;
        x = W * Y / H;
        out.left = (X - x) / 2;
        out.top = 0;
    }
    else
    {
        out.left = 0;
        out.top = (Y - y) / 2;
    }

    out.left += position.left + left + 1;
    out.top += position.top + top + 1;
    out.right = out.left + x - 1;
    out.bottom = out.top + y - 1;

    return out;
}

wui::rect RenderersBox::GetRendererEvenGridPos(uint32_t pos, Video::Resolution resolution)
{
    auto width = position.width(), height = position.height();
    
    int32_t lines = 1;
    int32_t columns = 1;

    auto count = showedRenderers.size();
    if (count == 1) // 1
    {
        lines = 1;
        columns = 1;
    }
    else if (count == 2) // 2
    {
        lines = 2;
        columns = 1;
    }
    else if (count > 2 && count < 5) // 2x2
    {
        lines = 2;
        columns = 2;
    }
    else if (count > 4 && count < 7) // 2x3
    {
        lines = 3;
        columns = 2;
    }
    else if (count > 6 && count < 10) // 3x3
    {
        lines = 3;
        columns = 3;
    }
    else if (count > 9 && count < 13) // 3x4
    {
        lines = 3;
        columns = 4;
    }
    else if (count > 12 && count < 17) // 4x4
    {
        lines = 4;
        columns = 4;
    }
    else if (count > 16 && count < 21) // 5x4
    {
        lines = 5;
        columns = 4;
    }
    else if (count > 20 && count < 26) // 5x5
    {
        lines = 5;
        columns = 5;
    }
    else if (count > 25 && count < 31) // 6x5
    {
        lines = 6;
        columns = 5;
    }
    else if (count > 30 && count < 37) // 6x6
    {
        lines = 6;
        columns = 6;
    }
    else if (count > 36 && count < 43) // 7x6
    {
        lines = 7;
        columns = 6;
    }
    else if (count > 42 && count < 50) // 7x7
    {
        lines = 7;
        columns = 7;
    }
    else if (count > 49 && count < 57) // 8x7
    {
        lines = 8;
        columns = 7;
    }
    else if (count > 56 && count < 65) // 8x8
    {
        lines = 8;
        columns = 8;
    }
    else if (count > 64 && count < 73) // 9x8
    {
        lines = 9;
        columns = 8;
    }
    else if (count > 72 && count < 82) // 9x9
    {
        lines = 9;
        columns = 9;
    }
    else if (count > 81 && count < 91) // 10x9
    {
        lines = 10;
        columns = 9;
    }
    else if (count > 90 && count < 101) // 10x10
    {
        lines = 10;
        columns = 10;
    }
    else if (count > 100 && count < 111) // 11x10
    {
        lines = 11;
        columns = 10;
    }
    else if (count > 110 && count < 121) // 11x11
    {
        lines = 11;
        columns = 11;
    }
    else if (count > 120 && count < 133) // 12x11
    {
        lines = 12;
        columns = 11;
    }
    else if (count > 132 && count < 145) // 12x12
    {
        lines = 12;
        columns = 12;
    }
    else if (count > 144 && count < 157) // 13x12
    {
        lines = 13;
        columns = 12;
    }
    else if (count > 156 && count < 170) // 13x13
    {
        lines = 13;
        columns = 13;
    }
    else if (count > 169 && count < 183) // 14x13
    {
        lines = 14;
        columns = 13;
    }
    else if (count > 182 && count < 197) // 14x14
    {
        lines = 14;
        columns = 14;
    }
    else if (count > 196 && count < 211) // 15x14
    {
        lines = 15;
        columns = 14;
    }
    else if (count > 210 && count < 226) // 15x15
    {
        lines = 15;
        columns = 15;
    }

	width = width / columns;
	height = height / lines;

    int32_t left = 0, top = 0;
    int32_t cnt = 0;

    for (int32_t curLine = 0; curLine != lines; ++curLine)
    {
        for (int32_t curColumn = 0; curColumn != columns; ++curColumn)
        {
            if (cnt == pos)
            {
                goto endfor; // not break this!
            }
            left += width;

            ++cnt;
        }
        top += height;
        left = 0;
    }
endfor: // the ending goto label

    return FitRenderer(resolution, left, top, width, height);
}

wui::rect RenderersBox::GetRendererBigGridPos(uint32_t pos, Video::Resolution resolution)
{
    auto wndWidth = position.width(), wndHeight = position.height();

    auto count = showedRenderers.size();

    if (count == 1)
    {
        return FitRenderer(resolution, 0, 0, wndWidth, wndHeight);
    }
    else if (count == 2)
    {
        long smallRendWidth = static_cast<long>(wndWidth / 4);
        long smallRendHeight = static_cast<long>(wndHeight / 4);

        switch (pos)
        {
        case 0: return FitRenderer(resolution, 0, 0, wndWidth, wndHeight); break;
        case 1:	return FitRenderer(resolution, wndWidth - smallRendWidth, wndHeight - smallRendHeight, smallRendWidth, smallRendHeight); break;
        }
    }
    else if (count >= 3 && count <= 5)
    {
        long bigRendWidth = static_cast<long>(wndWidth * 0.75);

        long smallRendWidth = static_cast<long>(wndWidth * 0.25);
        long smallRendHeight = static_cast<long>(wndHeight * 0.25);

        switch (pos)
        {
        case 0: return FitRenderer(resolution, 0, 0, bigRendWidth, wndHeight); break;
        case 1:	return FitRenderer(resolution, bigRendWidth, 0, smallRendWidth, smallRendHeight); break;
        case 2:	return FitRenderer(resolution, bigRendWidth, smallRendHeight, smallRendWidth, smallRendHeight); break;
        case 3:	return FitRenderer(resolution, bigRendWidth, smallRendHeight * 2, smallRendWidth, smallRendHeight); break;
        case 4:	return FitRenderer(resolution, bigRendWidth, smallRendHeight * 3, smallRendWidth, smallRendHeight); break;
        }
    }
    else if (count >= 6 && count <= 8)
    {
        long bigRendWidth = static_cast<long>(wndWidth * 0.75); // 0.75 - 3/4
        long bigRendHeight = static_cast<long>(wndHeight * 0.75);

        long smallRendWidth = static_cast<long>(wndWidth / 4);
        long smallRendHeight = static_cast<long>(wndHeight / 4);

        switch (pos)
        {
        case 0: return FitRenderer(resolution, 0, 0, bigRendWidth, bigRendHeight); break;
        case 1:	return FitRenderer(resolution, bigRendWidth, 0, smallRendWidth, smallRendHeight); break;
        case 2:	return FitRenderer(resolution, bigRendWidth, smallRendHeight, smallRendWidth, smallRendHeight); break;
        case 3:	return FitRenderer(resolution, bigRendWidth, smallRendHeight * 2, smallRendWidth, smallRendHeight); break;
        case 4:	return FitRenderer(resolution, bigRendWidth, smallRendHeight * 3, smallRendWidth, smallRendHeight); break;
        case 5: return FitRenderer(resolution, 0, bigRendHeight, smallRendWidth, smallRendHeight); break;
        case 6:	return FitRenderer(resolution, smallRendWidth, bigRendHeight, smallRendWidth, smallRendHeight); break;
        case 7:	return FitRenderer(resolution, smallRendWidth * 2, bigRendHeight, smallRendWidth, smallRendHeight); break;
        }
    }
    else if (count >= 9 && count <= 13)
    {
        long bigRendWidth = static_cast<long>(wndWidth / 2);
        long bigRendHeight = static_cast<long>(wndHeight / 2);

        long smallRendWidth = static_cast<long>(wndWidth / 4);
        long smallRendHeight = static_cast<long>(wndHeight / 4);

        switch (pos)
        {
        case 0: return FitRenderer(resolution, smallRendWidth, smallRendHeight, bigRendWidth, bigRendHeight); break;
        case 1:	return FitRenderer(resolution, 0, 0, smallRendWidth, smallRendHeight); break;
        case 2:	return FitRenderer(resolution, smallRendWidth, 0, smallRendWidth, smallRendHeight); break;
        case 3:	return FitRenderer(resolution, smallRendWidth * 2, 0, smallRendWidth, smallRendHeight); break;
        case 4:	return FitRenderer(resolution, smallRendWidth * 3, 0, smallRendWidth, smallRendHeight); break;
        case 5: return FitRenderer(resolution, smallRendWidth * 3, smallRendHeight, smallRendWidth, smallRendHeight); break;
        case 6: return FitRenderer(resolution, smallRendWidth * 3, smallRendHeight * 2, smallRendWidth, smallRendHeight); break;
        case 7: return FitRenderer(resolution, smallRendWidth * 3, smallRendHeight * 3, smallRendWidth, smallRendHeight); break;
        case 8: return FitRenderer(resolution, smallRendWidth * 2, smallRendHeight * 3, smallRendWidth, smallRendHeight); break;
        case 9: return FitRenderer(resolution, smallRendWidth, smallRendHeight * 3, smallRendWidth, smallRendHeight); break;
        case 10: return FitRenderer(resolution, 0, smallRendHeight * 3, smallRendWidth, smallRendHeight); break;
        case 11: return FitRenderer(resolution, 0, smallRendHeight * 2, smallRendWidth, smallRendHeight); break;
        case 12: return FitRenderer(resolution, 0, smallRendHeight, smallRendWidth, smallRendHeight); break;
        }
    }
    else if (count >= 14 && count <= 17)
    {
        long bigRendWidth = static_cast<long>(wndWidth * 0.6); // 0.6 - 3/5
        long bigRendHeight = static_cast<long>(wndHeight * 0.6);

        long smallRendWidth = static_cast<long>(wndWidth / 5);
        long smallRendHeight = static_cast<long>(wndHeight / 5);

        switch (pos)
        {
        case 0: return FitRenderer(resolution, smallRendWidth, smallRendHeight, bigRendWidth, bigRendHeight); break;
        case 1:	return FitRenderer(resolution, 0, 0, smallRendWidth, smallRendHeight); break;
        case 2:	return FitRenderer(resolution, smallRendWidth, 0, smallRendWidth, smallRendHeight); break;
        case 3:	return FitRenderer(resolution, smallRendWidth * 2, 0, smallRendWidth, smallRendHeight); break;
        case 4:	return FitRenderer(resolution, smallRendWidth * 3, 0, smallRendWidth, smallRendHeight); break;
        case 5:	return FitRenderer(resolution, smallRendWidth * 4, 0, smallRendWidth, smallRendHeight); break;
        case 6: return FitRenderer(resolution, smallRendWidth * 4, smallRendHeight, smallRendWidth, smallRendHeight); break;
        case 7: return FitRenderer(resolution, smallRendWidth * 4, smallRendHeight * 2, smallRendWidth, smallRendHeight); break;
        case 8: return FitRenderer(resolution, smallRendWidth * 4, smallRendHeight * 3, smallRendWidth, smallRendHeight); break;
        case 9: return FitRenderer(resolution, smallRendWidth * 4, smallRendHeight * 4, smallRendWidth, smallRendHeight); break;
        case 10: return FitRenderer(resolution, smallRendWidth * 3, smallRendHeight * 4, smallRendWidth, smallRendHeight); break;
        case 11: return FitRenderer(resolution, smallRendWidth * 2, smallRendHeight * 4, smallRendWidth, smallRendHeight); break;
        case 12: return FitRenderer(resolution, smallRendWidth, smallRendHeight * 4, smallRendWidth, smallRendHeight); break;
        case 13: return FitRenderer(resolution, 0, smallRendHeight * 4, smallRendWidth, smallRendHeight); break;
        case 14: return FitRenderer(resolution, 0, smallRendHeight * 3, smallRendWidth, smallRendHeight); break;
        case 15: return FitRenderer(resolution, 0, smallRendHeight * 2, smallRendWidth, smallRendHeight); break;
        case 16: return FitRenderer(resolution, 0, smallRendHeight, smallRendWidth, smallRendHeight); break;
        }
    }
    else if (count > 17)
    {
        return GetRendererEvenGridPos(pos, resolution);
    }

    return { 0 };
}

void RenderersBox::Redraw()
{
    auto parent_ = parent.lock();
    if (parent_)
    {
        parent_->redraw({ position.left - 5,
                position.top,
                position.right + 5,
                position.bottom },
            true);
    }
}

}
