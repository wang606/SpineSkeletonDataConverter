#include "SkeletonData.h"

namespace spine37 {

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
    writeByte(binary, (unsigned char)frame.curveType);
    if (frame.curveType == CurveType::CURVE_BEZIER) {
        writeFloat(binary, frame.curve[0]);
        writeFloat(binary, frame.curve[1]);
        writeFloat(binary, frame.curve[2]);
        writeFloat(binary, frame.curve[3]);
    }
}

void writeTimeline(Binary& binary, const Timeline& timeline, int valueNum) {
    for (int frameIndex = 0; frameIndex < timeline.size(); frameIndex++) {
        writeFloat(binary, timeline[frameIndex].time); 
        writeFloat(binary, timeline[frameIndex].value1);
        if (valueNum > 1) writeFloat(binary, timeline[frameIndex].value2);
        if (frameIndex < timeline.size() - 1) writeCurve(binary, timeline[frameIndex]);
    }
}

void writeSkin(Binary& binary, const Skin& skin, const SkeletonData& skeletonData, bool defaultSkin) {
    if (defaultSkin) {
        writeVarint(binary, skin.attachments.size(), true); 
    } else {
        writeString(binary, skin.name); 
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
            writeString(binary, attachmentName);
            if (attachment.name != attachmentName) writeString(binary, attachment.name);
            else writeString(binary, std::nullopt);
            writeByte(binary, (unsigned char)attachment.type);
            switch (attachment.type) {
                case AttachmentType_Region: {
                    const RegionAttachment& region = std::get<RegionAttachment>(attachment.data);
                    if (attachment.path != attachment.name) writeString(binary, attachment.path);
                    else writeString(binary, std::nullopt);
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
                    if (attachment.path != attachment.name) writeString(binary, attachment.path);
                    else writeString(binary, std::nullopt);
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
                    if (attachment.path != attachment.name) writeString(binary, attachment.path);
                    else writeString(binary, std::nullopt);
                    if (linkedMesh.color) writeColor(binary, linkedMesh.color.value());
                    else writeColor(binary, Color{0xff, 0xff, 0xff, 0xff});
                    writeString(binary, linkedMesh.skin);
                    writeString(binary, linkedMesh.parentMesh);
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
            if (timelineType == SlotTimelineType::SLOT_ALPHA) continue;
            switch (timelineType) {
                case SlotTimelineType::SLOT_ATTACHMENT:
                    writeByte(binary, 0);  // SLOT_ATTACHMENT
                    break;
                case SlotTimelineType::SLOT_RGBA:
                case SlotTimelineType::SLOT_RGB:
                    writeByte(binary, 1);  // SLOT_COLOR
                    break;
                case SlotTimelineType::SLOT_RGBA2:
                case SlotTimelineType::SLOT_RGB2:
                    writeByte(binary, 2);  // SLOT_TWO_COLOR
                    break;
            }
            writeVarint(binary, timeline.size(), true);
            switch (timelineType) {
                case SlotTimelineType::SLOT_ATTACHMENT: {
                    for (const auto& frame : timeline) {
                        writeFloat(binary, frame.time);
                        writeString(binary, frame.str1);
                    }
                    break;
                }
                case SlotTimelineType::SLOT_RGBA:
                case SlotTimelineType::SLOT_RGB: {
                    for (int frameIndex = 0; frameIndex < timeline.size(); frameIndex++) {
                        writeFloat(binary, timeline[frameIndex].time); 
                        writeColor(binary, timeline[frameIndex].color1.value()); 
                        if (frameIndex < timeline.size() - 1) writeCurve(binary, timeline[frameIndex]);
                    }
                    break;
                }
                case SlotTimelineType::SLOT_RGBA2:
                case SlotTimelineType::SLOT_RGB2: {
                    for (int frameIndex = 0; frameIndex < timeline.size(); frameIndex++) {
                        writeFloat(binary, timeline[frameIndex].time); 
                        writeColor(binary, timeline[frameIndex].color1.value()); 
                        writeByte(binary, timeline[frameIndex].color2->a); 
                        writeByte(binary, timeline[frameIndex].color2->r); 
                        writeByte(binary, timeline[frameIndex].color2->g); 
                        writeByte(binary, timeline[frameIndex].color2->b); 
                        if (frameIndex < timeline.size() - 1) writeCurve(binary, timeline[frameIndex]);
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
            switch (timelineType) {
                case BONE_ROTATE:
                    writeByte(binary, 0);  // BONE_ROTATE
                    break;
                case BONE_TRANSLATE:
                case BONE_TRANSLATEX:
                case BONE_TRANSLATEY:
                    writeByte(binary, 1);  // BONE_TRANSLATE
                    break;
                case BONE_SCALE:
                case BONE_SCALEX:
                case BONE_SCALEY:
                    writeByte(binary, 2);  // BONE_SCALE
                    break;
                case BONE_SHEAR:
                case BONE_SHEARX:
                case BONE_SHEARY:
                    writeByte(binary, 3);  // BONE_SHEAR
                    break;
            }
            writeVarint(binary, timeline.size(), true);
            switch (timelineType) {
                case BONE_ROTATE: {
                    writeTimeline(binary, timeline, 1);
                    break;
                }
                case BONE_TRANSLATEX:
                case BONE_SCALEX:
                case BONE_SHEARX: {
                    for (int frameIndex = 0; frameIndex < timeline.size(); frameIndex++) {
                        writeFloat(binary, timeline[frameIndex].time); 
                        writeFloat(binary, timeline[frameIndex].value1); 
                        writeFloat(binary, 0.0f); 
                        if (frameIndex < timeline.size() - 1) writeCurve(binary, timeline[frameIndex]);
                    }
                    break;
                }
                case BONE_TRANSLATEY:
                case BONE_SCALEY:
                case BONE_SHEARY: {
                    for (int frameIndex = 0; frameIndex < timeline.size(); frameIndex++) {
                        writeFloat(binary, timeline[frameIndex].time); 
                        writeFloat(binary, 0.0f); 
                        writeFloat(binary, timeline[frameIndex].value1); 
                        if (frameIndex < timeline.size() - 1) writeCurve(binary, timeline[frameIndex]);
                    }
                    break;
                }
                case BONE_TRANSLATE:
                case BONE_SCALE:
                case BONE_SHEAR: {
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
        for (int frameIndex = 0; frameIndex < timeline.size(); frameIndex++) {
            writeFloat(binary, timeline[frameIndex].time);
            writeFloat(binary, timeline[frameIndex].value1);
            writeSByte(binary, timeline[frameIndex].bendPositive ? 1 : -1); 
            writeBoolean(binary, timeline[frameIndex].compress);
            writeBoolean(binary, timeline[frameIndex].stretch);
            if (frameIndex < timeline.size() - 1) writeCurve(binary, timeline[frameIndex]);
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
        for (int frameIndex = 0; frameIndex < timeline.size(); frameIndex++) {
            writeFloat(binary, timeline[frameIndex].time);
            writeFloat(binary, timeline[frameIndex].value1);
            writeFloat(binary, timeline[frameIndex].value2);
            writeFloat(binary, timeline[frameIndex].value4);
            writeFloat(binary, timeline[frameIndex].value6);
            if (frameIndex < timeline.size() - 1) writeCurve(binary, timeline[frameIndex]);
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
            writeSByte(binary, (signed char)timelineType);
            writeVarint(binary, timeline.size(), true);
            switch (timelineType) {
                case PATH_POSITION:
                case PATH_SPACING: {
                    writeTimeline(binary, timeline, 1);
                    break;
                }
                case PATH_MIX: {
                    writeTimeline(binary, timeline, 2);
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
                writeString(binary, attachmentName); 
                writeVarint(binary, timeline.size(), true);
                for (int frameIndex = 0; frameIndex < timeline.size(); frameIndex++) {
                    writeFloat(binary, timeline[frameIndex].time);
                    writeVarint(binary, timeline[frameIndex].vertices.size(), true); 
                    if (timeline[frameIndex].vertices.size() > 0) {
                        writeVarint(binary, timeline[frameIndex].int1, true); 
                        for (float v : timeline[frameIndex].vertices) {
                            writeFloat(binary, v);
                        }
                    }
                    if (frameIndex < timeline.size() - 1) writeCurve(binary, timeline[frameIndex]);
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
    
    if (skeletonData.hashString) {
        writeString(binary, skeletonData.hashString);
    } else {
        writeString(binary, uint64ToBase64(skeletonData.hash));
    }
    writeString(binary, skeletonData.version);
    writeFloat(binary, skeletonData.width);
    writeFloat(binary, skeletonData.height);
    writeBoolean(binary, skeletonData.nonessential);
    if (skeletonData.nonessential) {
        writeFloat(binary, skeletonData.fps);
        writeString(binary, skeletonData.imagesPath);
        writeString(binary, skeletonData.audioPath);
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
            writeByte(binary, slot.darkColor.value().r);
            writeByte(binary, slot.darkColor.value().g);
            writeByte(binary, slot.darkColor.value().b);
            writeByte(binary, slot.darkColor.value().a); 
        } else writeColor(binary, Color{0xff, 0xff, 0xff, 0xff});
        writeString(binary, slot.attachmentName);
        writeVarint(binary, slot.blendMode, true); 
    }

    /* IK constraints */
    writeVarint(binary, skeletonData.ikConstraints.size(), true);
    for (const IKConstraintData& ik : skeletonData.ikConstraints) {
        writeString(binary, ik.name);
        writeVarint(binary, ik.order, true);
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
        // writeFloat(binary, transform.mixY);
        writeFloat(binary, transform.mixScaleX);
        // writeFloat(binary, transform.mixScaleY);
        writeFloat(binary, transform.mixShearY);
    }

    /* Path constraints */
    writeVarint(binary, skeletonData.pathConstraints.size(), true);
    for (const PathConstraintData& path : skeletonData.pathConstraints) {
        writeString(binary, path.name);
        writeVarint(binary, path.order, true);
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
        // writeFloat(binary, path.mixY);
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
        writeString(binary, event.name);
        writeVarint(binary, event.intValue, false);
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
