/**
 * VideoFormat.h - Contains video format structure
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#pragma once

#include <Video/Resolution.h>
#include <Video/ColorSpace.h>

#include <vector>

namespace Video
{

struct VideoFormat
{
    Video::Resolution resolution;
    std::vector<Video::ColorSpace> colorSpaces;
    
    VideoFormat()
        : resolution(0), colorSpaces() {}
    
    VideoFormat(const Video::Resolution &resolution_)
        : resolution(resolution_), colorSpaces() {}
    
    VideoFormat(const Video::Resolution &resolution_, Video::ColorSpace colorSpace)
        : resolution(resolution_), colorSpaces()
    {
        colorSpaces.emplace_back(colorSpace);
    }
    
    inline bool operator==(const VideoFormat& lv)
    {
        return lv.resolution == resolution;
    }
};

}
