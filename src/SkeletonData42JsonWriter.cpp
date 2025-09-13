#include "SkeletonData42.h"
using namespace spine42; 

static const std::map<spine42::Inherit, std::string> inheritString = {
    { Inherit_Normal, "normal" },
    { Inherit_OnlyTranslation, "onlyTranslation" },
    { Inherit_NoRotationOrReflection, "noRotationOrReflection" },
    { Inherit_NoScale, "noScale" },
    { Inherit_NoScaleOrReflection, "noScaleOrReflection" }
};

static const std::map<spine42::BlendMode, std::string> blendModeString = {
    { BlendMode_Normal, "normal" },
    { BlendMode_Additive, "additive" },
    { BlendMode_Multiply, "multiply" },
    { BlendMode_Screen, "screen" }
};

static const std::map<spine42::PositionMode, std::string> positionModeString = {
    { PositionMode_Fixed, "fixed" },
    { PositionMode_Percent, "percent" }
};

static const std::map<spine42::SpacingMode, std::string> spacingModeString = {
    { SpacingMode_Length, "length" },
    { SpacingMode_Fixed, "fixed" },
    { SpacingMode_Percent, "percent" },
    { SpacingMode_Proportional, "proportional" }
};

static const std::map<spine42::RotateMode, std::string> rotateModeString = {
    { RotateMode_Tangent, "tangent" },
    { RotateMode_Chain, "chain" },
    { RotateMode_ChainScale, "chainScale" }
};

static const std::map<spine42::AttachmentType, std::string> attachmentTypeString = {
    { AttachmentType_Region, "region" },
    { AttachmentType_Boundingbox, "boundingbox" },
    { AttachmentType_Mesh, "mesh" },
    { AttachmentType_Linkedmesh, "linkedmesh" },
    { AttachmentType_Path, "path" },
    { AttachmentType_Point, "point" },
    { AttachmentType_Clipping, "clipping" }
};

static const std::map<spine42::SequenceMode, std::string> sequenceModeString = {
    { SequenceMode::hold, "hold" },
    { SequenceMode::once, "once" },
    { SequenceMode::loop, "loop" },
    { SequenceMode::pingpong, "pingpong" },
    { SequenceMode::onceReverse, "onceReverse" },
    { SequenceMode::loopReverse, "loopReverse" },
    { SequenceMode::pingpongReverse, "pingpongReverse" }
};

std::string colorToString(const Color& color, bool hasAlpha) {
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

Json writeSequence(const Sequence& sequence) {
    Json j = Json::object();
    if (sequence.count != 0) j["count"] = sequence.count;
    if (sequence.start != 1) j["start"] = sequence.start;
    if (sequence.digits != 0) j["digits"] = sequence.digits;
    if (sequence.setupIndex != 0) j["setupIndex"] = sequence.setupIndex;
    return j;
}

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
        if (frame.time != 0.0f) frameJson["time"] = frame.time;
        if (frame.value1 != defaultValue) frameJson[key1] = frame.value1;
        if (valueNum > 1 && frame.value2 != defaultValue) frameJson[key2] = frame.value2;
        writeCurve(frame, frameJson);
        j.push_back(frameJson);
    }
}

Json spine42::writeJsonData(const SkeletonData& skeletonData) {
    Json j = Json::object();

    Json skeleton = Json::object();
    if (skeletonData.hash != 0) skeleton["hash"] = uint64ToBase64(skeletonData.hash);
    if (skeletonData.version) skeleton["spine"] = skeletonData.version;
    skeleton["x"] = skeletonData.x;
    skeleton["y"] = skeletonData.y;
    skeleton["width"] = skeletonData.width;
    skeleton["height"] = skeletonData.height;
    if (skeletonData.referenceScale != 100.0f) skeleton["referenceScale"] = skeletonData.referenceScale;
    if (skeletonData.nonessential) {
        if (skeletonData.fps != 30.0f) skeleton["fps"] = skeletonData.fps;
        if(skeletonData.imagesPath) skeleton["images"] = skeletonData.imagesPath;
        if(skeletonData.audioPath) skeleton["audio"] = skeletonData.audioPath;
    }
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
        if (bone.inherit != Inherit_Normal) boneJson["inherit"] = inheritString.at(bone.inherit);
        if (bone.skinRequired) boneJson["skin"] = bone.skinRequired;
        if (bone.color) boneJson["color"] = colorToString(bone.color.value(), true);
        if (bone.icon && bone.icon != "") boneJson["icon"] = bone.icon;
        if (!bone.visible) boneJson["visible"] = bone.visible;
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
        if (!slot.visible) slotJson["visible"] = slot.visible;
        j["slots"].push_back(slotJson);
    }

    /* IK constraints */
    for (const auto& ik : skeletonData.ikConstraints) {
        Json ikJson = Json::object();
        if (ik.name) ikJson["name"] = ik.name;
        if (ik.order != 0) ikJson["order"] = ik.order;
        if (ik.skinRequired) ikJson["skin"] = ik.skinRequired;
        if (!ik.bones.empty()) ikJson["bones"] = ik.bones;
        if (ik.target) ikJson["target"] = ik.target;
        if (ik.mix != 1.0f) ikJson["mix"] = ik.mix;
        if (ik.softness != 0.0f) ikJson["softness"] = ik.softness;
        if (!ik.bendPositive) ikJson["bendPositive"] = ik.bendPositive;
        if (ik.compress) ikJson["compress"] = ik.compress;
        if (ik.stretch) ikJson["stretch"] = ik.stretch;
        if (ik.uniform) ikJson["uniform"] = ik.uniform;
        j["ik"].push_back(ikJson);
    }

    /* Transform constraints */
    for (const auto& transform : skeletonData.transformConstraints) {
        Json transformJson = Json::object();
        if (transform.name) transformJson["name"] = transform.name;
        if (transform.order != 0) transformJson["order"] = transform.order;
        if (transform.skinRequired) transformJson["skin"] = transform.skinRequired;
        if (!transform.bones.empty()) transformJson["bones"] = transform.bones;
        if (transform.target) transformJson["target"] = transform.target;
        if (transform.mixRotate != 1.0f) transformJson["mixRotate"] = transform.mixRotate;
        if (transform.mixX != 1.0f) transformJson["mixX"] = transform.mixX;
        if (transform.mixY != transform.mixX) transformJson["mixY"] = transform.mixY;
        if (transform.mixScaleX != 1.0f) transformJson["mixScaleX"] = transform.mixScaleX;
        if (transform.mixScaleY != transform.mixScaleX) transformJson["mixScaleY"] = transform.mixScaleY;
        if (transform.mixShearY != 1.0f) transformJson["mixShearY"] = transform.mixShearY;
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
        if (path.order != 0) pathJson["order"] = path.order;
        if (path.skinRequired) pathJson["skin"] = path.skinRequired;
        if (!path.bones.empty()) pathJson["bones"] = path.bones;
        if (path.target) pathJson["target"] = path.target;
        if (path.positionMode != PositionMode::PositionMode_Percent) pathJson["positionMode"] = positionModeString.at(path.positionMode);
        if (path.spacingMode != SpacingMode::SpacingMode_Length) pathJson["spacingMode"] = spacingModeString.at(path.spacingMode);
        if (path.rotateMode != RotateMode::RotateMode_Tangent) pathJson["rotateMode"] = rotateModeString.at(path.rotateMode);
        if (path.offsetRotation != 0.0f) pathJson["rotation"] = path.offsetRotation;
        if (path.position != 0.0f) pathJson["position"] = path.position;
        if (path.spacing != 0.0f) pathJson["spacing"] = path.spacing;
        if (path.mixRotate != 1.0f) pathJson["mixRotate"] = path.mixRotate;
        if (path.mixX != 1.0f) pathJson["mixX"] = path.mixX;
        if (path.mixY != path.mixX) pathJson["mixY"] = path.mixY;
        j["path"].push_back(pathJson);
    }

    /* Physics constraints */
    for (const auto& physics : skeletonData.physicsConstraints) {
        Json physicsJson = Json::object();
        if (physics.name) physicsJson["name"] = physics.name;
        if (physics.order != 0) physicsJson["order"] = physics.order;
        if (physics.skinRequired) physicsJson["skin"] = physics.skinRequired;
        if (physics.bone) physicsJson["bone"] = physics.bone;
        if (physics.x != 0.0f) physicsJson["x"] = physics.x;
        if (physics.y != 0.0f) physicsJson["y"] = physics.y;
        if (physics.rotate != 0.0f) physicsJson["rotate"] = physics.rotate;
        if (physics.scaleX != 0.0f) physicsJson["scaleX"] = physics.scaleX;
        if (physics.shearX != 0.0f) physicsJson["shearX"] = physics.shearX;
        if (physics.limit != 5000.0f) physicsJson["limit"] = physics.limit;
        if (physics.fps != 60.0f) physicsJson["fps"] = physics.fps;
        if (physics.inertia != 1.0f) physicsJson["inertia"] = physics.inertia;
        if (physics.strength != 100.0f) physicsJson["strength"] = physics.strength;
        if (physics.damping != 1.0f) physicsJson["damping"] = physics.damping;
        if (physics.mass != 1.0f) physicsJson["mass"] = physics.mass;
        if (physics.wind != 0.0f) physicsJson["wind"] = physics.wind;
        if (physics.gravity != 0.0f) physicsJson["gravity"] = physics.gravity;
        if (physics.mix != 1.0f) physicsJson["mix"] = physics.mix;
        if (physics.inertiaGlobal) physicsJson["inertiaGlobal"] = physics.inertiaGlobal;
        if (physics.strengthGlobal) physicsJson["strengthGlobal"] = physics.strengthGlobal;
        if (physics.dampingGlobal) physicsJson["dampingGlobal"] = physics.dampingGlobal;
        if (physics.massGlobal) physicsJson["massGlobal"] = physics.massGlobal;
        if (physics.windGlobal) physicsJson["windGlobal"] = physics.windGlobal;
        if (physics.gravityGlobal) physicsJson["gravityGlobal"] = physics.gravityGlobal;
        if (physics.mixGlobal) physicsJson["mixGlobal"] = physics.mixGlobal;
        j["physics"].push_back(physicsJson);
    }

    /* Skins */
    for (const auto& skin : skeletonData.skins) {
        Json skinJson = Json::object();
        skinJson["name"] = skin.name; 
        if (!skin.bones.empty()) skinJson["bones"] = skin.bones;
        if (!skin.ik.empty()) skinJson["ik"] = skin.ik;
        if (!skin.transform.empty()) skinJson["transform"] = skin.transform;
        if (!skin.path.empty()) skinJson["path"] = skin.path;
        if (!skin.physics.empty()) skinJson["physics"] = skin.physics;
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
                            if (region.sequence) attachmentJson["sequence"] = writeSequence(region.sequence.value());
                            break;
                        }
                        case AttachmentType_Mesh: {
                            const auto& mesh = std::get<MeshAttachment>(attachmentMap.data);
                            attachmentJson["width"] = mesh.width;
                            attachmentJson["height"] = mesh.height;
                            if (mesh.color) attachmentJson["color"] = colorToString(mesh.color.value(), true);
                            if (mesh.sequence) attachmentJson["sequence"] = writeSequence(mesh.sequence.value());
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
                            if (linkedMesh.sequence) attachmentJson["sequence"] = writeSequence(linkedMesh.sequence.value());
                            attachmentJson["parent"] = linkedMesh.parentMesh; 
                            if (linkedMesh.timelines != 1) attachmentJson["timelines"] = linkedMesh.timelines;
                            attachmentJson["skin"] = linkedMesh.skin;
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
                    skinJson["attachments"][slotName][attachmentName] = attachmentJson; 
                }
            }
        }
        j["skins"].push_back(skinJson);
    }

    /* Events */
    for (const auto& event : skeletonData.events) {
        Json eventJson = Json::object();
        if (event.intValue != 0) eventJson["int"] = event.intValue;
        if (event.floatValue != 0.0f) eventJson["float"] = event.floatValue;
        if (event.stringValue) eventJson["string"] = event.stringValue.value();
        if (event.audioPath) {
            eventJson["audio"] = event.audioPath.value();
            if (event.volume != 1.0f) eventJson["volume"] = event.volume;
            if (event.balance != 0.0f) eventJson["balance"] = event.balance;
        }
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
                        if (frame.time != 0.0f) frameJson["time"] = frame.time;
                        if (frame.str1) frameJson["name"] = frame.str1.value();
                        slotJson["attachment"].push_back(frameJson);
                    }
                }
                if (slotMap.contains("rgba")) {
                    for (const auto& frame : slotMap.at("rgba")) {
                        Json frameJson = Json::object();
                        if (frame.time != 0.0f) frameJson["time"] = frame.time;
                        if (frame.color1) frameJson["color"] = colorToString(frame.color1.value(), true);
                        writeCurve(frame, frameJson);
                        slotJson["rgba"].push_back(frameJson);
                    }
                }
                if (slotMap.contains("rgb")) {
                    for (const auto& frame : slotMap.at("rgb")) {
                        Json frameJson = Json::object();
                        if (frame.time != 0.0f) frameJson["time"] = frame.time;
                        if (frame.color1) frameJson["color"] = colorToString(frame.color1.value(), false);
                        writeCurve(frame, frameJson);
                        slotJson["rgb"].push_back(frameJson);
                    }
                }
                if (slotMap.contains("alpha")) {
                    writeTimeline(slotMap.at("alpha"), slotJson["alpha"], 1, "value", "", 0.0f);
                }
                if (slotMap.contains("rgba2")) {
                    for (const auto& frame : slotMap.at("rgba2")) {
                        Json frameJson = Json::object();
                        if (frame.time != 0.0f) frameJson["time"] = frame.time;
                        if (frame.color1) frameJson["light"] = colorToString(frame.color1.value(), true);
                        if (frame.color2) frameJson["dark"] = colorToString(frame.color2.value(), false);
                        writeCurve(frame, frameJson);
                        slotJson["rgba2"].push_back(frameJson);
                    }
                }
                if (slotMap.contains("rgb2")) {
                    for (const auto& frame : slotMap.at("rgb2")) {
                        Json frameJson = Json::object();
                        if (frame.time != 0.0f) frameJson["time"] = frame.time;
                        if (frame.color1) frameJson["light"] = colorToString(frame.color1.value(), false);
                        if (frame.color2) frameJson["dark"] = colorToString(frame.color2.value(), false);
                        writeCurve(frame, frameJson);
                        slotJson["rgb2"].push_back(frameJson);
                    }
                }
                animationJson["slots"][slotName] = slotJson;
            }
        }
        if (!animation.bones.empty()) {
            for (const auto& [boneName, boneMap] : animation.bones) {
                Json boneJson = Json::object();
                if (boneMap.contains("rotate")) {
                    writeTimeline(boneMap.at("rotate"), boneJson["rotate"], 1, "value", "", 0.0f);
                }
                if (boneMap.contains("translate")) {
                    writeTimeline(boneMap.at("translate"), boneJson["translate"], 2, "x", "y", 0.0f);
                }
                if (boneMap.contains("translatex")) {
                    writeTimeline(boneMap.at("translatex"), boneJson["translatex"], 1, "value", "", 0.0f);
                }
                if (boneMap.contains("translatey")) {
                    writeTimeline(boneMap.at("translatey"), boneJson["translatey"], 1, "value", "", 0.0f);
                }
                if (boneMap.contains("scale")) {
                    writeTimeline(boneMap.at("scale"), boneJson["scale"], 2, "x", "y", 1.0f);
                }
                if (boneMap.contains("scalex")) {
                    writeTimeline(boneMap.at("scalex"), boneJson["scalex"], 1, "value", "", 1.0f);
                }
                if (boneMap.contains("scaley")) {
                    writeTimeline(boneMap.at("scaley"), boneJson["scaley"], 1, "value", "", 1.0f);
                }
                if (boneMap.contains("shear")) {
                    writeTimeline(boneMap.at("shear"), boneJson["shear"], 2, "x", "y", 0.0f);
                }
                if (boneMap.contains("shearx")) {
                    writeTimeline(boneMap.at("shearx"), boneJson["shearx"], 1, "value", "", 0.0f);
                }
                if (boneMap.contains("sheary")) {
                    writeTimeline(boneMap.at("sheary"), boneJson["sheary"], 1, "value", "", 0.0f);
                }
                if (boneMap.contains("inherit")) {
                    for (const auto& frame : boneMap.at("inherit")) {
                        Json frameJson = Json::object();
                        if (frame.time != 0.0f) frameJson["time"] = frame.time;
                        if (frame.inherit != Inherit_Normal) frameJson["inherit"] = inheritString.at(frame.inherit);
                        boneJson["inherit"].push_back(frameJson);
                    }
                }
                animationJson["bones"][boneName] = boneJson;
            }
        }
        if (!animation.ik.empty()) {
            for (const auto& [ikName, ikTimeline] : animation.ik) {
                Json ikJson = Json::array();
                for (const auto& frame : ikTimeline) {
                    Json frameJson = Json::object();
                    if (frame.time != 0.0f) frameJson["time"] = frame.time;
                    if (frame.value1 != 1.0f) frameJson["mix"] = frame.value1;
                    if (frame.value2 != 0.0f) frameJson["softness"] = frame.value2;
                    if (!frame.bendPositive) frameJson["bendPositive"] = frame.bendPositive;
                    if (frame.compress) frameJson["compress"] = frame.compress;
                    if (frame.stretch) frameJson["stretch"] = frame.stretch;
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
                    if (frame.time != 0.0f) frameJson["time"] = frame.time;
                    if (frame.value1 != 1.0f) frameJson["mixRotate"] = frame.value1;
                    if (frame.value2 != 1.0f) frameJson["mixX"] = frame.value2;
                    if (frame.value3 != frame.value2) frameJson["mixY"] = frame.value3;
                    if (frame.value4 != 1.0f) frameJson["mixScaleX"] = frame.value4;
                    if (frame.value5 != frame.value4) frameJson["mixScaleY"] = frame.value5;
                    if (frame.value6 != 1.0f) frameJson["mixShearY"] = frame.value6;
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
                    writeTimeline(pathMap.at("position"), pathJson["position"], 1, "value", "", 0.0f);
                }
                if (pathMap.contains("spacing")) {
                    writeTimeline(pathMap.at("spacing"), pathJson["spacing"], 1, "value", "", 0.0f);
                }
                if (pathMap.contains("mix")) {
                    for (const auto& frame : pathMap.at("mix")) {
                        Json frameJson = Json::object();
                        if (frame.time != 0.0f) frameJson["time"] = frame.time;
                        if (frame.value1 != 1.0f) frameJson["mixRotate"] = frame.value1;
                        if (frame.value2 != 1.0f) frameJson["mixX"] = frame.value2;
                        if (frame.value3 != frame.value2) frameJson["mixY"] = frame.value3; 
                        writeCurve(frame, frameJson);
                        pathJson["mix"].push_back(frameJson);
                    }
                }
                animationJson["path"][pathName] = pathJson;
            }
        }
        if (!animation.physics.empty()) {
            for (const auto& [physicsName, physicsMap] : animation.physics) {
                Json physicsJson = Json::object();
                if (physicsMap.contains("reset")) {
                    for (const auto& frame : physicsMap.at("reset")) {
                        Json frameJson = Json::object();
                        if (frame.time != 0.0f) frameJson["time"] = frame.time;
                        physicsJson["reset"].push_back(frameJson);
                    }
                }
                if (physicsMap.contains("inertia")) {
                    writeTimeline(physicsMap.at("inertia"), physicsJson["inertia"], 1, "value", "", 0.0f);
                }
                if (physicsMap.contains("strength")) {
                    writeTimeline(physicsMap.at("strength"), physicsJson["strength"], 1, "value", "", 0.0f);
                }
                if (physicsMap.contains("damping")) {
                    writeTimeline(physicsMap.at("damping"), physicsJson["damping"], 1, "value", "", 0.0f);
                }
                if (physicsMap.contains("mass")) {
                    writeTimeline(physicsMap.at("mass"), physicsJson["mass"], 1, "value", "", 0.0f);
                }
                if (physicsMap.contains("wind")) {
                    writeTimeline(physicsMap.at("wind"), physicsJson["wind"], 1, "value", "", 0.0f);
                }
                if (physicsMap.contains("gravity")) {
                    writeTimeline(physicsMap.at("gravity"), physicsJson["gravity"], 1, "value", "", 0.0f);
                }
                if (physicsMap.contains("mix")) {
                    writeTimeline(physicsMap.at("mix"), physicsJson["mix"], 1, "value", "", 0.0f);
                }
                animationJson["physics"][physicsName] = physicsJson;
            }
        }
        if (!animation.attachments.empty()) {
            for (const auto& [skinName, skinMap] : animation.attachments) {
                for (const auto& [slotName, slotMap] : skinMap) {
                    for (const auto& [attachmentName, attachmentMap] : slotMap) {
                        Json attachmentJson = Json::object();
                        if (attachmentMap.contains("deform")) {
                            for (const auto& frame : attachmentMap.at("deform")) {
                                Json frameJson = Json::object();
                                if (frame.time != 0.0f) frameJson["time"] = frame.time;
                                if (!frame.vertices.empty()) {
                                    if (frame.int1 != 0) frameJson["offset"] = frame.int1; 
                                    frameJson["vertices"] = frame.vertices; 
                                }
                                writeCurve(frame, frameJson); 
                                attachmentJson["deform"].push_back(frameJson);
                            }
                        }
                        if (attachmentMap.contains("sequence")) {
                            float lastDelay = 0.0f; 
                            for (const auto& frame : attachmentMap.at("sequence")) {
                                Json frameJson = Json::object();
                                if (frame.time != 0.0f) frameJson["time"] = frame.time;
                                if (frame.value1 != lastDelay) frameJson["delay"] = frame.value1; 
                                lastDelay = frame.value1;
                                if (frame.int1 != 0) frameJson["index"] = frame.int1;
                                if (frame.sequenceMode != SequenceMode::hold) frameJson["mode"] = sequenceModeString.at(frame.sequenceMode);
                                attachmentJson["sequence"].push_back(frameJson);
                            }
                        }
                        animationJson["attachments"][skinName][slotName][attachmentName] = attachmentJson;
                    }
                }
            }
        }
        if (!animation.drawOrder.empty()) {
            for (const auto& frame : animation.drawOrder) {
                Json frameJson = Json::object();
                if (frame.time != 0.0f) frameJson["time"] = frame.time;
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
                if (frame.time != 0.0f) frameJson["time"] = frame.time; 
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
                if (eventData.audioPath) {
                    if (frame.value2 != 1.0f) frameJson["volume"] = frame.value2;
                    if (frame.value3 != 0.0f) frameJson["balance"] = frame.value3;
                }
                animationJson["events"].push_back(frameJson);
            }
        }
        j["animations"][animation.name] = animationJson;
    }

    return j;
}
