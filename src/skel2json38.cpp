#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include "json.hpp"
using json = nlohmann::ordered_json;
#include "jsonformat.hpp"

namespace spine38 {

const int AttachmentType_Region = 0;
const int AttachmentType_BoundingBox = 1;
const int AttachmentType_Mesh = 2;
const int AttachmentType_LinkedMesh = 3;
const int AttachmentType_Path = 4;
const int AttachmentType_Point = 5;
const int AttachmentType_Clipping = 6;

const int BONE_ROTATE = 0;
const int BONE_TRANSLATE = 1;
const int BONE_SCALE = 2;
const int BONE_SHEAR = 3;

const int SLOT_ATTACHMENT = 0;
const int SLOT_COLOR = 1;
const int SLOT_TWO_COLOR = 2;

const int PATH_POSITION = 0;
const int PATH_SPACING = 1;
const int PATH_MIX = 2;

const int CURVE_LINEAR = 0;
const int CURVE_STEPPED = 1;
const int CURVE_BEZIER = 2;

std::vector<std::string> TransformMode = {
    "normal", 
    "onlyTranslation", 
    "noRotationOrReflection",
    "noScale",
    "noScaleOrReflection"
}; 

std::vector<std::string> BlendMode = {
    "normal", 
    "additive",
    "multiply",
    "screen"
}; 

std::vector<std::string> PositionMode = {
    "fixed", 
    "percent"
}; 

std::vector<std::string> SpacingMode = {
    "length", 
    "fixed", 
    "percent", 
    "proportional"
};

std::vector<std::string> RotateMode = {
    "tangent", 
    "chain", 
    "chainScale"
};

std::vector<std::string> AttachmentType = {
    "region", 
    "boundingbox", 
    "mesh", 
    "linkedmesh", 
    "path", 
    "point", 
    "clipping"
}; 

std::vector<std::string> SequenceMode = {
    "hold", 
    "once", 
    "loop", 
    "pingpong", 
    "onceReverse", 
    "loopReverse", 
    "pingpongReverse"
}; 

struct DataInput {
    const unsigned char* cursor; 
    const unsigned char* end;
}; 

struct Color {
    int r, g, b, a;
};

unsigned char readByte(DataInput* input) {
    return *input->cursor++;
}

signed char readSByte(DataInput *input) {
    return (signed char) readByte(input);
}

bool readBoolean(DataInput *input) {
    return readByte(input) != 0;
}

int readInt(DataInput *input) {
    int result = readByte(input);
    result <<= 8;
    result |= readByte(input);
    result <<= 8;
    result |= readByte(input);
    result <<= 8;
    result |= readByte(input);
    return result;
}

void _readColor(DataInput *input, Color &color, bool hasAlpha) {
    color.r = readByte(input);
    color.g = readByte(input);
    color.b = readByte(input);
    color.a = hasAlpha ? readByte(input) : 255;
}

std::string Color2String(const Color& color, bool hasAlpha) {
    char buffer[9];
    snprintf(buffer, sizeof(buffer), "%02x%02x%02x", 
             static_cast<int>(color.r), 
             static_cast<int>(color.g), 
             static_cast<int>(color.b));
    if (hasAlpha) {
        snprintf(buffer + 6, 3, "%02x", static_cast<int>(color.a));
    }
    return std::string(buffer);
}

std::string readColor(DataInput* input, bool hasAlpha) {
    Color color;
    _readColor(input, color, hasAlpha);
    return Color2String(color, hasAlpha);
}

int readVarint(DataInput *input, bool optimizePositive) {
    unsigned char b = readByte(input);
    int value = b & 0x7F;
    if (b & 0x80) {
        b = readByte(input);
        value |= (b & 0x7F) << 7;
        if (b & 0x80) {
            b = readByte(input);
            value |= (b & 0x7F) << 14;
            if (b & 0x80) {
                b = readByte(input);
                value |= (b & 0x7F) << 21;
                if (b & 0x80) value |= (readByte(input) & 0x7F) << 28;
            }
        }
    }
    if (!optimizePositive) value = (((unsigned int) value >> 1) ^ -(value & 1));
    return value;
}

float readFloat(DataInput *input) {
    union {
        int intValue;
        float floatValue;
    } intToFloat;
    intToFloat.intValue = readInt(input);
    return intToFloat.floatValue;
}

std::string readString(DataInput* input) {
    int length = readVarint(input, true);
    if (length == 0) return ""; 
    std::string result;
    result.resize(length - 1);
    memcpy(result.data(), input->cursor, length - 1);
    input->cursor += length - 1;
    return result;
}

std::string readStringRef(DataInput* input, const json& root) {
    int index = readVarint(input, true);
    return (index == 0) ? "" : root["strings"][index - 1];
}

void readFloatArray(DataInput* input, int n, std::vector<float>& array) {
    array.resize(n);
    for (int i = 0; i < n; i++) {
        array[i] = readFloat(input); 
    }
}

void readShortArray(DataInput* input, std::vector<unsigned short>& array) {
    int n = readVarint(input, true); 
    array.resize(n); 
    for (int i = 0; i < n; i++) {
        array[i] = readByte(input) << 8;
        array[i] |= readByte(input);
    }
}

void readVertices(DataInput* input, std::vector<float>& vertices, std::vector<int>& bones, int vertexCount) {
    int verticesLength = vertexCount << 1;
    if (!readBoolean(input)) {
        readFloatArray(input, verticesLength, vertices);
        return;
    }
    for (int i = 0; i < vertexCount; i++) {
        int boneCount = readVarint(input, true);
        bones.push_back(boneCount);
        for (int ii = 0; ii < boneCount; ii++) {
            bones.push_back(readVarint(input, true));
            vertices.push_back(readFloat(input));
            vertices.push_back(readFloat(input));
            vertices.push_back(readFloat(input));
        }
    }
}

std::vector<float> combineBonesAndVertices(std::vector<int>& bones, std::vector<float>& vertices) {
    if (bones.empty()) return vertices; 
    std::vector<float> combined;
    size_t vIndex = 0;
    for (size_t i = 0; i < bones.size(); ) {
        int boneCount = bones[i++];
        combined.push_back(boneCount);
        for (int j = 0; j < boneCount; j++) {
            combined.push_back(bones[i++]); 
            combined.push_back(vertices[vIndex++]); 
            combined.push_back(vertices[vIndex++]); 
            combined.push_back(vertices[vIndex++]); 
        }
    }
    return combined;
}

json readAttachment(DataInput* input, const json& skin, int slotIndex, const std::string& attachmentName, const json& root, bool nonessential) {
    json attachment = json::object(); 
    std::string name = readStringRef(input, root);
    if (!name.empty())
        attachment["name"] = name;
    int type = readByte(input); 
    attachment["type"] = AttachmentType[type];
    switch (type) {
        case AttachmentType_Region: {
            std::string path = readStringRef(input, root);
            if (!path.empty())
                attachment["path"] = path;
            attachment["rotation"] = readFloat(input);
            attachment["x"] = readFloat(input);
            attachment["y"] = readFloat(input);
            attachment["scaleX"] = readFloat(input);
            attachment["scaleY"] = readFloat(input);
            attachment["width"] = readFloat(input);
            attachment["height"] = readFloat(input);
            attachment["color"] = readColor(input, true);
            return attachment; 
        }
        case AttachmentType_BoundingBox: {
            int vertexCount = readVarint(input, true);
            std::vector<float> vertices; 
            std::vector<int> bones;
            readVertices(input, vertices, bones, vertexCount);
            attachment["vertexCount"] = vertexCount; 
            attachment["vertices"] = combineBonesAndVertices(bones, vertices);
            if (nonessential) {
                // attachment["color"] = readColor(input, true);
                /* Skip color. */
                readInt(input); 
            }
            return attachment; 
        }
        case AttachmentType_Mesh: {
            std::string path = readStringRef(input, root);
            if (!path.empty())
                attachment["path"] = path;
            attachment["color"] = readColor(input, true);
            int vertexCount = readVarint(input, true);
            std::vector<float> uvs; 
            readFloatArray(input, vertexCount << 1, uvs);
            attachment["uvs"] = uvs;
            std::vector<unsigned short> triangles;
            readShortArray(input, triangles);
            attachment["triangles"] = triangles;
            std::vector<float> vertices;
            std::vector<int> bones; 
            readVertices(input, vertices, bones, vertexCount);
            attachment["vertices"] = combineBonesAndVertices(bones, vertices);
            attachment["hull"] = readVarint(input, true);
            if (nonessential) {
                std::vector<unsigned short> edges;
                readShortArray(input, edges);
                attachment["edges"] = edges;
                attachment["width"] = readFloat(input);
                attachment["height"] = readFloat(input);
            }
            return attachment; 
        }
        case AttachmentType_LinkedMesh: {
            std::string path = readStringRef(input, root);
            if (!path.empty())
                attachment["path"] = path;
            attachment["color"] = readColor(input, true);
            attachment["skin"] = readStringRef(input, root);
            attachment["parent"] = readStringRef(input, root);
            attachment["deform"] = int(readBoolean(input)); 
            if (nonessential) {
                attachment["width"] = readFloat(input);
                attachment["height"] = readFloat(input);
            }
            return attachment;
        }
        case AttachmentType_Path: {
            attachment["closed"] = int(readBoolean(input)); 
            attachment["constantSpeed"] = int(readBoolean(input));
            int vertexCount = readVarint(input, true); 
            std::vector<float> vertices; 
            std::vector<int> bones; 
            readVertices(input, vertices, bones, vertexCount);
            attachment["vertexCount"] = vertexCount;
            attachment["vertices"] = combineBonesAndVertices(bones, vertices);
            std::vector<float> lengths;
            for (int i = 0; i < vertexCount / 3; i++) {
                lengths.push_back(readFloat(input));
            }
            attachment["lengths"] = lengths;
            if (nonessential) {
                // attachment["color"] = readColor(input, true);
                /* Skip color. */
                readInt(input); 
            }
            return attachment;
        }
        case AttachmentType_Point: {
            attachment["rotation"] = readFloat(input); 
            attachment["x"] = readFloat(input);
            attachment["y"] = readFloat(input);
            if (nonessential) {
                // attachment["color"] = readColor(input, true);
                /* Skip color. */
                readInt(input); 
            }
            return attachment;
        }
        case AttachmentType_Clipping: {
            int endSlotIndex = readVarint(input, true);
            attachment["end"] = root["slots"][endSlotIndex]["name"].get<std::string>();
            int vertexCount = readVarint(input, true); 
            std::vector<float> vertices; 
            std::vector<int> bones; 
            readVertices(input, vertices, bones, vertexCount);
            attachment["vertexCount"] = vertexCount;
            attachment["vertices"] = combineBonesAndVertices(bones, vertices);
            if (nonessential) {
                // attachment["color"] = readColor(input, true);
                /* Skip color. */
                readInt(input); 
            }
            return attachment;
        }
    }
    return attachment;
}

json readSkin(DataInput* input, bool defaultSkin, const json& root, bool nonessential) {
    json skin = json::object();
    int slotCount = 0; 
    if (defaultSkin) {
        slotCount = readVarint(input, true);
        if (slotCount == 0) return json::object();
        skin["name"] = "default";
    } else {
        skin["name"] = readStringRef(input, root);
        for (int i = 0, n = readVarint(input, true); i < n; i++) {
            int boneIndex = readVarint(input, true);
            skin["bones"].push_back(root["bones"][boneIndex]["name"].get<std::string>());
        }
        for (int i = 0, n = readVarint(input, true); i < n; i++) {
            int ikIndex = readVarint(input, true);
            skin["ik"].push_back(root["ik"][ikIndex]["name"]);
        }
        for (int i = 0, n = readVarint(input, true); i < n; i++) {
            int transformIndex = readVarint(input, true);
            skin["transform"].push_back(root["transform"][transformIndex]["name"]);
        }
        for (int i = 0, n = readVarint(input, true); i < n; i++) {
            int pathIndex = readVarint(input, true);
            skin["path"].push_back(root["path"][pathIndex]["name"]);
        }
        slotCount = readVarint(input, true);
    }
    skin["attachments"] = json::object(); 
    for (int i = 0; i < slotCount; i++) {
        int slotIndex = readVarint(input, true);
        skin["attachments"][root["slots"][slotIndex]["name"].get<std::string>()] = json::object();
        for (int ii = 0, nn = readVarint(input, true); ii < nn; ++ii) {
            std::string name = readStringRef(input, root);
            json attachment = readAttachment(input, skin, slotIndex, name, root, nonessential);
            skin["attachments"][root["slots"][slotIndex]["name"].get<std::string>()][name] = attachment;
        }
    }
    return skin;
}

void readCurve(DataInput* input, json& timeline) {
    switch (readByte(input)) {
        case CURVE_STEPPED: {
            timeline["curve"] = "stepped";
            break;
        }
        case CURVE_BEZIER: {
            timeline["curve"] = readFloat(input);
            timeline["c2"] = readFloat(input);
            timeline["c3"] = readFloat(input);
            timeline["c4"] = readFloat(input);
            break;
        }
    }
}

void readTimeline(DataInput* input, json& timelines, int frameCount, int valueCount, std::string name1, std::string name2 = "") {
    timelines = json::array(); 
    for (int frame = 0; frame < frameCount; frame++) {
        timelines.push_back(json::object());
        timelines.back()["time"] = readFloat(input); 
        timelines.back()[name1] = readFloat(input);
        if (valueCount > 1)
            timelines.back()[name2] = readFloat(input);
        if (frame < frameCount - 1) 
            readCurve(input, timelines.back());
    }
}

json readAnimation(const std::string& name, DataInput* input, json& root) {
    json animation = json::object(); 
    // Slot timelines. 
    animation["slots"] = json::object(); 
    for (int i = 0, n = readVarint(input, true); i < n; i++) {
        int slotIndex = readVarint(input, true); 
        std::string slotName = root["slots"][slotIndex]["name"].get<std::string>();
        animation["slots"][slotName] = json::object();
        for (int ii = 0, nn = readVarint(input, true); ii < nn; ii++) {
            unsigned char timelineType = readByte(input);
            int frameCount = readVarint(input, true); 
            switch (timelineType) {
                case SLOT_ATTACHMENT: {
                    animation["slots"][slotName]["attachment"] = json::array(); 
                    for (int frame = 0; frame < frameCount; frame++) {
                        float time = readFloat(input);
                        std::string attachmentName = readStringRef(input, root);
                        animation["slots"][slotName]["attachment"].push_back(json::object());
                        animation["slots"][slotName]["attachment"].back()["time"] = time;
                        animation["slots"][slotName]["attachment"].back()["name"] = attachmentName;
                    }
                    break; 
                }
                case SLOT_COLOR: {
                    animation["slots"][slotName]["color"] = json::array();
                    for (int frame = 0; frame < frameCount; frame++) {
                        float time = readFloat(input);
                        std::string color = readColor(input, true);
                        animation["slots"][slotName]["color"].push_back(json::object());
                        animation["slots"][slotName]["color"].back()["time"] = time;
                        animation["slots"][slotName]["color"].back()["color"] = color;
                        if (frame < frameCount - 1) 
                            readCurve(input, animation["slots"][slotName]["color"].back());
                    }
                    break;
                }
                case SLOT_TWO_COLOR: {
                    animation["slots"][slotName]["twoColor"] = json::array();  
                    for (int frame = 0; frame < frameCount; frame++) {
                        float time = readFloat(input);
                        std::string color = readColor(input, true);
                        readByte(input); // 0xaarrggbb
                        std::string color2 = readColor(input, false);
                        animation["slots"][slotName]["twoColor"].push_back(json::object());
                        animation["slots"][slotName]["twoColor"].back()["time"] = time;
                        animation["slots"][slotName]["twoColor"].back()["light"] = color;
                        animation["slots"][slotName]["twoColor"].back()["dark"] = color2;
                        if (frame < frameCount - 1)
                            readCurve(input, animation["slots"][slotName]["twoColor"].back());
                    }
                    break;
                }
            }
        }
    }
    if (animation["slots"].empty()) animation.erase("slots");

    // Bone timelines. 
    animation["bones"] = json::object();
    for (int i = 0, n = readVarint(input, true); i < n; i++) {
        int boneIndex = readVarint(input, true);
        std::string boneName = root["bones"][boneIndex]["name"].get<std::string>();
        animation["bones"][boneName] = json::object();
        for (int ii = 0, nn = readVarint(input, true); ii < nn; ii++) {
            unsigned char timelineType = readByte(input); 
            int frameCount = readVarint(input, true); 
            switch (timelineType) {
                case BONE_ROTATE: {
                    readTimeline(input, animation["bones"][boneName]["rotate"], frameCount, 1, "angle");
                    break; 
                }
                case BONE_TRANSLATE: {
                    readTimeline(input, animation["bones"][boneName]["translate"], frameCount, 2, "x", "y");
                    break;
                }
                case BONE_SCALE: {
                    readTimeline(input, animation["bones"][boneName]["scale"], frameCount, 2, "x", "y");
                    break;
                }
                case BONE_SHEAR: {
                    readTimeline(input, animation["bones"][boneName]["shear"], frameCount, 2, "x", "y");
                    break; 
                }
            }
        }
    }
    if (animation["bones"].empty()) animation.erase("bones");

    // IK timelines. 
    animation["ik"] = json::object(); 
    for (int i = 0, n = readVarint(input, true); i < n; i++) {
        int index = readVarint(input, true); 
        std::string ikName = root["ik"][index]["name"];
        animation["ik"][ikName] = json::array();
        int frameCount = readVarint(input, true); 
        for (int frame = 0; frame < frameCount; frame++) {
            animation["ik"][ikName].push_back(json::object());
            animation["ik"][ikName].back()["time"] = readFloat(input);
            animation["ik"][ikName].back()["mix"] = readFloat(input);
            animation["ik"][ikName].back()["softness"] = readFloat(input);
            animation["ik"][ikName].back()["bendPositive"] = readSByte(input) > 0;
            animation["ik"][ikName].back()["compress"] = readBoolean(input);
            animation["ik"][ikName].back()["stretch"] = readBoolean(input);
            if (frame < frameCount - 1)
                readCurve(input, animation["ik"][ikName].back());
        }
    }
    if (animation["ik"].empty()) animation.erase("ik");

    // Transform constraint timelines. 
    animation["transform"] = json::object(); 
    for (int i = 0, n = readVarint(input, true); i < n; i++) {
        int index = readVarint(input, true); 
        std::string name = root["transform"][index]["name"];
        animation["transform"][name] = json::array();
        int frameCount = readVarint(input, true);
        for (int frame = 0; frame < frameCount; frame++) {
            animation["transform"][name].push_back(json::object());
            animation["transform"][name].back()["time"] = readFloat(input);
            animation["transform"][name].back()["rotateMix"] = readFloat(input);
            animation["transform"][name].back()["translateMix"] = readFloat(input);
            animation["transform"][name].back()["scaleMix"] = readFloat(input);
            animation["transform"][name].back()["shearMix"] = readFloat(input);
            if (frame < frameCount - 1) {
                readCurve(input, animation["transform"][name].back());
            }
        }
    }
    if (animation["transform"].empty()) animation.erase("transform");

    // Path constraint timelines. 
    animation["path"] = json::object(); 
    for (int i = 0, n = readVarint(input, true); i < n; i++) {
        int index = readVarint(input, true); 
        std::string name = root["path"][index]["name"];
        animation["path"][name] = json::object(); 
        for (int ii = 0, nn = readVarint(input, true); ii < nn; ii++) {
            int type = readSByte(input); 
            int frameCount = readVarint(input, true); 
            switch (type) {
                case PATH_POSITION: {
                    readTimeline(input, animation["path"][name]["position"], frameCount, 1, "position");
                    break; 
                }
                case PATH_SPACING: {
                    readTimeline(input, animation["path"][name]["spacing"], frameCount, 1, "spacing");
                    break; 
                }
                case PATH_MIX: {
                    readTimeline(input, animation["path"][name]["mix"], frameCount, 2, "rotateMix", "translateMix");
                    break; 
                }
            }
        }
    }
    if (animation["path"].empty()) animation.erase("path");

    // Deform timelines. 
    animation["deform"] = json::object(); 
    for (int i = 0, n = readVarint(input, true); i < n; i++) {
        int skinIndex = readVarint(input, true); 
        std::string skinName = root["skins"][skinIndex]["name"];
        animation["deform"][skinName] = json::object();
        for (int ii = 0, nn = readVarint(input, true); ii < nn; ii++) {
            int slotIndex = readVarint(input, true); 
            std::string slotName = root["slots"][slotIndex]["name"].get<std::string>();
            animation["deform"][skinName][slotName] = json::object();
            for (int iii = 0, nnn = readVarint(input, true); iii < nnn; iii++) {
                std::string attachmentName = readStringRef(input, root);
                animation["deform"][skinName][slotName][attachmentName] = json::array();
                size_t frameCount = (size_t) readVarint(input, true); 
                for (size_t frame = 0; frame < frameCount; frame++) {
                    float time = readFloat(input); 
                    animation["deform"][skinName][slotName][attachmentName].push_back(json::object());
                    animation["deform"][skinName][slotName][attachmentName].back()["time"] = time; 
                    size_t end = (size_t) readVarint(input, true); 
                    if (end != 0) {
                        size_t start = (size_t) readVarint(input, true);
                        animation["deform"][skinName][slotName][attachmentName].back()["offset"] = start; 
                        animation["deform"][skinName][slotName][attachmentName].back()["vertices"] = json::array();
                        end += start; 
                        for (size_t v = start; v < end; v++)
                            animation["deform"][skinName][slotName][attachmentName].back()["vertices"].push_back(readFloat(input));
                    }
                    if (frame < frameCount - 1)
                        readCurve(input, animation["deform"][skinName][slotName][attachmentName].back());
                }
            }
        }
    }
    if (animation["attachments"].empty()) animation.erase("attachments");

    // Draw order timeline. 
    animation["drawOrder"] = json::array(); 
    size_t drawOrderCount = (size_t) readVarint(input, true); 
    if (drawOrderCount > 0) {
        size_t slotCount = root["slots"].size();
        for (size_t i = 0; i < drawOrderCount; i++) {
            animation["drawOrder"].push_back(json::object());
            float time = readFloat(input); 
            animation["drawOrder"].back()["time"] = time;
            size_t offsetCount = (size_t) readVarint(input, true); 
            animation["drawOrder"].back()["offsets"] = json::array();
            std::vector<int> drawOrder; 
            drawOrder.resize(slotCount, -1);
            std::vector<int> unchanged; 
            unchanged.resize(slotCount - offsetCount, 0);
            size_t originalIndex = 0, unchangedIndex = 0; 
            for (size_t ii = 0; ii < offsetCount; ii++) {
                animation["drawOrder"].back()["offsets"].push_back(json::object());
                size_t slotIndex = (size_t) readVarint(input, true);
                animation["drawOrder"].back()["offsets"].back()["slot"] = root["slots"][slotIndex]["name"].get<std::string>();
                while (originalIndex != slotIndex)
                    unchanged[unchangedIndex++] = (int) originalIndex++;
                int offset = readVarint(input, true);
                animation["drawOrder"].back()["offsets"].back()["offset"] = offset;
                drawOrder[originalIndex + offset] = (int) originalIndex; 
                originalIndex++;
            }
            while (originalIndex < slotCount)
                unchanged[unchangedIndex++] = (int) originalIndex++;
            for (int ii = (int) slotCount - 1; ii >= 0; --ii)
                if (drawOrder[ii] == -1) drawOrder[ii] = unchanged[--unchangedIndex];
        }
    }
    if (animation["drawOrder"].empty()) animation.erase("drawOrder");

    // Event timeline. 
    animation["events"] = json::array();
    int eventCount = readVarint(input, true);
    if (eventCount > 0) {
        for (int i = 0; i < eventCount; i++) {
            animation["events"].push_back(json::object());
            animation["events"].back()["time"] = readFloat(input);
            animation["events"].back()["name"] = root["eventNames"][readVarint(input, true)];
            animation["events"].back()["int"] = readVarint(input, false);
            animation["events"].back()["float"] = readFloat(input);
            if (readBoolean(input))
                animation["events"].back()["string"] = readString(input);
            if (!std::string(root["events"][animation["events"].back()["name"].get<std::string>()]["audio"]).empty()) {
                animation["events"].back()["volume"] = readFloat(input);
                animation["events"].back()["balance"] = readFloat(input); 
            }
        }
    }
    if (animation["events"].empty()) animation.erase("events");

    return animation; 
}

bool skel2json38(const std::string& skelPath, const std::string& jsonPath) {
    FILE* file;
    errno_t err = fopen_s(&file, skelPath.c_str(), "rb");
    if (err != 0 || !file) return false;
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);
    std::vector<unsigned char> buffer(size);
    if (fread(buffer.data(), 1, size, file) != size) {
        fclose(file);
        return false;
    }
    fclose(file);

    DataInput input;
    input.cursor = buffer.data();
    input.end = buffer.data() + size;

    json root; 

    json skeleton; 
    skeleton["hash"] = readString(&input);
    std::string version = readString(&input);
    if (!version.starts_with("3.8")) {
        std::cout << "Skeleton version " << version << " does not match runtime version 3.8" << std::endl;
        return false; 
    }
    skeleton["spine"] = version;
    skeleton["x"] = readFloat(&input);
    skeleton["y"] = readFloat(&input);
    skeleton["width"] = readFloat(&input);
    skeleton["height"] = readFloat(&input);
    bool nonessential = readBoolean(&input);
    if (nonessential) {
        skeleton["fps"] = readFloat(&input);
        skeleton["images"] = readString(&input);
        skeleton["audio"] = readString(&input);
    }
    root["skeleton"] = skeleton;

    json strings = json::array();
    int numStrings = readVarint(&input, true);
    for (int i = 0; i < numStrings; i++) {
        strings.push_back(readString(&input));
    }
    root["strings"] = strings;

    /* Bones */
    json bones = json::array();
    int numBones = readVarint(&input, true);
    for (int i = 0; i < numBones; i++) {
        json bone; 
        bone["name"] = readString(&input);
        if (i != 0) {
            bone["parent"] = bones[readVarint(&input, true)]["name"];
        }
        bone["rotation"] = readFloat(&input);
        bone["x"] = readFloat(&input);
        bone["y"] = readFloat(&input);
        bone["scaleX"] = readFloat(&input);
        bone["scaleY"] = readFloat(&input);
        bone["shearX"] = readFloat(&input);
        bone["shearY"] = readFloat(&input);
        bone["length"] = readFloat(&input);
        bone["transform"] = TransformMode[readVarint(&input, true)];
        bone["skin"] = readBoolean(&input); 
        if (nonessential) {
            // bone["color"] = readColor(&input, true);
            /* Skip bone color. */
            readInt(&input); 
        }
        bones.push_back(bone);
    }
    if (!bones.empty())
        root["bones"] = bones;

    /* Slots */
    json slots = json::array();
    int slotCount = readVarint(&input, true);
    for (int i = 0; i < slotCount; i++) {
        json slot;
        slot["name"] = readString(&input);
        int boneIndex = readVarint(&input, true);
        slot["bone"] = bones[boneIndex]["name"];
        slot["color"] = readColor(&input, true); 
        unsigned char r = readByte(&input);
        unsigned char g = readByte(&input);
        unsigned char b = readByte(&input);
        unsigned char a = readByte(&input);
        if (!(r == 0xff && g == 0xff && b == 0xff && a == 0xff)) {
            slot["dark"] = Color2String({int(r), int(g), int(b), 255}, false);
        }
        slot["attachment"] = readStringRef(&input, root);
        slot["blend"] = BlendMode[readVarint(&input, true)];
        slots.push_back(slot);
    }
    if (!slots.empty())
        root["slots"] = slots;

    /* IK constraints */
    json ikConstraints = json::array();
    int ikConstraintsCount = readVarint(&input, true);
    for (int i = 0; i < ikConstraintsCount; i++) {
        json ikConstraint;
        ikConstraint["name"] = readString(&input);
        ikConstraint["order"] = readVarint(&input, true);
        ikConstraint["skin"] = readBoolean(&input);
        int boneCount = readVarint(&input, true);
        ikConstraint["bones"] = json::array();
        for (int j = 0; j < boneCount; j++) {
            int boneIndex = readVarint(&input, true);
            ikConstraint["bones"].push_back(bones[boneIndex]["name"]);
        }
        ikConstraint["target"] = bones[readVarint(&input, true)]["name"];
        ikConstraint["mix"] = readFloat(&input);
        ikConstraint["softness"] = readFloat(&input);
        ikConstraint["bendPositive"] = int(readSByte(&input) > 0);
        ikConstraint["compress"] = readBoolean(&input);
        ikConstraint["stretch"] = readBoolean(&input);
        ikConstraint["uniform"] = readBoolean(&input);
        ikConstraints.push_back(ikConstraint);
    }
    if (!ikConstraints.empty())
        root["ik"] = ikConstraints;

    /* Transform constraints */
    json transformConstraints = json::array();
    int transformConstraintsCount = readVarint(&input, true);
    for (int i = 0; i < transformConstraintsCount; i++) {
        json transformConstraint;
        transformConstraint["name"] = readString(&input);
        transformConstraint["order"] = readVarint(&input, true);
        transformConstraint["skin"] = readBoolean(&input);
        int boneCount = readVarint(&input, true);
        transformConstraint["bones"] = json::array();
        for (int j = 0; j < boneCount; j++) {
            int boneIndex = readVarint(&input, true);
            transformConstraint["bones"].push_back(bones[boneIndex]["name"]);
        }
        transformConstraint["target"] = bones[readVarint(&input, true)]["name"];
        transformConstraint["local"] = readBoolean(&input);
        transformConstraint["relative"] = readBoolean(&input);
        transformConstraint["rotation"] = readFloat(&input);
        transformConstraint["x"] = readFloat(&input);
        transformConstraint["y"] = readFloat(&input);
        transformConstraint["scaleX"] = readFloat(&input);
        transformConstraint["scaleY"] = readFloat(&input);
        transformConstraint["shearY"] = readFloat(&input);
        transformConstraint["rotateMix"] = readFloat(&input);
        transformConstraint["translateMix"] = readFloat(&input);
        transformConstraint["scaleMix"] = readFloat(&input);
        transformConstraint["shearMix"] = readFloat(&input);
        transformConstraints.push_back(transformConstraint);
    }
    if (!transformConstraints.empty())
        root["transform"] = transformConstraints;

    /* Path constrains */
    json pathConstraints = json::array();
    int pathConstraintsCount = readVarint(&input, true);
    for (int i = 0; i < pathConstraintsCount; ++i) {
        json pathConstraint;
        pathConstraint["name"] = readString(&input);
        pathConstraint["order"] = readVarint(&input, true);
        pathConstraint["skin"] = readBoolean(&input);
        int boneCount = readVarint(&input, true);
        pathConstraint["bones"] = json::array();
        for (int j = 0; j < boneCount; ++j) {
            int boneIndex = readVarint(&input, true);
            pathConstraint["bones"].push_back(bones[boneIndex]["name"]);
        }
        pathConstraint["target"] = slots[readVarint(&input, true)]["name"];
        pathConstraint["positionMode"] = PositionMode[readVarint(&input, true)];
        pathConstraint["spacingMode"] = SpacingMode[readVarint(&input, true)];
        pathConstraint["rotateMode"] = RotateMode[readVarint(&input, true)];
        pathConstraint["rotation"] = readFloat(&input);
        pathConstraint["position"] = readFloat(&input);
        pathConstraint["spacing"] = readFloat(&input);
        pathConstraint["rotateMix"] = readFloat(&input);
        pathConstraint["translateMix"] = readFloat(&input);
        pathConstraints.push_back(pathConstraint);
    }
    if (!pathConstraints.empty())
        root["path"] = pathConstraints;

    /* Skins */
    json skins = json::array(); 
    json defaultSkin = readSkin(&input, true, root, nonessential);
    skins.push_back(defaultSkin);
    for (size_t i = 0, n = (size_t)readVarint(&input, true); i < n; i++) {
        json skin = readSkin(&input, false, root, nonessential);
        skins.push_back(skin);
    }
    if (!skins.empty())
        root["skins"] = skins;

    /* Events */
    json eventNames = json::array(); 
    json events = json::object(); 
    int eventsCount = readVarint(&input, true);
    for (int i = 0; i < eventsCount; i++) {
        std::string name = readStringRef(&input, root);
        eventNames.push_back(name);
        json event = json::object();
        event["int"] = readVarint(&input, false); 
        event["float"] = readFloat(&input); 
        event["string"] = readString(&input); 
        event["audio"] = readString(&input); 
        if (!std::string(event["audio"]).empty()) {
            event["volume"] = readFloat(&input); 
            event["balance"] = readFloat(&input); 
        }
        events[name] = event;
    }
    if (!events.empty())
        root["events"] = events;
    root["eventNames"] = eventNames;

    /* Animations */
    json animations = json::object(); 
    int animationsCount = readVarint(&input, true); 
    for (int i = 0; i < animationsCount; i++) {
        std::string name = readString(&input);
        json animation = readAnimation(name, &input, root);
        animations[name] = animation;
    }
    if (!animations.empty())
        root["animations"] = animations;

    /* Adjustment */
    root.erase("strings"); 
    root.erase("eventNames");

    // 写入JSON文件
    std::ofstream outputFile(jsonPath);
    if (!outputFile) {
        std::cerr << "Failed to open output file: " << jsonPath << std::endl;
        return false;
    }
    outputFile << customDump(root) << std::endl;
    outputFile.close();

    return true;
}

} // namespace spine38
