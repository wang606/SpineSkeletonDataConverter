#include "SkeletonData.h"

namespace spine40 {

struct DataInput {
    const unsigned char* cursor;
    const unsigned char* end;
};

unsigned char readByte(DataInput* input) {
    return *input->cursor++;
}

signed char readSByte(DataInput* input) {
    return (signed char)readByte(input);
}

bool readBoolean(DataInput* input) {
    return readByte(input) != 0;
}

int readInt(DataInput* input) {
    int result = readByte(input);
    result <<= 8;
    result |= readByte(input);
    result <<= 8;
    result |= readByte(input);
    result <<= 8;
    result |= readByte(input);
    return result;
}

Color readColor(DataInput* input, bool hasAlpha = true) {
    Color color; 
    color.r = readByte(input); 
    color.g = readByte(input); 
    color.b = readByte(input); 
    color.a = hasAlpha ? readByte(input) : 255; 
    return color; 
}

int readVarint(DataInput* input, bool optimizePositive) {
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
    if (!optimizePositive) value = (((unsigned int)value >> 1) ^ -(value & 1));
    return value;
}

float readFloat(DataInput* input) {
    union {
        int intValue; 
        float floatValue; 
    } intToFloat; 
    intToFloat.intValue = readInt(input);
    return intToFloat.floatValue;
}

OptStr readString(DataInput* input) {
    int length = readVarint(input, true); 
    if (length == 0) return std::nullopt;
    std::string string; 
    string.resize(length - 1); 
    memcpy(string.data(), input->cursor, length - 1); 
    input->cursor += length - 1; 
    return string; 
}

OptStr readStringRef(DataInput* input, SkeletonData* skeletonData) {
    int index = readVarint(input, true); 
    if (index == 0) return std::nullopt; 
    else return skeletonData->strings[index - 1]; 
}

void readFloatArray(DataInput* input, int n, std::vector<float>& array) {
    array.resize(n, 0);
    for (int i = 0; i < n; i++)
        array[i] = readFloat(input);
}

void readShortArray(DataInput* input, std::vector<unsigned short>& array) {
    int n = readVarint(input, true);
    array.resize(n, 0);
    for (int i = 0; i < n; i++) {
        array[i] = readByte(input) << 8; 
        array[i] |= readByte(input);
    }
}

void readVertices(DataInput* input, std::vector<float>& vertices, int vertexCount) {
    if (!readBoolean(input)) {
        readFloatArray(input, vertexCount << 1, vertices);
    } else {
        for (int i = 0; i < vertexCount; i++) {
            int boneCount = readVarint(input, true); 
            vertices.push_back(boneCount);
            for (int ii = 0; ii < boneCount; ii++) {
                vertices.push_back(readVarint(input, true));
                vertices.push_back(readFloat(input));
                vertices.push_back(readFloat(input));
                vertices.push_back(readFloat(input));
            }
        }
    }
}

void readCurve(DataInput* input, TimelineFrame& frame, int timelineCount) {
    for (int i = 0; i < timelineCount * 4; i++) {
        frame.curve.push_back(readFloat(input));
    }
}

Timeline readTimeline(DataInput* input, int frameCount, int valueNum) {
    Timeline timeline;
    float time = readFloat(input);
    float value1 = readFloat(input);
    float value2 = valueNum > 1 ? readFloat(input) : 0.0f;
    float value3 = valueNum > 2 ? readFloat(input) : 0.0f;
    for (int frameIndex = 0; ; frameIndex++) {
        TimelineFrame frame; 
        frame.time = time; 
        frame.value1 = value1; 
        if (valueNum > 1) frame.value2 = value2; 
        if (valueNum > 2) frame.value3 = value3;
        if (frameIndex == frameCount - 1) {
            timeline.push_back(frame);
            break;
        }
        time = readFloat(input);
        value1 = readFloat(input);
        if (valueNum > 1) value2 = readFloat(input);
        if (valueNum > 2) value3 = readFloat(input);
        switch (readSByte(input)) {
            case CURVE_STEPPED: {
                frame.curveType = CurveType::CURVE_STEPPED;
                break; 
            }
            case CURVE_BEZIER: {
                frame.curveType = CurveType::CURVE_BEZIER; 
                readCurve(input, frame, valueNum);
                break; 
            }
        }
        timeline.push_back(frame);
    }
    return timeline;
}

Skin readSkin(DataInput* input, bool defaultSkin, SkeletonData* skeletonData) {
    Skin skin; 
    int slotCount = 0; 
    if (defaultSkin) {
        slotCount = readVarint(input, true);
        skin.name = "default";
    } else {
        skin.name = readStringRef(input, skeletonData).value();
        for (int i = 0, n = readVarint(input, true); i < n; i++) {
            skin.bones.push_back(skeletonData->bones[readVarint(input, true)].name.value());
        }
        for (int i = 0, n = readVarint(input, true); i < n; i++) {
            skin.ik.push_back(skeletonData->ikConstraints[readVarint(input, true)].name.value());
        }
        for (int i = 0, n = readVarint(input, true); i < n; i++) {
            skin.transform.push_back(skeletonData->transformConstraints[readVarint(input, true)].name.value());
        }
        for (int i = 0, n = readVarint(input, true); i < n; i++) {
            skin.path.push_back(skeletonData->pathConstraints[readVarint(input, true)].name.value());
        }
        slotCount = readVarint(input, true);
    }
    for (int i = 0; i < slotCount; i++) {
        std::string slotName = skeletonData->slots[readVarint(input, true)].name.value();
        for (int ii = 0, nn = readVarint(input, true); ii < nn; ii++) {
            std::string attachmentName = readStringRef(input, skeletonData).value();
            Attachment attachment;
            auto name = readStringRef(input, skeletonData);
            attachment.name = (name.has_value() && !name->empty()) ? name.value() : attachmentName;
            attachment.type = static_cast<AttachmentType>(readByte(input));
            switch (attachment.type) {
                case AttachmentType_Region: {
                    RegionAttachment region; 
                    auto path = readStringRef(input, skeletonData);
                    attachment.path = (path.has_value() && !path->empty()) ? path.value() : attachment.name;
                    region.rotation = readFloat(input);
                    region.x = readFloat(input);
                    region.y = readFloat(input);
                    region.scaleX = readFloat(input);
                    region.scaleY = readFloat(input);
                    region.width = readFloat(input);
                    region.height = readFloat(input);
                    Color color = readColor(input);
                    if (color != Color{0xff, 0xff, 0xff, 0xff}) region.color = color;
                    attachment.data = region;
                    break; 
                }
                case AttachmentType_Boundingbox: {
                    BoundingboxAttachment box;
                    attachment.path = attachment.name; 
                    box.vertexCount = readVarint(input, true);
                    readVertices(input, box.vertices, box.vertexCount);
                    if (skeletonData->nonessential) {
                        Color color = readColor(input);
                        if (color != Color{0xff, 0xff, 0xff, 0xff}) box.color = color;
                    }
                    attachment.data = box;
                    break; 
                }
                case AttachmentType_Mesh: {
                    MeshAttachment mesh; 
                    auto path = readStringRef(input, skeletonData);
                    attachment.path = (path.has_value() && !path->empty()) ? path.value() : attachment.name;
                    Color color = readColor(input);
                    if (color != Color{0xff, 0xff, 0xff, 0xff}) mesh.color = color;
                    int vertexCount = readVarint(input, true);
                    readFloatArray(input, vertexCount << 1, mesh.uvs);
                    readShortArray(input, mesh.triangles);
                    readVertices(input, mesh.vertices, vertexCount);
                    mesh.hullLength = readVarint(input, true);
                    if (skeletonData->nonessential) {
                        readShortArray(input, mesh.edges);
                        mesh.width = readFloat(input);
                        mesh.height = readFloat(input);
                    }
                    attachment.data = mesh;
                    break; 
                }
                case AttachmentType_Linkedmesh: {
                    LinkedmeshAttachment linkedMesh; 
                    auto path = readStringRef(input, skeletonData);
                    attachment.path = (path.has_value() && !path->empty()) ? path.value() : attachment.name;
                    Color color = readColor(input);
                    if (color != Color{0xff, 0xff, 0xff, 0xff}) linkedMesh.color = color;
                    linkedMesh.skin = readStringRef(input, skeletonData);
                    linkedMesh.parentMesh = readStringRef(input, skeletonData).value();
                    linkedMesh.timelines = readBoolean(input) ? 1 : 0;
                    if (skeletonData->nonessential) {
                        linkedMesh.width = readFloat(input);
                        linkedMesh.height = readFloat(input);
                    }
                    attachment.data = linkedMesh;
                    break; 
                }
                case AttachmentType_Path: {
                    PathAttachment path; 
                    attachment.path = attachment.name; 
                    path.closed = readBoolean(input);
                    path.constantSpeed = readBoolean(input);
                    path.vertexCount = readVarint(input, true);
                    readVertices(input, path.vertices, path.vertexCount);
                    readFloatArray(input, path.vertexCount / 3, path.lengths);
                    if (skeletonData->nonessential) {
                        Color color = readColor(input);
                        if (color != Color{0xff, 0xff, 0xff, 0xff}) path.color = color;
                    }
                    attachment.data = path;
                    break; 
                }
                case AttachmentType_Point: {
                    PointAttachment point; 
                    attachment.path = attachment.name; 
                    point.rotation = readFloat(input);
                    point.x = readFloat(input);
                    point.y = readFloat(input);
                    if (skeletonData->nonessential) {
                        Color color = readColor(input);
                        if (color != Color{0xff, 0xff, 0xff, 0xff}) point.color = color;
                    }
                    attachment.data = point;
                    break; 
                }
                case AttachmentType_Clipping: {
                    ClippingAttachment clipping; 
                    attachment.path = attachment.name; 
                    clipping.endSlot = skeletonData->slots[readVarint(input, true)].name;
                    clipping.vertexCount = readVarint(input, true);
                    readVertices(input, clipping.vertices, clipping.vertexCount);
                    if (skeletonData->nonessential) {
                        Color color = readColor(input);
                        if (color != Color{0xff, 0xff, 0xff, 0xff}) clipping.color = color;
                    }
                    attachment.data = clipping;
                    break;
                }
            }
            skin.attachments[slotName][attachmentName] = attachment;
        }
    }
    return skin;
}

Animation readAnimation(DataInput* input, SkeletonData* skeletonData) {
    Animation animation; 
    animation.name = readString(input).value();
    int numTimelines = readVarint(input, true);
    for (int i = 0, n = readVarint(input, true); i < n; i++) {
        std::string slotName = skeletonData->slots[readVarint(input, true)].name.value();
        MultiTimeline slotTimeline; 
        for (int ii = 0, nn = readVarint(input, true); ii < nn; ii++) {
            SlotTimelineType timelineType = static_cast<SlotTimelineType>(readByte(input));
            int frameCount = readVarint(input, true);
            switch (timelineType) {
                case SlotTimelineType::SLOT_ATTACHMENT: {
                    Timeline timeline;
                    for (int frameIndex = 0; frameIndex < frameCount; frameIndex++) {
                        TimelineFrame frame; 
                        frame.time = readFloat(input);
                        frame.str1 = readStringRef(input, skeletonData);
                        timeline.push_back(frame);
                    }
                    slotTimeline["attachment"] = timeline;
                    break; 
                }
                case SlotTimelineType::SLOT_RGBA: {
                    Timeline timeline;
                    int bezierCount = readVarint(input, true);
                    float time = readFloat(input);
                    Color color = readColor(input);
                    for (int frameIndex = 0; ; frameIndex++) {
                        TimelineFrame frame; 
                        frame.time = time; 
                        frame.color1 = color; 
                        if (frameIndex == frameCount - 1) {
                            timeline.push_back(frame);
                            break;
                        }
                        time = readFloat(input);
                        color = readColor(input);
                        switch (readSByte(input)) {
                            case CURVE_STEPPED: {
                                frame.curveType = CurveType::CURVE_STEPPED;
                                break; 
                            }
                            case CURVE_BEZIER: {
                                frame.curveType = CurveType::CURVE_BEZIER;
                                readCurve(input, frame, 4);
                                break; 
                            }
                        }
                        timeline.push_back(frame);
                    }
                    slotTimeline["rgba"] = timeline;
                    break; 
                }
                case SlotTimelineType::SLOT_RGB: {
                    Timeline timeline;
                    int bezierCount = readVarint(input, true);
                    float time = readFloat(input);
                    Color color = readColor(input, false);
                    for (int frameIndex = 0; ; frameIndex++) {
                        TimelineFrame frame; 
                        frame.time = time; 
                        frame.color1 = color; 
                        if (frameIndex == frameCount - 1) {
                            timeline.push_back(frame);
                            break;
                        }
                        time = readFloat(input);
                        color = readColor(input, false);
                        switch (readSByte(input)) {
                            case CURVE_STEPPED: {
                                frame.curveType = CurveType::CURVE_STEPPED;
                                break; 
                            }
                            case CURVE_BEZIER: {
                                frame.curveType = CurveType::CURVE_BEZIER;
                                readCurve(input, frame, 3);
                                break; 
                            }
                        }
                        timeline.push_back(frame);
                    }
                    slotTimeline["rgb"] = timeline;
                    break; 
                }
                case SlotTimelineType::SLOT_RGBA2: {
                    Timeline timeline;
                    int bezierCount = readVarint(input, true);
                    float time = readFloat(input);
                    Color light = readColor(input);
                    Color dark = readColor(input, false);
                    for (int frameIndex = 0; ; frameIndex++) {
                        TimelineFrame frame; 
                        frame.time = time; 
                        frame.color1 = light; 
                        frame.color2 = dark; 
                        if (frameIndex == frameCount - 1) {
                            timeline.push_back(frame);
                            break;
                        }
                        time = readFloat(input);
                        light = readColor(input);
                        dark = readColor(input, false);
                        switch (readSByte(input)) {
                            case CURVE_STEPPED: {
                                frame.curveType = CurveType::CURVE_STEPPED;
                                break; 
                            }
                            case CURVE_BEZIER: {
                                frame.curveType = CurveType::CURVE_BEZIER;
                                readCurve(input, frame, 7);
                                break; 
                            }
                        }
                        timeline.push_back(frame);
                    }
                    slotTimeline["rgba2"] = timeline;
                    break; 
                }
                case SlotTimelineType::SLOT_RGB2: {
                    Timeline timeline;
                    int bezierCount = readVarint(input, true);
                    float time = readFloat(input);
                    Color light = readColor(input, false);
                    Color dark = readColor(input, false);
                    for (int frameIndex = 0; ; frameIndex++) {
                        TimelineFrame frame; 
                        frame.time = time; 
                        frame.color1 = light; 
                        frame.color2 = dark; 
                        if (frameIndex == frameCount - 1) {
                            timeline.push_back(frame);
                            break;
                        }
                        time = readFloat(input);
                        light = readColor(input, false);
                        dark = readColor(input, false);
                        switch (readSByte(input)) {
                            case CURVE_STEPPED: {
                                frame.curveType = CurveType::CURVE_STEPPED;
                                break; 
                            }
                            case CURVE_BEZIER: {
                                frame.curveType = CurveType::CURVE_BEZIER;
                                readCurve(input, frame, 6);
                                break; 
                            }
                        }
                        timeline.push_back(frame);
                    }
                    slotTimeline["rgb2"] = timeline;
                    break; 
                }
                case SlotTimelineType::SLOT_ALPHA: {
                    Timeline timeline;
                    int bezierCount = readVarint(input, true);
                    float time = readFloat(input);
                    float alpha = readByte(input) / 255.0f; 
                    for (int frameIndex = 0; ; frameIndex++) {
                        TimelineFrame frame; 
                        frame.time = time; 
                        frame.value1 = alpha; 
                        if (frameIndex == frameCount - 1) {
                            timeline.push_back(frame);
                            break;
                        }
                        time = readFloat(input);
                        alpha = readByte(input) / 255.0f; 
                        switch (readSByte(input)) {
                            case CURVE_STEPPED: {
                                frame.curveType = CurveType::CURVE_STEPPED;
                                break; 
                            }
                            case CURVE_BEZIER: {
                                frame.curveType = CurveType::CURVE_BEZIER;
                                readCurve(input, frame, 1);
                                break; 
                            }
                        }
                        timeline.push_back(frame);
                    }
                    slotTimeline["alpha"] = timeline;
                    break;
                }
            }
        }
        animation.slots[slotName] = slotTimeline;
    }
    for (int i = 0, n = readVarint(input, true); i < n; i++) {
        std::string boneName = skeletonData->bones[readVarint(input, true)].name.value();
        MultiTimeline boneTimeline;
        for (int ii = 0, nn = readVarint(input, true); ii < nn; ii++) {
            BoneTimelineType timelineType = static_cast<BoneTimelineType>(readByte(input));
            int frameCount = readVarint(input, true);
            int bezierCount = readVarint(input, true);
            switch (timelineType) {
                case BONE_ROTATE: {
                    boneTimeline["rotate"] = readTimeline(input, frameCount, 1);
                    break;
                }
                case BONE_TRANSLATE: {
                    boneTimeline["translate"] = readTimeline(input, frameCount, 2);
                    break;
                }
                case BONE_TRANSLATEX: {
                    boneTimeline["translatex"] = readTimeline(input, frameCount, 1);
                    break;
                }
                case BONE_TRANSLATEY: {
                    boneTimeline["translatey"] = readTimeline(input, frameCount, 1);
                    break;
                }
                case BONE_SCALE: {
                    boneTimeline["scale"] = readTimeline(input, frameCount, 2);
                    break;
                }
                case BONE_SCALEX: {
                    boneTimeline["scalex"] = readTimeline(input, frameCount, 1);
                    break;
                }
                case BONE_SCALEY: {
                    boneTimeline["scaley"] = readTimeline(input, frameCount, 1);
                    break;
                }
                case BONE_SHEAR: {
                    boneTimeline["shear"] = readTimeline(input, frameCount, 2);
                    break;
                }
                case BONE_SHEARX: {
                    boneTimeline["shearx"] = readTimeline(input, frameCount, 1);
                    break;
                }
                case BONE_SHEARY: {
                    boneTimeline["sheary"] = readTimeline(input, frameCount, 1);
                    break;
                }
            }
        }
        animation.bones[boneName] = boneTimeline;
    }
    for (int i = 0, n = readVarint(input, true); i < n; i++) {
        std::string ikName = skeletonData->ikConstraints[readVarint(input, true)].name.value();
        int frameCount = readVarint(input, true);
        int bezierCount = readVarint(input, true);
        Timeline timeline;
        float time = readFloat(input);
        float mix = readFloat(input);
        float softness = readFloat(input);
        for (int frameIndex = 0; ; frameIndex++) {
            TimelineFrame frame; 
            frame.time = time; 
            frame.value1 = mix; 
            frame.value2 = softness; 
            frame.bendPositive = readSByte(input) > 0;
            frame.compress = readBoolean(input);
            frame.stretch = readBoolean(input);
            if (frameIndex == frameCount - 1) {
                timeline.push_back(frame);
                break;
            }
            time = readFloat(input);
            mix = readFloat(input);
            softness = readFloat(input);
            switch (readSByte(input)) {
                case CURVE_STEPPED: {
                    frame.curveType = CurveType::CURVE_STEPPED;
                    break;
                }
                case CURVE_BEZIER: {
                    frame.curveType = CurveType::CURVE_BEZIER;
                    readCurve(input, frame, 2);
                    break;
                }
            }
            timeline.push_back(frame);
        }
        animation.ik[ikName] = timeline;
    }
    for (int i = 0, n = readVarint(input, true); i < n; i++) {
        std::string transformName = skeletonData->transformConstraints[readVarint(input, true)].name.value();
        int frameCount = readVarint(input, true);
        int bezierCount = readVarint(input, true);
        Timeline timeline;
        float time = readFloat(input);
        float mixRotate = readFloat(input);
        float mixX = readFloat(input);
        float mixY = readFloat(input);
        float mixScaleX = readFloat(input);
        float mixScaleY = readFloat(input);
        float mixShearY = readFloat(input);
        for (int frameIndex = 0; ; frameIndex++) {
            TimelineFrame frame; 
            frame.time = time; 
            frame.value1 = mixRotate; 
            frame.value2 = mixX; 
            frame.value3 = mixY; 
            frame.value4 = mixScaleX; 
            frame.value5 = mixScaleY; 
            frame.value6 = mixShearY; 
            if (frameIndex == frameCount - 1) {
                timeline.push_back(frame);
                break;
            }
            time = readFloat(input);
            mixRotate = readFloat(input);
            mixX = readFloat(input);
            mixY = readFloat(input);
            mixScaleX = readFloat(input);
            mixScaleY = readFloat(input);
            mixShearY = readFloat(input);
            switch (readSByte(input)) {
                case CURVE_STEPPED: {
                    frame.curveType = CurveType::CURVE_STEPPED;
                    break; 
                }
                case CURVE_BEZIER: {
                    frame.curveType = CurveType::CURVE_BEZIER;
                    readCurve(input, frame, 6);
                    break; 
                }
            }
            timeline.push_back(frame);
        }
        animation.transform[transformName] = timeline;
    }
    for (int i = 0, n = readVarint(input, true); i < n; i++) {
        std::string pathName = skeletonData->pathConstraints[readVarint(input, true)].name.value();
        MultiTimeline pathTimeline; 
        for (int ii = 0, nn = readVarint(input, true); ii < nn; ii++) {
            PathTimelineType timelineType = static_cast<PathTimelineType>(readSByte(input));
            int frameCount = readVarint(input, true);
            int bezierCount = readVarint(input, true);
            switch (timelineType) {
                case PATH_POSITION: {
                    pathTimeline["position"] = readTimeline(input, frameCount, 1);
                    break;
                }
                case PATH_SPACING: {
                    pathTimeline["spacing"] = readTimeline(input, frameCount, 1);
                    break;
                }
                case PATH_MIX: {
                    pathTimeline["mix"] = readTimeline(input, frameCount, 3);
                    break; 
                }
            }
        }
        animation.path[pathName] = pathTimeline;
    }
    for (int i = 0, n = readVarint(input, true); i < n; i++) {
        std::string skinName = skeletonData->skins[readVarint(input, true)].name; 
        for (int ii = 0, nn = readVarint(input, true); ii < nn; ii++) {
            std::string slotName = skeletonData->slots[readVarint(input, true)].name.value();
            for (int iii = 0, nnn = readVarint(input, true); iii < nnn; iii++) {
                std::string attachmentName = readStringRef(input, skeletonData).value();
                Timeline attachmentTimeline;
                int frameCount = readVarint(input, true);
                int bezierCount = readVarint(input, true);
                float time = readFloat(input);
                for (int frameIndex = 0; ; frameIndex++) {
                    TimelineFrame frame; 
                    frame.time = time; 
                    size_t end = (size_t) readVarint(input, true);
                    if (end != 0) {
                        size_t start = (size_t) readVarint(input, true);
                        frame.int1 = start;
                        end += start;
                        for (size_t v = start; v < end; v++)
                            frame.vertices.push_back(readFloat(input));
                    }
                    if (frameIndex == frameCount - 1) {
                        attachmentTimeline.push_back(frame);
                        break; 
                    }
                    time = readFloat(input);
                    switch (readSByte(input)) {
                        case CURVE_STEPPED: {
                            frame.curveType = CurveType::CURVE_STEPPED;
                            break; 
                        }
                        case CURVE_BEZIER: {
                            frame.curveType = CurveType::CURVE_BEZIER;
                            readCurve(input, frame, 1);
                            break;
                        }
                    }
                    attachmentTimeline.push_back(frame);
                }
                animation.attachments[skinName][slotName][attachmentName]["deform"] = attachmentTimeline;
            }
        }
    }
    size_t drawOrderCount = (size_t) readVarint(input, true);
    for (size_t i = 0; i < drawOrderCount; i++) {
        TimelineFrame frame; 
        frame.time = readFloat(input);
        size_t offsetCount = (size_t) readVarint(input, true);
        for (size_t ii = 0; ii < offsetCount; ii++) {
            frame.offsets.push_back({
                skeletonData->slots[readVarint(input, true)].name.value(),
                readVarint(input, true)
            });
        }
        animation.drawOrder.push_back(frame);
    }
    int eventCount = readVarint(input, true);
    for (int i = 0; i < eventCount; i++) {
        TimelineFrame frame; 
        frame.time = readFloat(input);
        int eventIndex = readVarint(input, true);
        const EventData& eventData = skeletonData->events[eventIndex];
        frame.str1 = eventData.name;
        frame.int1 = readVarint(input, false);
        frame.value1 = readFloat(input);
        bool freeString = readBoolean(input);
        frame.str2 = freeString ? readString(input) : eventData.stringValue;
        if (eventData.audioPath && !eventData.audioPath->empty()) {
            frame.value2 = readFloat(input);
            frame.value3 = readFloat(input);
        }
        animation.events.push_back(frame);
    }
    return animation; 
}

SkeletonData readBinaryData(const Binary& binary) {
    SkeletonData skeletonData;
    DataInput input; 
    input.cursor = binary.data(); 
    input.end = binary.data() + binary.size();

    uint64_t lowHash = (uint64_t) readInt(&input); 
    uint64_t highHash = (uint64_t) readInt(&input);
    skeletonData.hash = highHash << 32 | (lowHash & 0xffffffff);
    skeletonData.version = readString(&input).value();

    skeletonData.x = readFloat(&input); 
    skeletonData.y = readFloat(&input);
    skeletonData.width = readFloat(&input);
    skeletonData.height = readFloat(&input);

    skeletonData.nonessential = readBoolean(&input); 

    if (skeletonData.nonessential) {
        skeletonData.fps = readFloat(&input); 
        skeletonData.imagesPath = readString(&input).value(); 
        skeletonData.audioPath = readString(&input).value();
    }

    int numStrings = readVarint(&input, true);
    for (int i = 0; i < numStrings; i++)
        skeletonData.strings.push_back(readString(&input).value());
    
    /* Bones */
    int numBones = readVarint(&input, true);
    for (int i = 0; i < numBones; i++) {
        BoneData boneData; 
        boneData.name = readString(&input); 
        if (i != 0) boneData.parent = skeletonData.bones[readVarint(&input, true)].name;
        boneData.rotation = readFloat(&input);
        boneData.x = readFloat(&input);
        boneData.y = readFloat(&input);
        boneData.scaleX = readFloat(&input);
        boneData.scaleY = readFloat(&input);
        boneData.shearX = readFloat(&input);
        boneData.shearY = readFloat(&input);
        boneData.length = readFloat(&input);
        boneData.inherit = static_cast<Inherit>(readVarint(&input, true));
        boneData.skinRequired = readBoolean(&input);
        if (skeletonData.nonessential) {
            Color color = readColor(&input);
            if (color != Color{0x9b, 0x9b, 0x9b, 0xff}) boneData.color = color; 
        }
        skeletonData.bones.push_back(boneData);
    }

    /* Slots */
    int slotCount = readVarint(&input, true);
    for (int i = 0; i < slotCount; i++) {
        SlotData slotData; 
        slotData.name = readString(&input);
        slotData.bone = skeletonData.bones[readVarint(&input, true)].name;
        Color color = readColor(&input);
        if (color != Color{0xff, 0xff, 0xff, 0xff}) slotData.color = color;
        unsigned char a = readByte(&input);
        unsigned char r = readByte(&input);
        unsigned char g = readByte(&input);
        unsigned char b = readByte(&input);
        if (!(r == 0xff && g == 0xff && b == 0xff && a == 0xff)) {
            slotData.darkColor = Color{ r, g, b, a }; 
        }
        slotData.attachmentName = readStringRef(&input, &skeletonData);
        slotData.blendMode = static_cast<BlendMode>(readVarint(&input, true));
        skeletonData.slots.push_back(slotData);
    }

    /* IK constraints */
    int ikConstraintsCount = readVarint(&input, true);
    for (int i = 0; i < ikConstraintsCount; i++) {
        IKConstraintData ikData; 
        ikData.name = readString(&input);
        ikData.order = readVarint(&input, true);
        ikData.skinRequired = readBoolean(&input);
        int bonesCount = readVarint(&input, true);
        for (int ii = 0; ii < bonesCount; ii++)
            ikData.bones.push_back(skeletonData.bones[readVarint(&input, true)].name.value());
        ikData.target = skeletonData.bones[readVarint(&input, true)].name;
        ikData.mix = readFloat(&input);
        ikData.softness = readFloat(&input);
        ikData.bendPositive = readSByte(&input) > 0;
        ikData.compress = readBoolean(&input);
        ikData.stretch = readBoolean(&input);
        ikData.uniform = readBoolean(&input);
        skeletonData.ikConstraints.push_back(ikData);
    }

    /* Transform constraints */
    int transformConstraintsCount = readVarint(&input, true);
    for (int i = 0; i < transformConstraintsCount; i++) {
        TransformConstraintData transformData;
        transformData.name = readString(&input);
        transformData.order = readVarint(&input, true);
        transformData.skinRequired = readBoolean(&input);
        int bonesCount = readVarint(&input, true);
        for (int ii = 0; ii < bonesCount; ii++)
            transformData.bones.push_back(skeletonData.bones[readVarint(&input, true)].name.value());
        transformData.target = skeletonData.bones[readVarint(&input, true)].name;
        transformData.local = readBoolean(&input);
        transformData.relative = readBoolean(&input);
        transformData.offsetRotation = readFloat(&input);
        transformData.offsetX = readFloat(&input);
        transformData.offsetY = readFloat(&input);
        transformData.offsetScaleX = readFloat(&input);
        transformData.offsetScaleY = readFloat(&input);
        transformData.offsetShearY = readFloat(&input);
        transformData.mixRotate = readFloat(&input);
        transformData.mixX = readFloat(&input);
        transformData.mixY = readFloat(&input);
        transformData.mixScaleX = readFloat(&input);
        transformData.mixScaleY = readFloat(&input);
        transformData.mixShearY = readFloat(&input);
        skeletonData.transformConstraints.push_back(transformData);
    }

    /* Path constraints */
    int pathConstraintsCount = readVarint(&input, true);
    for (int i = 0; i < pathConstraintsCount; i++) {
        PathConstraintData pathData; 
        pathData.name = readString(&input);
        pathData.order = readVarint(&input, true);
        pathData.skinRequired = readBoolean(&input);
        int bonesCount = readVarint(&input, true);
        for (int ii = 0; ii < bonesCount; ii++)
            pathData.bones.push_back(skeletonData.bones[readVarint(&input, true)].name.value());
        pathData.target = skeletonData.slots[readVarint(&input, true)].name;
        pathData.positionMode = static_cast<PositionMode>(readVarint(&input, true));
        pathData.spacingMode = static_cast<SpacingMode>(readVarint(&input, true));
        pathData.rotateMode = static_cast<RotateMode>(readVarint(&input, true));
        pathData.offsetRotation = readFloat(&input);
        pathData.position = readFloat(&input);
        pathData.spacing = readFloat(&input);
        pathData.mixRotate = readFloat(&input);
        pathData.mixX = readFloat(&input);
        pathData.mixY = readFloat(&input);
        skeletonData.pathConstraints.push_back(pathData);
    }

    /* Skins */
    skeletonData.skins.push_back(readSkin(&input, true, &skeletonData));
    int skinCount = readVarint(&input, true);
    for (int i = 0; i < skinCount; i++) {
        skeletonData.skins.push_back(readSkin(&input, false, &skeletonData));
    }

    /* Events */
    int eventCount = readVarint(&input, true);
    for (int i = 0; i < eventCount; i++) {
        EventData eventData; 
        eventData.name = readStringRef(&input, &skeletonData).value();
        eventData.intValue = readVarint(&input, false);
        eventData.floatValue = readFloat(&input);
        eventData.stringValue = readString(&input);
        eventData.audioPath = readString(&input);
        if (eventData.audioPath && eventData.audioPath->length() > 0) {
            eventData.volume = readFloat(&input);
            eventData.balance = readFloat(&input);
        }
        skeletonData.events.push_back(eventData);
    }

    /* Animations */
    int animationCount = readVarint(&input, true);
    for (int i = 0; i < animationCount; i++) {
        Animation animation = readAnimation(&input, &skeletonData);
        skeletonData.animations.push_back(animation);
    }

    return skeletonData;
}

}
