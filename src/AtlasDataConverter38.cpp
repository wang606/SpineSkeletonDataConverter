#include "AtlasData.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

// 辅助函数：去除字符串两端空白
std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

// 辅助函数：分割字符串
std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(trim(token));
    }
    return tokens;
}

// 解析键值对
std::pair<std::string, std::vector<std::string>> parseEntry(const std::string& line) {
    size_t colonPos = line.find(':');
    if (colonPos == std::string::npos) {
        return {"", {}};
    }
    
    std::string key = trim(line.substr(0, colonPos));
    std::string valueStr = trim(line.substr(colonPos + 1));
    
    std::vector<std::string> values = split(valueStr, ',');
    return {key, values};
}

// 解析Format枚举
Format parseFormat(const std::string& formatStr) {
    if (formatStr == "Alpha") return Format_Alpha;
    if (formatStr == "Intensity") return Format_Intensity;
    if (formatStr == "LuminanceAlpha") return Format_LuminanceAlpha;
    if (formatStr == "RGB565") return Format_RGB565;
    if (formatStr == "RGBA4444") return Format_RGBA4444;
    if (formatStr == "RGB888") return Format_RGB888;
    if (formatStr == "RGBA8888") return Format_RGBA8888;
    return Format_RGBA8888;
}

// 解析TextureFilter枚举
TextureFilter parseTextureFilter(const std::string& filterStr) {
    if (filterStr == "Nearest") return TextureFilter_Nearest;
    if (filterStr == "Linear") return TextureFilter_Linear;
    if (filterStr == "MipMap") return TextureFilter_MipMap;
    if (filterStr == "MipMapNearestNearest") return TextureFilter_MipMapNearestNearest;
    if (filterStr == "MipMapLinearNearest") return TextureFilter_MipMapLinearNearest;
    if (filterStr == "MipMapNearestLinear") return TextureFilter_MipMapNearestLinear;
    if (filterStr == "MipMapLinearLinear") return TextureFilter_MipMapLinearLinear;
    return TextureFilter_Nearest;
}

// 解析TextureWrap
void parseRepeat(const std::string& repeatStr, TextureWrap& uWrap, TextureWrap& vWrap) {
    uWrap = TextureWrap_ClampToEdge;
    vWrap = TextureWrap_ClampToEdge;
    
    if (repeatStr == "none") return;
    
    if (repeatStr.find('x') != std::string::npos) {
        uWrap = TextureWrap_Repeat;
    }
    if (repeatStr.find('y') != std::string::npos) {
        vWrap = TextureWrap_Repeat;
    }
}

AtlasData readAtlasData(const std::string& content) {
    AtlasData atlasData;
    std::istringstream stream(content);
    std::string line;
    
    AtlasPage* currentPage = nullptr;
    
    while (std::getline(stream, line)) {
        line = trim(line);
        
        // 跳过空行
        if (line.empty()) {
            continue;
        }
        
        // 检查是否是页面名称（不包含冒号的行且不在区域内）
        if (line.find(':') == std::string::npos) {
            // 检查下一行是否包含页面属性（如size:）
            std::streampos pos = stream.tellg();
            std::string nextLine;
            bool isPageStart = false;
            
            if (std::getline(stream, nextLine)) {
                nextLine = trim(nextLine);
                if (nextLine.find("size:") == 0) {
                    isPageStart = true;
                }
            }
            stream.seekg(pos); // 恢复位置
            
            if (isPageStart) {
                // 新页面
                AtlasPage page;
                page.name = line;
                atlasData.pages.push_back(page);
                currentPage = &atlasData.pages.back();
            } else if (currentPage) {
                // 新区域
                AtlasRegion region;
                region.name = line;
                currentPage->regions.push_back(region);
            }
            continue;
        }
        
        // 解析键值对
        auto [key, values] = parseEntry(line);
        if (key.empty()) continue;
        
        if (currentPage != nullptr && currentPage->regions.empty() &&
            (key == "size" || key == "format" || key == "filter" || key == "repeat" || key == "pma")) {
            // 页面属性
            if (key == "size" && values.size() >= 2) {
                currentPage->width = std::stoi(values[0]);
                currentPage->height = std::stoi(values[1]);
            } else if (key == "format" && !values.empty()) {
                currentPage->format = parseFormat(values[0]);
            } else if (key == "filter" && values.size() >= 2) {
                currentPage->minFilter = parseTextureFilter(values[0]);
                currentPage->magFilter = parseTextureFilter(values[1]);
            } else if (key == "repeat" && !values.empty()) {
                parseRepeat(values[0], currentPage->uWrap, currentPage->vWrap);
            } else if (key == "pma" && !values.empty()) {
                currentPage->pma = (values[0] == "true");
            }
        } else if (currentPage && !currentPage->regions.empty()) {
            // 区域属性
            AtlasRegion& region = currentPage->regions.back();
            
            if (key == "bounds" && values.size() >= 4) {
                region.x = std::stoi(values[0]);
                region.y = std::stoi(values[1]);
                region.width = std::stoi(values[2]);
                region.height = std::stoi(values[3]);
            } else if (key == "xy" && values.size() >= 2) {
                region.x = std::stoi(values[0]);
                region.y = std::stoi(values[1]);
            } else if (key == "size" && values.size() >= 2) {
                region.width = std::stoi(values[0]);
                region.height = std::stoi(values[1]);
            } else if (key == "offset" && values.size() >= 2) {
                region.offsetX = std::stoi(values[0]);
                region.offsetY = std::stoi(values[1]);
            } else if (key == "offsets" && values.size() >= 4) {
                region.offsetX = std::stoi(values[0]);
                region.offsetY = std::stoi(values[1]);
                region.originalWidth = std::stoi(values[2]);
                region.originalHeight = std::stoi(values[3]);
            } else if (key == "orig" && values.size() >= 2) {
                region.originalWidth = std::stoi(values[0]);
                region.originalHeight = std::stoi(values[1]);
            } else if (key == "rotate" && !values.empty()) {
                if (values[0] == "true") {
                    region.degrees = 90;
                } else if (values[0] != "false") {
                    region.degrees = std::stoi(values[0]);
                }
            } else if (key == "index" && !values.empty()) {
                region.index = std::stoi(values[0]);
            } else if (key == "split" && values.size() >= 4) {
                region.splits.clear();
                for (const auto& val : values) {
                    region.splits.push_back(std::stoi(val));
                }
            } else if (key == "pad" && values.size() >= 4) {
                region.pads.clear();
                for (const auto& val : values) {
                    region.pads.push_back(std::stoi(val));
                }
            } else {
                // 其他自定义属性
                region.names.push_back(key);
                for (const auto& val : values) {
                    region.values.push_back(std::stoi(val));
                }
            }
        }
    }
    
    return atlasData;
}

std::string convertAtlasDataTo38(const std::string& content) {
    AtlasData atlasData = readAtlasData(content);
    std::ostringstream output;
    
    for (const auto& page : atlasData.pages) {
        // 页面名称
        output << page.name << "\n";
        
        // size (必需)
        output << "size: " << page.width << ", " << page.height << "\n";
        
        // format (必需)
        std::string formatStr = "RGBA8888";
        switch (page.format) {
            case Format_Alpha: formatStr = "Alpha"; break;
            case Format_Intensity: formatStr = "Intensity"; break;
            case Format_LuminanceAlpha: formatStr = "LuminanceAlpha"; break;
            case Format_RGB565: formatStr = "RGB565"; break;
            case Format_RGBA4444: formatStr = "RGBA4444"; break;
            case Format_RGB888: formatStr = "RGB888"; break;
            case Format_RGBA8888: formatStr = "RGBA8888"; break;
        }
        output << "format: " << formatStr << "\n";
        
        // filter (必需)
        std::string minFilterStr = "Nearest", magFilterStr = "Nearest";
        const char* filterNames[] = {"", "Nearest", "Linear", "MipMap", "MipMapNearestNearest", 
                                    "MipMapLinearNearest", "MipMapNearestLinear", "MipMapLinearLinear"};
        if (page.minFilter < 8) minFilterStr = filterNames[page.minFilter];
        if (page.magFilter < 8) magFilterStr = filterNames[page.magFilter];
        output << "filter: " << minFilterStr << ", " << magFilterStr << "\n";
        
        // repeat (必需)
        std::string repeatStr = "none";
        if (page.uWrap == TextureWrap_Repeat && page.vWrap == TextureWrap_Repeat) {
            repeatStr = "xy";
        } else if (page.uWrap == TextureWrap_Repeat) {
            repeatStr = "x";
        } else if (page.vWrap == TextureWrap_Repeat) {
            repeatStr = "y";
        }
        output << "repeat: " << repeatStr << "\n";
        
        // 输出此页面的所有区域
        for (const auto& region : page.regions) {
            // 区域名称
            output << region.name << "\n";
            
            // rotate (必需)
            if (region.degrees == 90) {
                output << "  rotate: true\n";
            } else if (region.degrees == 0) {
                output << "  rotate: false\n";
            } else {
                output << "  rotate: " << region.degrees << "\n";
            }
            
            // xy (必需)
            output << "  xy: " << region.x << ", " << region.y << "\n";
            
            // size (必需)
            output << "  size: " << region.width << ", " << region.height << "\n";
            
            // split (可选，仅当存在时)
            if (!region.splits.empty() && region.splits.size() >= 4) {
                output << "  split: " << region.splits[0] << ", " << region.splits[1] 
                       << ", " << region.splits[2] << ", " << region.splits[3] << "\n";
            }
            
            // pad (可选，仅当存在split时)
            if (!region.pads.empty() && region.pads.size() >= 4) {
                output << "  pad: " << region.pads[0] << ", " << region.pads[1] 
                       << ", " << region.pads[2] << ", " << region.pads[3] << "\n";
            }
            
            // orig (必需)
            int origWidth = region.originalWidth > 0 ? region.originalWidth : region.width;
            int origHeight = region.originalHeight > 0 ? region.originalHeight : region.height;
            output << "  orig: " << origWidth << ", " << origHeight << "\n";
            
            // offset (必需)
            output << "  offset: " << region.offsetX << ", " << region.offsetY << "\n";
            
            // index (必需) - 使用region自身的index属性
            output << "  index: " << region.index << "\n";
        }
    }
    
    return output.str();
}
