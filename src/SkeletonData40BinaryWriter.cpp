#include "SkeletonData.h"
#include <set>

namespace spine40 {

void writeFloatArray(Binary& binary, const std::vector<float>& array) {
    for (float value : array) {
        writeFloat(binary, value);
    }
}

void writeShortArray(Binary& binary, const std::vector<unsigned short>& array) {
    writeVarint(binary, array.size(), true);
    for (unsigned short value : array) {
        writeByte(binary, value >> 8);
        writeByte(binary, value & 0xff);
    }
}

void writeVertices(Binary& binary, const std::vector<float>& vertices, bool weighted) {
    if (!weighted) {
        writeBoolean(binary, false);
        writeFloatArray(binary, vertices);
    } else {
        writeBoolean(binary, true);
        int verticesIdx = 0; 
        while (verticesIdx < vertices.size()) {
            int boneCount = (int)vertices[verticesIdx++];
            writeVarint(binary, boneCount, true);
            for (int ii = 0; ii < boneCount; ii++) {
                writeVarint(binary, (int)vertices[verticesIdx++], true);
                writeFloat(binary, vertices[verticesIdx++]);
                writeFloat(binary, vertices[verticesIdx++]);
                writeFloat(binary, vertices[verticesIdx++]);
            }
        }
    }
}

void writeCurve(Binary& binary, const TimelineFrame& frame) {
    for (int i = 0; i < frame.curve.size(); i++) {
        writeFloat(binary, frame.curve[i]);
    }
}

void writeTimeline(Binary& binary, const Timeline& timeline, int valueNum) {
    writeFloat(binary, timeline[0].time); 
    writeFloat(binary, timeline[0].value1);
    if (valueNum > 1) writeFloat(binary, timeline[0].value2);
    if (valueNum > 2) writeFloat(binary, timeline[0].value3);
    for (int frameIndex = 1; frameIndex < timeline.size(); frameIndex++) {
        writeFloat(binary, timeline[frameIndex].time); 
        writeFloat(binary, timeline[frameIndex].value1);
        if (valueNum > 1) writeFloat(binary, timeline[frameIndex].value2);
        if (valueNum > 2) writeFloat(binary, timeline[frameIndex].value3);
        CurveType curveType = timeline[frameIndex - 1].curveType;
        writeSByte(binary, (signed char)curveType);
        if (curveType == CurveType::CURVE_BEZIER) {
            writeCurve(binary, timeline[frameIndex - 1]);
        }
    }
}

void writeSkin(Binary& binary, const Skin& skin, const SkeletonData& skeletonData, bool defaultSkin) {
    if (defaultSkin) {
        writeVarint(binary, skin.attachments.size(), true); 
    } else {
        writeStringRef(binary, skin.name, skeletonData); 
        writeVarint(binary, skin.bones.size(), true);
        for (const std::string& boneName : skin.bones) {
            int boneIndex = 0;
            for (size_t i = 0; i < skeletonData.bones.size(); i++) {
                if (skeletonData.bones[i].name && *skeletonData.bones[i].name == boneName) {
                    boneIndex = i;
                    break;
                }
            }
            writeVarint(binary, boneIndex, true);
        }
        writeVarint(binary, skin.ik.size(), true);
        for (const std::string& ikName : skin.ik) {
            int ikIndex = 0;
            for (size_t i = 0; i < skeletonData.ikConstraints.size(); i++) {
                if (skeletonData.ikConstraints[i].name && *skeletonData.ikConstraints[i].name == ikName) {
                    ikIndex = i;
                    break;
                }
            }
            writeVarint(binary, ikIndex, true);
        }
        writeVarint(binary, skin.transform.size(), true);
        for (const std::string& transformName : skin.transform) {
            int transformIndex = 0;
            for (size_t i = 0; i < skeletonData.transformConstraints.size(); i++) {
                if (skeletonData.transformConstraints[i].name && *skeletonData.transformConstraints[i].name == transformName) {
                    transformIndex = i;
                    break;
                }
            }
            writeVarint(binary, transformIndex, true);
        }
        writeVarint(binary, skin.path.size(), true);
        for (const std::string& pathName : skin.path) {
            int pathIndex = 0;
            for (size_t i = 0; i < skeletonData.pathConstraints.size(); i++) {
                if (skeletonData.pathConstraints[i].name && *skeletonData.pathConstraints[i].name == pathName) {
                    pathIndex = i;
                    break;
                }
            }
            writeVarint(binary, pathIndex, true);
        }
        writeVarint(binary, skin.attachments.size(), true);
    }
    for (const auto& [slotName, slotMap] : skin.attachments) {
        int slotIndex = 0;
        for (size_t i = 0; i < skeletonData.slots.size(); i++) {
            if (skeletonData.slots[i].name && *skeletonData.slots[i].name == slotName) {
                slotIndex = i;
                break;
            }
        }
        writeVarint(binary, slotIndex, true);
        writeVarint(binary, slotMap.size(), true);
        for (const auto& [attachmentName, attachment] : slotMap) {
            writeStringRef(binary, attachmentName, skeletonData);
            if (attachment.name != attachmentName) writeStringRef(binary, attachment.name, skeletonData);
            else writeStringRef(binary, std::nullopt, skeletonData);
            writeByte(binary, (unsigned char)attachment.type);
            switch (attachment.type) {
                case AttachmentType_Region: {
                    const RegionAttachment& region = std::get<RegionAttachment>(attachment.data);
                    if (attachment.path != attachment.name) writeStringRef(binary, attachment.path, skeletonData);
                    else writeStringRef(binary, std::nullopt, skeletonData);
                    writeFloat(binary, region.rotation);
                    writeFloat(binary, region.x);
                    writeFloat(binary, region.y);
                    writeFloat(binary, region.scaleX);
                    writeFloat(binary, region.scaleY);
                    writeFloat(binary, region.width);
                    writeFloat(binary, region.height);
                    if (region.color) writeColor(binary, region.color.value());
                    else writeColor(binary, Color{0xff, 0xff, 0xff, 0xff});
                    break; 
                }
                case AttachmentType_Boundingbox: {
                    const BoundingboxAttachment& box = std::get<BoundingboxAttachment>(attachment.data);
                    writeVarint(binary, box.vertexCount, true);
                    writeVertices(binary, box.vertices, box.vertices.size() > box.vertexCount * 2);
                    if (skeletonData.nonessential) {
                        if (box.color) writeColor(binary, box.color.value());
                        else writeColor(binary, Color{0xff, 0xff, 0xff, 0xff});
                    }
                    break;
                }
                case AttachmentType_Mesh: {
                    const MeshAttachment& mesh = std::get<MeshAttachment>(attachment.data);
                    if (attachment.path != attachment.name) writeStringRef(binary, attachment.path, skeletonData);
                    else writeStringRef(binary, std::nullopt, skeletonData);
                    if (mesh.color) writeColor(binary, mesh.color.value());
                    else writeColor(binary, Color{0xff, 0xff, 0xff, 0xff});
                    int vertexCount = mesh.uvs.size() / 2;
                    writeVarint(binary, vertexCount, true);
                    writeFloatArray(binary, mesh.uvs);
                    writeShortArray(binary, mesh.triangles);
                    writeVertices(binary, mesh.vertices, mesh.vertices.size() > vertexCount * 2);
                    writeVarint(binary, mesh.hullLength, true);
                    if (skeletonData.nonessential) {
                        writeShortArray(binary, mesh.edges);
                        writeFloat(binary, mesh.width);
                        writeFloat(binary, mesh.height);
                    }
                    break; 
                }
                case AttachmentType_Linkedmesh: {
                    const LinkedmeshAttachment& linkedMesh = std::get<LinkedmeshAttachment>(attachment.data);
                    if (attachment.path != attachment.name) writeStringRef(binary, attachment.path, skeletonData);
                    else writeStringRef(binary, std::nullopt, skeletonData);
                    if (linkedMesh.color) writeColor(binary, linkedMesh.color.value());
                    else writeColor(binary, Color{0xff, 0xff, 0xff, 0xff});
                    writeStringRef(binary, linkedMesh.skin, skeletonData);
                    writeStringRef(binary, linkedMesh.parentMesh, skeletonData);
                    writeBoolean(binary, linkedMesh.timelines > 0); 
                    if (skeletonData.nonessential) {
                        writeFloat(binary, linkedMesh.width);
                        writeFloat(binary, linkedMesh.height);
                    }
                    break; 
                }
                case AttachmentType_Path: {
                    const PathAttachment& path = std::get<PathAttachment>(attachment.data);
                    writeBoolean(binary, path.closed);
                    writeBoolean(binary, path.constantSpeed);
                    writeVarint(binary, path.vertexCount, true);
                    writeVertices(binary, path.vertices, path.vertices.size() > path.vertexCount * 2);
                    writeFloatArray(binary, path.lengths); 
                    if (skeletonData.nonessential) {
                        if (path.color) writeColor(binary, path.color.value());
                        else writeColor(binary, Color{0xff, 0xff, 0xff, 0xff});
                    }
                    break; 
                }
                case AttachmentType_Point: {
                    const PointAttachment& point = std::get<PointAttachment>(attachment.data);
                    writeFloat(binary, point.rotation);
                    writeFloat(binary, point.x);
                    writeFloat(binary, point.y);
                    if (skeletonData.nonessential) {
                        if (point.color) writeColor(binary, point.color.value());
                        else writeColor(binary, Color{0xff, 0xff, 0xff, 0xff});
                    }
                    break;
                }
                case AttachmentType_Clipping: {
                    const ClippingAttachment& clipping = std::get<ClippingAttachment>(attachment.data);
                    int slotIndex = -1; 
                    for (size_t i = 0; i < skeletonData.slots.size(); i++) {
                        if (skeletonData.slots[i].name == clipping.endSlot) {
                            slotIndex = i;
                            break;
                        }
                    }
                    writeVarint(binary, slotIndex, true);
                    writeVarint(binary, clipping.vertexCount, true);
                    writeVertices(binary, clipping.vertices, clipping.vertices.size() > clipping.vertexCount * 2);
                    if (skeletonData.nonessential) {
                        if (clipping.color) writeColor(binary, clipping.color.value());
                        else writeColor(binary, Color{0xff, 0xff, 0xff, 0xff});
                    }
                    break;
                }
            }
        }
    }
}

void writeAnimation(Binary& binary, const Animation& animation, const SkeletonData& skeletonData) {
    writeString(binary, animation.name); 
    writeVarint(binary, 0, true); // numTimelines is no use.
    writeVarint(binary, animation.slots.size(), true);
    for (const auto& [slotName, multiTimeline] : animation.slots) {
        int slotIndex = 0; 
        for (size_t i = 0; i < skeletonData.slots.size(); i++) {
            if (skeletonData.slots[i].name && *skeletonData.slots[i].name == slotName) {
                slotIndex = i;
                break;
            }
        }
        writeVarint(binary, slotIndex, true);
        writeVarint(binary, multiTimeline.size(), true);
        for (const auto& [timelineName, timeline] : multiTimeline) {
            SlotTimelineType timelineType = slotTimelineTypeMap.at(timelineName);
            writeByte(binary, (unsigned char)timelineType);
            writeVarint(binary, timeline.size(), true);
            switch (timelineType) {
                case SlotTimelineType::SLOT_ATTACHMENT: {
                    for (const auto& frame : timeline) {
                        writeFloat(binary, frame.time);
                        writeStringRef(binary, frame.str1, skeletonData);
                    }
                    break;
                }
                case SlotTimelineType::SLOT_RGBA: {
                    writeVarint(binary, timeline.size() * 4, true); 
                    writeFloat(binary, timeline[0].time);
                    writeColor(binary, timeline[0].color1.value()); 
                    for (int frameIndex = 1; frameIndex < timeline.size(); frameIndex++) {
                        writeFloat(binary, timeline[frameIndex].time); 
                        writeColor(binary, timeline[frameIndex].color1.value()); 
                        CurveType curveType = timeline[frameIndex - 1].curveType;
                        writeSByte(binary, (signed char)curveType);
                        if (curveType == CurveType::CURVE_BEZIER) {
                            writeCurve(binary, timeline[frameIndex - 1]);
                        }
                    }
                    break;
                }
                case SlotTimelineType::SLOT_RGB: {
                    writeVarint(binary, timeline.size() * 3, true); 
                    writeFloat(binary, timeline[0].time);
                    writeColor(binary, timeline[0].color1.value(), false); 
                    for (int frameIndex = 1; frameIndex < timeline.size(); frameIndex++) {
                        writeFloat(binary, timeline[frameIndex].time); 
                        writeColor(binary, timeline[frameIndex].color1.value(), false); 
                        CurveType curveType = timeline[frameIndex - 1].curveType;
                        writeSByte(binary, (signed char)curveType);
                        if (curveType == CurveType::CURVE_BEZIER) {
                            writeCurve(binary, timeline[frameIndex - 1]);
                        }
                    }
                    break;
                }
                case SlotTimelineType::SLOT_RGBA2: {
                    writeVarint(binary, timeline.size() * 7, true); 
                    writeFloat(binary, timeline[0].time);
                    writeColor(binary, timeline[0].color1.value()); 
                    writeColor(binary, timeline[0].color2.value(), false); 
                    for (int frameIndex = 1; frameIndex < timeline.size(); frameIndex++) {
                        writeFloat(binary, timeline[frameIndex].time); 
                        writeColor(binary, timeline[frameIndex].color1.value()); 
                        writeColor(binary, timeline[frameIndex].color2.value(), false); 
                        CurveType curveType = timeline[frameIndex - 1].curveType;
                        writeSByte(binary, (signed char)curveType);
                        if (curveType == CurveType::CURVE_BEZIER) {
                            writeCurve(binary, timeline[frameIndex - 1]);
                        }
                    }
                    break;
                }
                case SlotTimelineType::SLOT_RGB2: {
                    writeVarint(binary, timeline.size() * 6, true); 
                    writeFloat(binary, timeline[0].time);
                    writeColor(binary, timeline[0].color1.value(), false); 
                    writeColor(binary, timeline[0].color2.value(), false); 
                    for (int frameIndex = 1; frameIndex < timeline.size(); frameIndex++) {
                        writeFloat(binary, timeline[frameIndex].time); 
                        writeColor(binary, timeline[frameIndex].color1.value(), false); 
                        writeColor(binary, timeline[frameIndex].color2.value(), false); 
                        CurveType curveType = timeline[frameIndex - 1].curveType;
                        writeSByte(binary, (signed char)curveType);
                        if (curveType == CurveType::CURVE_BEZIER) {
                            writeCurve(binary, timeline[frameIndex - 1]);
                        }
                    }
                    break;
                }
                case SlotTimelineType::SLOT_ALPHA: {
                    writeVarint(binary, timeline.size(), true);
                    writeFloat(binary, timeline[0].time);
                    writeByte(binary, (int)(timeline[0].value1 * 255.0f));
                    for (int frameIndex = 1; frameIndex < timeline.size(); frameIndex++) {
                        writeFloat(binary, timeline[frameIndex].time); 
                        writeByte(binary, (int)(timeline[frameIndex].value1 * 255.0f));
                        CurveType curveType = timeline[frameIndex - 1].curveType;
                        writeSByte(binary, (signed char)curveType);
                        if (curveType == CurveType::CURVE_BEZIER) {
                            writeCurve(binary, timeline[frameIndex - 1]);
                        }
                    }
                    break;
                }
            }
        }
    }
    writeVarint(binary, animation.bones.size(), true);
    for (const auto& [boneName, multiTimeline] : animation.bones) {
        int boneIndex = 0; 
        for (size_t i = 0; i < skeletonData.bones.size(); i++) {
            if (skeletonData.bones[i].name && *skeletonData.bones[i].name == boneName) {
                boneIndex = i;
                break;
            }
        }
        writeVarint(binary, boneIndex, true);
        writeVarint(binary, multiTimeline.size(), true);
        for (const auto& [timelineName, timeline] : multiTimeline) {
            BoneTimelineType timelineType = boneTimelineTypeMap.at(timelineName);
            if (timelineType == BONE_INHERIT) continue;
            writeByte(binary, (unsigned char)timelineType);
            writeVarint(binary, timeline.size(), true);
            switch (timelineType) {
                case BONE_ROTATE:
                case BONE_TRANSLATEX:
                case BONE_TRANSLATEY:
                case BONE_SCALEX:
                case BONE_SCALEY:
                case BONE_SHEARX:
                case BONE_SHEARY: {
                    writeVarint(binary, timeline.size(), true); 
                    writeTimeline(binary, timeline, 1);
                    break;
                }
                case BONE_TRANSLATE:
                case BONE_SCALE:
                case BONE_SHEAR: {
                    writeVarint(binary, timeline.size() * 2, true); 
                    writeTimeline(binary, timeline, 2);
                    break;
                }
            }
        }
    }
    writeVarint(binary, animation.ik.size(), true);
    for (const auto& [ikName, timeline] : animation.ik) {
        int ikIndex = 0; 
        for (size_t i = 0; i < skeletonData.ikConstraints.size(); i++) {
            if (skeletonData.ikConstraints[i].name && *skeletonData.ikConstraints[i].name == ikName) {
                ikIndex = i;
                break;
            }
        }
        writeVarint(binary, ikIndex, true);
        writeVarint(binary, timeline.size(), true);
        writeVarint(binary, timeline.size() * 2, true);
        writeFloat(binary, timeline[0].time);
        writeFloat(binary, timeline[0].value1);
        writeFloat(binary, timeline[0].value2);
        for (int frameIndex = 0; ; frameIndex++) {
            writeSByte(binary, timeline[frameIndex].bendPositive ? 1 : -1); 
            writeBoolean(binary, timeline[frameIndex].compress);
            writeBoolean(binary, timeline[frameIndex].stretch);
            if (frameIndex == timeline.size() - 1) break;
            writeFloat(binary, timeline[frameIndex + 1].time);
            writeFloat(binary, timeline[frameIndex + 1].value1);
            writeFloat(binary, timeline[frameIndex + 1].value2);
            CurveType curveType = timeline[frameIndex].curveType;
            writeSByte(binary, (signed char)curveType);
            if (curveType == CurveType::CURVE_BEZIER) {
                writeCurve(binary, timeline[frameIndex]);
            }
        }
    }
    writeVarint(binary, animation.transform.size(), true); 
    for (const auto& [transformName, timeline] : animation.transform) {
        int transformIndex = 0; 
        for (size_t i = 0; i < skeletonData.transformConstraints.size(); i++) {
            if (skeletonData.transformConstraints[i].name && *skeletonData.transformConstraints[i].name == transformName) {
                transformIndex = i;
                break;
            }
        }
        writeVarint(binary, transformIndex, true);
        writeVarint(binary, timeline.size(), true);
        writeVarint(binary, timeline.size() * 6, true);
        writeFloat(binary, timeline[0].time);
        writeFloat(binary, timeline[0].value1);
        writeFloat(binary, timeline[0].value2);
        writeFloat(binary, timeline[0].value3);
        writeFloat(binary, timeline[0].value4);
        writeFloat(binary, timeline[0].value5);
        writeFloat(binary, timeline[0].value6);
        for (int frameIndex = 1; frameIndex < timeline.size(); frameIndex++) {
            writeFloat(binary, timeline[frameIndex].time);
            writeFloat(binary, timeline[frameIndex].value1);
            writeFloat(binary, timeline[frameIndex].value2);
            writeFloat(binary, timeline[frameIndex].value3);
            writeFloat(binary, timeline[frameIndex].value4);
            writeFloat(binary, timeline[frameIndex].value5);
            writeFloat(binary, timeline[frameIndex].value6);
            CurveType curveType = timeline[frameIndex - 1].curveType;
            writeSByte(binary, (signed char)curveType);
            if (curveType == CurveType::CURVE_BEZIER) {
                writeCurve(binary, timeline[frameIndex - 1]);
            }
        }
    }
    writeVarint(binary, animation.path.size(), true); 
    for (const auto& [pathName, multiTimeline] : animation.path) {
        int pathIndex = 0; 
        for (size_t i = 0; i < skeletonData.pathConstraints.size(); i++) {
            if (skeletonData.pathConstraints[i].name && *skeletonData.pathConstraints[i].name == pathName) {
                pathIndex = i;
                break;
            }
        }
        writeVarint(binary, pathIndex, true);
        writeVarint(binary, multiTimeline.size(), true);
        for (const auto& [timelineName, timeline] : multiTimeline) {
            PathTimelineType timelineType = pathTimelineTypeMap.at(timelineName);
            writeByte(binary, (unsigned char)timelineType);
            writeVarint(binary, timeline.size(), true);
            switch (timelineType) {
                case PATH_POSITION:
                case PATH_SPACING: {
                    writeVarint(binary, timeline.size(), true); 
                    writeTimeline(binary, timeline, 1);
                    break;
                }
                case PATH_MIX: {
                    writeVarint(binary, timeline.size() * 3, true); 
                    writeTimeline(binary, timeline, 3);
                    break;
                }
            }
        }
    }
    writeVarint(binary, animation.attachments.size(), true); 
    for (const auto& [skinName, skinMap] : animation.attachments) {
        int skinIndex = 0;
        for (size_t i = 0; i < skeletonData.skins.size(); i++) {
            if (skeletonData.skins[i].name == skinName) {
                skinIndex = i;
                break;
            }
        }
        writeVarint(binary, skinIndex, true);
        writeVarint(binary, skinMap.size(), true);
        for (const auto& [slotName, slotMap] : skinMap) {
            int slotIndex = 0; 
            for (size_t i = 0; i < skeletonData.slots.size(); i++) {
                if (skeletonData.slots[i].name && *skeletonData.slots[i].name == slotName) {
                    slotIndex = i;
                    break;
                }
            }
            writeVarint(binary, slotIndex, true);
            writeVarint(binary, slotMap.size(), true);
            for (const auto& [attachmentName, multiTimeline] : slotMap) {
                if (!multiTimeline.contains("deform")) continue;
                const auto& timeline = multiTimeline.at("deform");
                writeStringRef(binary, attachmentName, skeletonData); 
                writeVarint(binary, timeline.size(), true);
                writeVarint(binary, timeline.size(), true);
                writeFloat(binary, timeline[0].time);
                for (int frameIndex = 0; ; frameIndex++) {
                    writeVarint(binary, timeline[frameIndex].vertices.size(), true); 
                    if (timeline[frameIndex].vertices.size() > 0) {
                        writeVarint(binary, timeline[frameIndex].int1, true); 
                        for (float v : timeline[frameIndex].vertices) {
                            writeFloat(binary, v);
                        }
                    }
                    if (frameIndex == timeline.size() - 1) break;
                    writeFloat(binary, timeline[frameIndex + 1].time);
                    CurveType curveType = timeline[frameIndex].curveType;
                    writeSByte(binary, (signed char)curveType);
                    if (curveType == CurveType::CURVE_BEZIER) {
                        writeCurve(binary, timeline[frameIndex]);
                    }
                }
            }
        }
    }
    writeVarint(binary, animation.drawOrder.size(), true);
    for (const auto& frame : animation.drawOrder) {
        writeFloat(binary, frame.time);
        writeVarint(binary, frame.offsets.size(), true);
        for (const auto& [slotName, offset] : frame.offsets) {
            int slotIndex = 0; 
            for (size_t i = 0; i < skeletonData.slots.size(); i++) {
                if (skeletonData.slots[i].name && *skeletonData.slots[i].name == slotName) {
                    slotIndex = i;
                    break;
                }
            }
            writeVarint(binary, slotIndex, true);
            writeVarint(binary, offset, true);
        }
    }
    writeVarint(binary, animation.events.size(), true);
    for (const auto& frame : animation.events) {
        writeFloat(binary, frame.time);
        int eventIndex = 0;
        for (size_t i = 0; i < skeletonData.events.size(); i++) {
            if (skeletonData.events[i].name == frame.str1) {
                eventIndex = i;
                break;
            }
        }
        const EventData& eventData = skeletonData.events[eventIndex];
        writeVarint(binary, eventIndex, true);
        writeVarint(binary, frame.int1, false);
        writeFloat(binary, frame.value1);
        if (frame.str2 != eventData.stringValue) {
            writeBoolean(binary, true);
            writeString(binary, frame.str2);
        } else {
            writeBoolean(binary, false);
        }
        if (eventData.audioPath && !eventData.audioPath->empty()) {
            writeFloat(binary, frame.value2);
            writeFloat(binary, frame.value3);
        }
    }
}

Binary writeBinaryData(SkeletonData& skeletonData) {
    Binary binary;
    
    writeInt(binary, skeletonData.hash & 0xffffffff); 
    writeInt(binary, (skeletonData.hash >> 32) & 0xffffffff);
    writeString(binary, skeletonData.version);
    writeFloat(binary, skeletonData.x); 
    writeFloat(binary, skeletonData.y);
    writeFloat(binary, skeletonData.width);
    writeFloat(binary, skeletonData.height);
    writeBoolean(binary, skeletonData.nonessential);
    if (skeletonData.nonessential) {
        writeFloat(binary, skeletonData.fps);
        writeString(binary, skeletonData.imagesPath);
        writeString(binary, skeletonData.audioPath);
    }

    std::set<std::string> strings; 
    for (const SlotData& slot : skeletonData.slots) {
        if (slot.attachmentName) strings.insert(slot.attachmentName.value()); 
    }
    for (const Skin& skin : skeletonData.skins) {
        if (skin.name != "default") strings.insert(skin.name);
        for (const auto& [slotName, slotMap] : skin.attachments) {
            for (const auto& [attachmentName, attachment] : slotMap) {
                strings.insert(attachmentName);
                if (attachment.name != attachmentName) strings.insert(attachment.name);
                if (attachment.path != attachment.name) strings.insert(attachment.path);
            }
        }
    }
    for (const EventData& event : skeletonData.events) {
        strings.insert(event.name);
    }
    writeVarint(binary, strings.size(), true);
    skeletonData.strings.clear();
    for (const std::string& str : strings) {
        skeletonData.strings.push_back(str);
        writeString(binary, str);
    }

    /* Bones */
    writeVarint(binary, skeletonData.bones.size(), true);
    for (const BoneData& bone : skeletonData.bones) {
        writeString(binary, bone.name);
        if (bone.parent) {
            int parentIndex = 0; 
            for (size_t i = 0; i < skeletonData.bones.size(); i++) {
                if (skeletonData.bones[i].name && *skeletonData.bones[i].name == *bone.parent) {
                    parentIndex = i;
                    break;
                }
            }
            writeVarint(binary, parentIndex, true);
        }
        writeFloat(binary, bone.rotation);
        writeFloat(binary, bone.x);
        writeFloat(binary, bone.y);
        writeFloat(binary, bone.scaleX);
        writeFloat(binary, bone.scaleY);
        writeFloat(binary, bone.shearX);
        writeFloat(binary, bone.shearY);
        writeFloat(binary, bone.length);
        writeVarint(binary, bone.inherit, true); 
        writeBoolean(binary, bone.skinRequired); 
        if (skeletonData.nonessential) {
            if (bone.color) writeColor(binary, bone.color.value());
            else writeColor(binary, Color{0x9b, 0x9b, 0x9b, 0xff});
        }
    }

    /* Slots */
    writeVarint(binary, skeletonData.slots.size(), true);
    for (const SlotData& slot : skeletonData.slots) {
        writeString(binary, slot.name);
        int boneIndex = 0; 
        for (size_t i = 0; i < skeletonData.bones.size(); i++) {
            if (skeletonData.bones[i].name && *skeletonData.bones[i].name == slot.bone) {
                boneIndex = i;
                break;
            }
        }
        writeVarint(binary, boneIndex, true);
        if (slot.color) writeColor(binary, slot.color.value());
        else writeColor(binary, Color{0xff, 0xff, 0xff, 0xff});
        if (slot.darkColor) {
            writeByte(binary, slot.darkColor.value().a); 
            writeByte(binary, slot.darkColor.value().r);
            writeByte(binary, slot.darkColor.value().g);
            writeByte(binary, slot.darkColor.value().b);
        } else writeColor(binary, Color{0xff, 0xff, 0xff, 0xff});
        writeStringRef(binary, slot.attachmentName, skeletonData);
        writeVarint(binary, slot.blendMode, true); 
    }

    /* IK constraints */
    writeVarint(binary, skeletonData.ikConstraints.size(), true);
    for (const IKConstraintData& ik : skeletonData.ikConstraints) {
        writeString(binary, ik.name);
        writeVarint(binary, ik.order, true);
        writeBoolean(binary, ik.skinRequired);
        writeVarint(binary, ik.bones.size(), true);
        for (const std::string& boneName : ik.bones) {
            int boneIndex = 0; 
            for (size_t i = 0; i < skeletonData.bones.size(); i++) {
                if (skeletonData.bones[i].name && *skeletonData.bones[i].name == boneName) {
                    boneIndex = i;
                    break;
                }
            }
            writeVarint(binary, boneIndex, true);
        }
        int targetIndex = 0; 
        for (size_t i = 0; i < skeletonData.bones.size(); i++) {
            if (skeletonData.bones[i].name && *skeletonData.bones[i].name == ik.target) {
                targetIndex = i;
                break;
            }
        }
        writeVarint(binary, targetIndex, true);
        writeFloat(binary, ik.mix);
        writeFloat(binary, ik.softness);
        writeSByte(binary, ik.bendPositive ? 1 : -1);
        writeBoolean(binary, ik.compress);
        writeBoolean(binary, ik.stretch);
        writeBoolean(binary, ik.uniform);
    }

    /* Transform constraints */
    writeVarint(binary, skeletonData.transformConstraints.size(), true);
    for (const TransformConstraintData& transform : skeletonData.transformConstraints) {
        writeString(binary, transform.name);
        writeVarint(binary, transform.order, true);
        writeBoolean(binary, transform.skinRequired);
        writeVarint(binary, transform.bones.size(), true);
        for (const std::string& boneName : transform.bones) {
            int boneIndex = 0; 
            for (size_t i = 0; i < skeletonData.bones.size(); i++) {
                if (skeletonData.bones[i].name && *skeletonData.bones[i].name == boneName) {
                    boneIndex = i;
                    break;
                }
            }
            writeVarint(binary, boneIndex, true);
        }
        int targetIndex = 0; 
        for (size_t i = 0; i < skeletonData.bones.size(); i++) {
            if (skeletonData.bones[i].name && *skeletonData.bones[i].name == transform.target) {
                targetIndex = i;
                break;
            }
        }
        writeVarint(binary, targetIndex, true);
        writeBoolean(binary, transform.local);
        writeBoolean(binary, transform.relative);
        writeFloat(binary, transform.offsetRotation);
        writeFloat(binary, transform.offsetX);
        writeFloat(binary, transform.offsetY);
        writeFloat(binary, transform.offsetScaleX);
        writeFloat(binary, transform.offsetScaleY);
        writeFloat(binary, transform.offsetShearY);
        writeFloat(binary, transform.mixRotate);
        writeFloat(binary, transform.mixX);
        writeFloat(binary, transform.mixY);
        writeFloat(binary, transform.mixScaleX);
        writeFloat(binary, transform.mixScaleY);
        writeFloat(binary, transform.mixShearY);
    }

    /* Path constraints */
    writeVarint(binary, skeletonData.pathConstraints.size(), true);
    for (const PathConstraintData& path : skeletonData.pathConstraints) {
        writeString(binary, path.name);
        writeVarint(binary, path.order, true);
        writeBoolean(binary, path.skinRequired);
        writeVarint(binary, path.bones.size(), true);
        for (const std::string& boneName : path.bones) {
            int boneIndex = 0; 
            for (size_t i = 0; i < skeletonData.bones.size(); i++) {
                if (skeletonData.bones[i].name && *skeletonData.bones[i].name == boneName) {
                    boneIndex = i;
                    break;
                }
            }
            writeVarint(binary, boneIndex, true);
        }
        int targetIndex = 0; 
        for (size_t i = 0; i < skeletonData.slots.size(); i++) {
            if (skeletonData.slots[i].name && *skeletonData.slots[i].name == path.target) {
                targetIndex = i;
                break;
            }
        }
        writeVarint(binary, targetIndex, true);
        writeVarint(binary, path.positionMode, true);
        writeVarint(binary, path.spacingMode, true);
        writeVarint(binary, path.rotateMode, true);
        writeFloat(binary, path.offsetRotation);
        writeFloat(binary, path.position);
        writeFloat(binary, path.spacing);
        writeFloat(binary, path.mixRotate); 
        writeFloat(binary, path.mixX);
        writeFloat(binary, path.mixY);
    }

    /* Skins */
    for (const Skin& skin : skeletonData.skins) {
        if (skin.name == "default") {
            writeSkin(binary, skin, skeletonData, true);
            break;
        }
    }
    writeVarint(binary, skeletonData.skins.size() - 1, true);
    for (const Skin& skin : skeletonData.skins) {
        if (skin.name != "default") {
            writeSkin(binary, skin, skeletonData, false);
        }
    }

    /* Events */
    writeVarint(binary, skeletonData.events.size(), true);
    for (const EventData& event : skeletonData.events) {
        writeStringRef(binary, event.name, skeletonData);
        writeVarint(binary, event.intValue, true);
        writeFloat(binary, event.floatValue);
        writeString(binary, event.stringValue);
        writeString(binary, event.audioPath);
        if (event.audioPath && event.audioPath.value().length() > 0) {
            writeFloat(binary, event.volume);
            writeFloat(binary, event.balance);
        }
    }

    /* Animations */
    writeVarint(binary, skeletonData.animations.size(), true);
    for (const Animation& animation : skeletonData.animations) {
        writeAnimation(binary, animation, skeletonData);
    }

    return binary;
}

}
