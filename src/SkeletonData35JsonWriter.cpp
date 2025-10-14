#include "SkeletonData.h"

namespace spine35 {

void writeCurve(const TimelineFrame& frame, Json& j) {
    if (frame.curveType == CurveType::CURVE_STEPPED) {
        j["curve"] = "stepped";
    } else if (frame.curveType == CurveType::CURVE_BEZIER) {
        j["curve"] = frame.curve;
    }
}

void writeTimeline(const std::vector<TimelineFrame>& timeline, Json& j, int valueNum, const std::string& key1, const std::string& key2, float defaultValue) {
    for (const auto& frame : timeline) {
        Json frameJson = Json::object();
        frameJson["time"] = frame.time;
        frameJson[key1] = frame.value1;
        if (valueNum > 1) frameJson[key2] = frame.value2;
        writeCurve(frame, frameJson);
        j.push_back(frameJson);
    }
}

Json writeJsonData(const SkeletonData& skeletonData) {
    Json j = Json::object();

    Json skeleton = Json::object();
    if (skeletonData.hashString) skeleton["hash"] = skeletonData.hashString;
    else if (skeletonData.hash != 0) skeleton["hash"] = uint64ToBase64(skeletonData.hash);
    if (skeletonData.version) skeleton["spine"] = skeletonData.version;
    skeleton["width"] = skeletonData.width;
    skeleton["height"] = skeletonData.height;
    if (skeletonData.imagesPath) skeleton["images"] = skeletonData.imagesPath.value();
    j["skeleton"] = skeleton;

    /* Bones */
    for (const auto& bone : skeletonData.bones) {
        Json boneJson = Json::object();
        if (bone.name) boneJson["name"] = bone.name;
        if (bone.parent) boneJson["parent"] = bone.parent;
        if (bone.length != 0.0f) boneJson["length"] = bone.length;
        if (bone.x != 0.0f) boneJson["x"] = bone.x;
        if (bone.y != 0.0f) boneJson["y"] = bone.y;
        if (bone.rotation != 0.0f) boneJson["rotation"] = bone.rotation;
        if (bone.scaleX != 1.0f) boneJson["scaleX"] = bone.scaleX;
        if (bone.scaleY != 1.0f) boneJson["scaleY"] = bone.scaleY;
        if (bone.shearX != 0.0f) boneJson["shearX"] = bone.shearX;
        if (bone.shearY != 0.0f) boneJson["shearY"] = bone.shearY;
        if (bone.inherit != Inherit_Normal) boneJson["transform"] = inheritString.at(bone.inherit);
        if (bone.color) boneJson["color"] = colorToString(bone.color.value(), true);
        j["bones"].push_back(boneJson);
    }

    /* Slots */
    for (const auto& slot : skeletonData.slots) {
        Json slotJson = Json::object();
        if (slot.name) slotJson["name"] = slot.name;
        if (slot.bone) slotJson["bone"] = slot.bone;
        if (slot.color) slotJson["color"] = colorToString(slot.color.value(), true);
        if (slot.darkColor) slotJson["dark"] = colorToString(slot.darkColor.value(), false);
        if (slot.attachmentName) slotJson["attachment"] = slot.attachmentName;
        if (slot.blendMode != BlendMode_Normal) slotJson["blend"] = blendModeString.at(slot.blendMode);
        j["slots"].push_back(slotJson);
    }

    /* IK constraints */
    for (const auto& ik : skeletonData.ikConstraints) {
        Json ikJson = Json::object();
        if (ik.name) ikJson["name"] = ik.name;
        ikJson["order"] = ik.order;
        if (!ik.bones.empty()) ikJson["bones"] = ik.bones;
        if (ik.target) ikJson["target"] = ik.target;
        if (ik.mix != 1.0f) ikJson["mix"] = ik.mix;
        if (!ik.bendPositive) ikJson["bendPositive"] = ik.bendPositive;
        j["ik"].push_back(ikJson);
    }

    /* Transform constraints */
    for (const auto& transform : skeletonData.transformConstraints) {
        Json transformJson = Json::object();
        if (transform.name) transformJson["name"] = transform.name;
        transformJson["order"] = transform.order;
        if (!transform.bones.empty()) transformJson["bones"] = transform.bones;
        if (transform.target) transformJson["target"] = transform.target;
        if (transform.mixRotate != 1.0f) transformJson["rotateMix"] = transform.mixRotate;
        if (transform.mixX != 1.0f) transformJson["translateMix"] = transform.mixX;
        if (transform.mixScaleX != 1.0f) transformJson["scaleMix"] = transform.mixScaleX;
        if (transform.mixShearY != 1.0f) transformJson["shearMix"] = transform.mixShearY;
        if (transform.offsetRotation != 0.0f) transformJson["rotation"] = transform.offsetRotation;
        if (transform.offsetX != 0.0f) transformJson["x"] = transform.offsetX;
        if (transform.offsetY != 0.0f) transformJson["y"] = transform.offsetY;
        if (transform.offsetScaleX != 0.0f) transformJson["scaleX"] = transform.offsetScaleX;
        if (transform.offsetScaleY != 0.0f) transformJson["scaleY"] = transform.offsetScaleY;
        if (transform.offsetShearY != 0.0f) transformJson["shearY"] = transform.offsetShearY;
        if (transform.relative) transformJson["relative"] = transform.relative;
        if (transform.local) transformJson["local"] = transform.local;
        j["transform"].push_back(transformJson);
    }

    /* Path constraints */
    for (const auto& path : skeletonData.pathConstraints) {
        Json pathJson = Json::object();
        if (path.name) pathJson["name"] = path.name;
        pathJson["order"] = path.order;
        if (!path.bones.empty()) pathJson["bones"] = path.bones;
        if (path.target) pathJson["target"] = path.target;
        if (path.positionMode != PositionMode::PositionMode_Percent) pathJson["positionMode"] = positionModeString.at(path.positionMode);
        if (path.spacingMode != SpacingMode::SpacingMode_Length) pathJson["spacingMode"] = spacingModeString.at(path.spacingMode);
        if (path.rotateMode != RotateMode::RotateMode_Tangent) pathJson["rotateMode"] = rotateModeString.at(path.rotateMode);
        if (path.offsetRotation != 0.0f) pathJson["rotation"] = path.offsetRotation;
        if (path.position != 0.0f) pathJson["position"] = path.position;
        if (path.spacing != 0.0f) pathJson["spacing"] = path.spacing;
        if (path.mixRotate != 1.0f) pathJson["rotateMix"] = path.mixRotate;
        if (path.mixX != 1.0f) pathJson["translateMix"] = path.mixX;
        j["path"].push_back(pathJson);
    }

    /* Skins */
    for (const auto& skin : skeletonData.skins) {
        Json skinJson = Json::object();
        if (!skin.attachments.empty()) {
            for (const auto& [slotName, slotMap] : skin.attachments) {
                for (const auto& [attachmentName, attachmentMap] : slotMap) {
                    Json attachmentJson = Json::object();
                    if (attachmentMap.name != attachmentName) attachmentJson["name"] = attachmentMap.name;
                    if (attachmentMap.path != attachmentMap.name) attachmentJson["path"] = attachmentMap.path;
                    if (attachmentMap.type != AttachmentType::AttachmentType_Region) attachmentJson["type"] = attachmentTypeString.at(attachmentMap.type);
                    switch (attachmentMap.type) {
                        case AttachmentType_Region: {
                            const auto& region = std::get<RegionAttachment>(attachmentMap.data);
                            if (region.x != 0.0f) attachmentJson["x"] = region.x;
                            if (region.y != 0.0f) attachmentJson["y"] = region.y;
                            if (region.rotation != 0.0f) attachmentJson["rotation"] = region.rotation;
                            if (region.scaleX != 1.0f) attachmentJson["scaleX"] = region.scaleX;
                            if (region.scaleY != 1.0f) attachmentJson["scaleY"] = region.scaleY;
                            attachmentJson["width"] = region.width;
                            attachmentJson["height"] = region.height;
                            if (region.color) attachmentJson["color"] = colorToString(region.color.value(), true);
                            break;
                        }
                        case AttachmentType_Mesh: {
                            const auto& mesh = std::get<MeshAttachment>(attachmentMap.data);
                            attachmentJson["width"] = mesh.width;
                            attachmentJson["height"] = mesh.height;
                            if (mesh.color) attachmentJson["color"] = colorToString(mesh.color.value(), true);
                            if (mesh.hullLength != 0) attachmentJson["hull"] = mesh.hullLength;
                            if (!mesh.triangles.empty()) attachmentJson["triangles"] = mesh.triangles;
                            if (!mesh.edges.empty()) attachmentJson["edges"] = mesh.edges;
                            if (!mesh.uvs.empty()) attachmentJson["uvs"] = mesh.uvs;
                            if (!mesh.vertices.empty()) attachmentJson["vertices"] = mesh.vertices;
                            break;
                        }
                        case AttachmentType_Linkedmesh: {
                            const auto& linkedMesh = std::get<LinkedmeshAttachment>(attachmentMap.data);
                            attachmentJson["width"] = linkedMesh.width;
                            attachmentJson["height"] = linkedMesh.height;
                            if (linkedMesh.color) attachmentJson["color"] = colorToString(linkedMesh.color.value(), true);
                            attachmentJson["parent"] = linkedMesh.parentMesh; 
                            if (linkedMesh.timelines != 1) attachmentJson["deform"] = linkedMesh.timelines;
                            if (linkedMesh.skin) attachmentJson["skin"] = linkedMesh.skin.value();
                            break; 
                        }
                        case AttachmentType_Boundingbox: {
                            const auto& boundingBox = std::get<BoundingboxAttachment>(attachmentMap.data);
                            if (boundingBox.vertexCount != 0) attachmentJson["vertexCount"] = boundingBox.vertexCount;
                            if (boundingBox.color) attachmentJson["color"] = colorToString(boundingBox.color.value(), true);
                            if (!boundingBox.vertices.empty()) attachmentJson["vertices"] = boundingBox.vertices;
                            break;
                        }
                        case AttachmentType_Path: {
                            const auto& path = std::get<PathAttachment>(attachmentMap.data);
                            if (path.vertexCount != 0) attachmentJson["vertexCount"] = path.vertexCount;
                            if (path.closed) attachmentJson["closed"] = path.closed;
                            if (!path.constantSpeed) attachmentJson["constantSpeed"] = path.constantSpeed;
                            if (path.color) attachmentJson["color"] = colorToString(path.color.value(), true);
                            if (!path.vertices.empty()) attachmentJson["vertices"] = path.vertices;
                            if (!path.lengths.empty()) attachmentJson["lengths"] = path.lengths;
                            break;
                        }
                        case AttachmentType_Point: {
                            const auto& point = std::get<PointAttachment>(attachmentMap.data);
                            if (point.x != 0.0f) attachmentJson["x"] = point.x;
                            if (point.y != 0.0f) attachmentJson["y"] = point.y;
                            if (point.rotation != 0.0f) attachmentJson["rotation"] = point.rotation;
                            if (point.color) attachmentJson["color"] = colorToString(point.color.value(), true);
                            break;
                        }
                        case AttachmentType_Clipping: {
                            const auto& clipping = std::get<ClippingAttachment>(attachmentMap.data);
                            if (clipping.vertexCount != 0) attachmentJson["vertexCount"] = clipping.vertexCount;
                            if (clipping.endSlot) attachmentJson["end"] = clipping.endSlot;
                            if (clipping.color) attachmentJson["color"] = colorToString(clipping.color.value(), true);
                            if (!clipping.vertices.empty()) attachmentJson["vertices"] = clipping.vertices;
                            break;
                        }
                    }
                    skinJson[slotName][attachmentName] = attachmentJson; 
                }
            }
        }
        j["skins"][skin.name] = skinJson;
    }

    /* Events */
    for (const auto& event : skeletonData.events) {
        Json eventJson = Json::object();
        if (event.intValue != 0) eventJson["int"] = event.intValue;
        if (event.floatValue != 0.0f) eventJson["float"] = event.floatValue;
        if (event.stringValue) eventJson["string"] = event.stringValue.value();
        j["events"][event.name] = eventJson;
    }

    /* Animations */
    for (const auto& animation : skeletonData.animations) {
        Json animationJson = Json::object();
        if (!animation.slots.empty()) {
            for (const auto& [slotName, slotMap] : animation.slots) {
                Json slotJson = Json::object();
                if (slotMap.contains("attachment")) {
                    for (const auto& frame : slotMap.at("attachment")) {
                        Json frameJson = Json::object();
                        frameJson["time"] = frame.time;
                        if (frame.str1) frameJson["name"] = frame.str1.value();
                        else frameJson["name"] = nullptr;
                        slotJson["attachment"].push_back(frameJson);
                    }
                }
                if (slotMap.contains("rgba") || slotMap.contains("rgb")) {
                    for (const auto& frame : slotMap.contains("rgba") ? slotMap.at("rgba") : slotMap.at("rgb")) {
                        Json frameJson = Json::object();
                        frameJson["time"] = frame.time;
                        if (frame.color1) frameJson["color"] = colorToString(frame.color1.value(), true);
                        writeCurve(frame, frameJson);
                        slotJson["color"].push_back(frameJson);
                    }
                }
                if (slotMap.contains("rgba2") || slotMap.contains("rgb2")) {
                    for (const auto& frame : slotMap.contains("rgba2") ? slotMap.at("rgba2") : slotMap.at("rgb2")) {
                        Json frameJson = Json::object();
                        frameJson["time"] = frame.time;
                        if (frame.color1) frameJson["light"] = colorToString(frame.color1.value(), true);
                        if (frame.color2) frameJson["dark"] = colorToString(frame.color2.value(), false);
                        writeCurve(frame, frameJson);
                        slotJson["twoColor"].push_back(frameJson);
                    }
                }
                animationJson["slots"][slotName] = slotJson;
            }
        }
        if (!animation.bones.empty()) {
            for (const auto& [boneName, boneMap] : animation.bones) {
                Json boneJson = Json::object();
                if (boneMap.contains("rotate")) {
                    writeTimeline(boneMap.at("rotate"), boneJson["rotate"], 1, "angle", "", 0.0f);
                }
                if (boneMap.contains("translate")) {
                    writeTimeline(boneMap.at("translate"), boneJson["translate"], 2, "x", "y", 0.0f);
                }
                if (boneMap.contains("scale")) {
                    writeTimeline(boneMap.at("scale"), boneJson["scale"], 2, "x", "y", 0.0f);
                }
                if (boneMap.contains("shear")) {
                    writeTimeline(boneMap.at("shear"), boneJson["shear"], 2, "x", "y", 0.0f);
                }
                animationJson["bones"][boneName] = boneJson;
            }
        }
        if (!animation.ik.empty()) {
            for (const auto& [ikName, ikTimeline] : animation.ik) {
                Json ikJson = Json::array();
                for (const auto& frame : ikTimeline) {
                    Json frameJson = Json::object();
                    frameJson["time"] = frame.time;
                    if (frame.value1 != 1.0f) frameJson["mix"] = frame.value1;
                    if (!frame.bendPositive) frameJson["bendPositive"] = frame.bendPositive;
                    writeCurve(frame, frameJson);
                    ikJson.push_back(frameJson);
                }
                animationJson["ik"][ikName] = ikJson;
            }
        }
        if (!animation.transform.empty()) {
            for (const auto& [transformName, transformTimeline] : animation.transform) {
                Json transformJson = Json::array();
                for (const auto& frame : transformTimeline) {
                    Json frameJson = Json::object();
                    frameJson["time"] = frame.time;
                    if (frame.value1 != 1.0f) frameJson["rotateMix"] = frame.value1;
                    if (frame.value2 != 1.0f) frameJson["translateMix"] = frame.value2;
                    if (frame.value4 != 1.0f) frameJson["scaleMix"] = frame.value4;
                    if (frame.value6 != 1.0f) frameJson["shearMix"] = frame.value6;
                    writeCurve(frame, frameJson);
                    transformJson.push_back(frameJson);
                }
                animationJson["transform"][transformName] = transformJson;
            }
        }
        if (!animation.path.empty()) {
            for (const auto& [pathName, pathMap] : animation.path) {
                Json pathJson = Json::object();
                if (pathMap.contains("position")) {
                    writeTimeline(pathMap.at("position"), pathJson["position"], 1, "position", "", 0.0f);
                }
                if (pathMap.contains("spacing")) {
                    writeTimeline(pathMap.at("spacing"), pathJson["spacing"], 1, "spacing", "", 0.0f);
                }
                if (pathMap.contains("mix")) {
                    for (const auto& frame : pathMap.at("mix")) {
                        Json frameJson = Json::object();
                        if (frame.time != 0.0f) frameJson["time"] = frame.time;
                        if (frame.value1 != 1.0f) frameJson["rotateMix"] = frame.value1;
                        if (frame.value2 != 1.0f) frameJson["translateMix"] = frame.value2; 
                        writeCurve(frame, frameJson);
                        pathJson["mix"].push_back(frameJson);
                    }
                }
                animationJson["paths"][pathName] = pathJson;
            }
        }
        if (!animation.attachments.empty()) {
            for (const auto& [skinName, skinMap] : animation.attachments) {
                for (const auto& [slotName, slotMap] : skinMap) {
                    for (const auto& [attachmentName, attachmentTimeline] : slotMap) {
                        if (!attachmentTimeline.contains("deform")) continue;
                        Json attachmentJson = Json::array(); 
                        for (const auto& frame : attachmentTimeline.at("deform")) {
                            Json frameJson = Json::object();
                            frameJson["time"] = frame.time;
                            if (!frame.vertices.empty()) {
                                if (frame.int1 != 0) frameJson["offset"] = frame.int1; 
                                frameJson["vertices"] = frame.vertices; 
                            }
                            writeCurve(frame, frameJson); 
                            attachmentJson.push_back(frameJson);
                        }
                        animationJson["deform"][skinName][slotName][attachmentName] = attachmentJson;
                    }
                }
            }
        }
        if (!animation.drawOrder.empty()) {
            for (const auto& frame : animation.drawOrder) {
                Json frameJson = Json::object();
                frameJson["time"] = frame.time;
                if (!frame.offsets.empty()) {
                    for (const auto& [slot, offset] : frame.offsets) {
                        Json offsetJson = Json::object();
                        offsetJson["slot"] = slot;
                        offsetJson["offset"] = offset;
                        frameJson["offsets"].push_back(offsetJson);
                    }
                }
                animationJson["drawOrder"].push_back(frameJson);
            }
        }
        if (!animation.events.empty()) {
            for (const auto& frame : animation.events) {
                Json frameJson = Json::object(); 
                frameJson["time"] = frame.time; 
                if (frame.str1) frameJson["name"] = frame.str1; 
                int eventIndex = -1; 
                for (size_t i = 0; i < skeletonData.events.size(); i++) {
                    if (skeletonData.events[i].name == frame.str1) {
                        eventIndex = static_cast<int>(i);
                        break;
                    }
                }
                EventData eventData = skeletonData.events[eventIndex];
                if (frame.int1 != eventData.intValue) frameJson["int"] = frame.int1;
                if (frame.value1 != eventData.floatValue) frameJson["float"] = frame.value1;
                if (frame.str2) frameJson["string"] = frame.str2;
                animationJson["events"].push_back(frameJson);
            }
        }
        j["animations"][animation.name] = animationJson;
    }

    return j;
}

}
