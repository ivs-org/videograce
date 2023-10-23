/**
 * RenderersBox.h - Contains video renderers box interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#pragma once

#include <UI/MiniWindow.h>
#include <UI/DemonstrationWindow.h>

#include <RendererSession/RendererVideoSession.h>

#include <wui/window/window.hpp>

#include <vector>

#include <mt/timer.h>

namespace Client
{

enum class GridType
{
    Even = 0,
    MainUser
};

class RenderersBox
{
public:
    RenderersBox(std::weak_ptr<wui::window> mainFrame, MiniWindow &miniWindow, std::vector<std::shared_ptr<RendererSession::RendererVideoSession>> &renderersVideo);
    ~RenderersBox();

    void Run(const wui::rect &pos);
    void End();
    bool IsShowed() const;

    /// Interface
    void Update();

    void DeleteRenderer(uint32_t deviceId);

	void SetGridType(GridType gridType);
	GridType GetGridType() const;

	void AccentuateSpeakerRenderer(int64_t clientId);
	
    void SetPosition(const wui::rect &pos);

private:
    std::weak_ptr<wui::window> parent;
    MiniWindow &miniWindow;
    std::vector<std::shared_ptr<RendererSession::RendererVideoSession>> &sourceRenderers, showedRenderers;

    std::map<uint32_t, std::shared_ptr<DemonstrationWindow>> demonstrationWindows;

    wui::rect position;

    GridType gridType;

    mt::timer timer_;

    wui::rect FitRenderer(Video::Resolution resolution, int32_t left, int32_t top, int32_t width, int32_t height);
    wui::rect GetRendererPos(uint32_t pos, Video::Resolution resolution);
    wui::rect GetRendererEvenGridPos(uint32_t pos, Video::Resolution resolution);
    wui::rect GetRendererBigGridPos(uint32_t pos, Video::Resolution resolution);

    void Reorder();

    void Redraw();
};

}
