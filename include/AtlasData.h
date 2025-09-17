#pragma once
#include <string>
#include <vector>

enum Format {
    Format_Alpha,
    Format_Intensity,
    Format_LuminanceAlpha,
    Format_RGB565,
    Format_RGBA4444,
    Format_RGB888,
    Format_RGBA8888
};

enum TextureFilter {
    TextureFilter_Unknown,
    TextureFilter_Nearest,
    TextureFilter_Linear,
    TextureFilter_MipMap,
    TextureFilter_MipMapNearestNearest,
    TextureFilter_MipMapLinearNearest,
    TextureFilter_MipMapNearestLinear,
    TextureFilter_MipMapLinearLinear
};

enum TextureWrap {
    TextureWrap_ClampToEdge,
    TextureWrap_Repeat,
    TextureWrap_MirroredRepeat
};

struct AtlasRegion {
    std::string name;
    int x = 0, y = 0;
    int width = 0, height = 0;
    int offsetX = 0, offsetY = 0;
    int originalWidth = 0, originalHeight = 0;
    int degrees = 0;
    int index = -1; 
    
    // 额外属性
    std::vector<int> splits;
    std::vector<int> pads;
    std::vector<std::string> names;
    std::vector<int> values;
};

struct AtlasPage {
    std::string name;
    int width = 0;
    int height = 0;
    Format format = Format_RGBA8888;
    TextureFilter minFilter = TextureFilter_Nearest;
    TextureFilter magFilter = TextureFilter_Nearest;
    TextureWrap uWrap = TextureWrap_ClampToEdge;
    TextureWrap vWrap = TextureWrap_ClampToEdge;
    bool pma = false;
    
    std::vector<AtlasRegion> regions;
};

struct AtlasData {
    std::vector<AtlasPage> pages;
};

std::string convertAtlasDataTo38(const std::string& content);
