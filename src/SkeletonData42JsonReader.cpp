#include "SkeletonData.h"

namespace spine42 {

static const std::map<std::string, Inherit> inheritMap = {
    {"normal", Inherit_Normal},
    {"onlyTranslation", Inherit_OnlyTranslation},
    {"noRotationOrReflection", Inherit_NoRotationOrReflection},
    {"noScale", Inherit_NoScale},
    {"noScaleOrReflection", Inherit_NoScaleOrReflection}
};

static const std::map<std::string, BlendMode> blendModeMap = {
    {"normal", BlendMode_Normal},
    {"additive", BlendMode_Additive},
    {"multiply", BlendMode_Multiply},
    {"screen", BlendMode_Screen}
};

static const std::map<std::string, PositionMode> positionModeMap = {
    {"fixed", PositionMode_Fixed},
    {"percent", PositionMode_Percent}
};

static const std::map<std::string, SpacingMode> spacingModeMap = {
    {"length", SpacingMode_Length},
    {"fixed", SpacingMode_Fixed},
    {"percent", SpacingMode_Percent},
    {"proportional", SpacingMode_Proportional}
};

static const std::map<std::string, RotateMode> rotateModeMap = {
    {"tangent", RotateMode_Tangent},
    {"chain", RotateMode_Chain},
    {"chainScale", RotateMode_ChainScale}
};

static const std::map<std::string, AttachmentType> attachmentTypeMap = {
    {"region", AttachmentType_Region},
    {"boundingbox", AttachmentType_Boundingbox},
    {"mesh", AttachmentType_Mesh},
    {"linkedmesh", AttachmentType_Linkedmesh},
    {"path", AttachmentType_Path},
    {"point", AttachmentType_Point},
    {"clipping", AttachmentType_Clipping}
};

static const std::map<std::string, SequenceMode> sequenceModeMap = {
    {"hold", SequenceMode::hold},
    {"once", SequenceMode::once},
    {"loop", SequenceMode::loop},
    {"pingpong", SequenceMode::pingpong},
    {"onceReverse", SequenceMode::onceReverse},
    {"loopReverse", SequenceMode::loopReverse},
    {"pingpongReverse", SequenceMode::pingpongReverse}
};

Color stringToColor(const std::string& str, bool hasAlpha) {
    Color color;
    const char* value = str.c_str();
    auto parseHex = [](const char* hex, size_t index) -> unsigned char {
        if (index * 2 + 1 >= strlen(hex)) return 255;
        char digits[3] = {hex[index * 2], hex[index * 2 + 1], '\0'};
        return (unsigned char)strtoul(digits, nullptr, 16);
    };
    color.r = parseHex(value, 0);
    color.g = parseHex(value, 1);
    color.b = parseHex(value, 2);
    color.a = hasAlpha ? parseHex(value, 3) : 255;
    return color;
}

Sequence readSequence(const Json& j) {
    Sequence sequence;
    sequence.count = j.value("count", 0);
    sequence.start = j.value("start", 1);
    sequence.digits = j.value("digits", 0);
    sequence.setupIndex = j.value("setupIndex", 0);
    return sequence;
}

void readCurve(const Json& j, TimelineFrame& frame) {
    if (j.contains("curve")) {
        if (j["curve"] == "stepped") {
            frame.curveType = CurveType::CURVE_STEPPED;
        } else {
            frame.curveType = CURVE_BEZIER; 
            frame.curve = j["curve"].get<std::vector<float>>();
        }
    }
}

void readTimeline(const Json& j, Timeline& timeline, int valueNum, const std::string& key1, const std::string& key2, float defaultValue) {
    for (const auto& frameJson : j) {
        TimelineFrame frame;
        frame.time = frameJson.value("time", 0.0f);
        frame.value1 = frameJson.value(key1, defaultValue);
        if (valueNum > 1) frame.value2 = frameJson.value(key2, defaultValue);
        readCurve(frameJson, frame);
        timeline.push_back(frame);
    }
}

SkeletonData readJsonData(const Json& j) {
    SkeletonData skeletonData; 

    const auto& skeleton = j["skeleton"];
    skeletonData.hash = skeleton.contains("hash") ? base64ToUint64(skeleton["hash"]) : 0;
    if (skeleton.contains("spine")) skeletonData.version = skeleton["spine"];
    skeletonData.x = skeleton.value("x", 0.0f);
    skeletonData.y = skeleton.value("y", 0.0f);
    skeletonData.width = skeleton.value("width", 0.0f);
    skeletonData.height = skeleton.value("height", 0.0f);
    skeletonData.referenceScale = skeleton.value("referenceScale", 100.0f);
    skeletonData.fps = skeleton.value("fps", 30.0f);
    if (skeleton.contains("images")) skeletonData.imagesPath = skeleton["images"];
    if (skeleton.contains("audio")) skeletonData.audioPath = skeleton["audio"];
    skeletonData.nonessential = !(skeletonData.fps == 30.0f && skeletonData.imagesPath == std::nullopt && skeletonData.audioPath == std::nullopt);

    /* Bones */
    if (j.contains("bones")) {
        for (const auto& boneJson : j["bones"]) {
            BoneData boneData;
            if (boneJson.contains("name")) boneData.name = boneJson["name"];
            if (boneJson.contains("parent")) boneData.parent = boneJson["parent"];
            boneData.length = boneJson.value("length", 0.0f);
            boneData.x = boneJson.value("x", 0.0f);
            boneData.y = boneJson.value("y", 0.0f);
            boneData.rotation = boneJson.value("rotation", 0.0f);
            boneData.scaleX = boneJson.value("scaleX", 1.0f);
            boneData.scaleY = boneJson.value("scaleY", 1.0f);
            boneData.shearX = boneJson.value("shearX", 0.0f);
            boneData.shearY = boneJson.value("shearY", 0.0f);
            boneData.inherit = inheritMap.at(boneJson.value("inherit", "normal"));
            boneData.skinRequired = boneJson.value("skin", false);
            if (boneJson.contains("color")) boneData.color = stringToColor(boneJson["color"], true);
            boneData.icon = boneJson.value("icon", "");
            boneData.visible = boneJson.value("visible", true);
            skeletonData.bones.push_back(boneData);
        }
    }

    /* Slots */
    if (j.contains("slots")) {
        for (const auto& slotJson : j["slots"]) {
            SlotData slotData;
            if (slotJson.contains("name")) slotData.name = slotJson["name"];
            if (slotJson.contains("bone")) slotData.bone = slotJson["bone"];
            if (slotJson.contains("color")) slotData.color = stringToColor(slotJson["color"], true);
            if (slotJson.contains("dark")) slotData.darkColor = stringToColor(slotJson["dark"], false);
            if (slotJson.contains("attachment") && !slotJson["attachment"].is_null()) slotData.attachmentName = slotJson["attachment"];
            slotData.blendMode = blendModeMap.at(slotJson.value("blend", "normal"));
            slotData.visible = slotJson.value("visible", true);
            skeletonData.slots.push_back(slotData);
        }
    }

    /* IK constraints */
    if (j.contains("ik")) {
        for (const auto& ikJson : j["ik"]) {
            IKConstraintData ikData;
            if (ikJson.contains("name")) ikData.name = ikJson["name"];
            ikData.order = ikJson.value("order", 0);
            ikData.skinRequired = ikJson.value("skin", false);
            if (ikJson.contains("bones")) ikData.bones = ikJson["bones"].get<std::vector<std::string>>();
            if (ikJson.contains("target")) ikData.target = ikJson["target"];
            ikData.mix = ikJson.value("mix", 1.0f);
            ikData.softness = ikJson.value("softness", 0.0f);
            ikData.bendPositive = ikJson.value("bendPositive", true);
            ikData.compress = ikJson.value("compress", false);
            ikData.stretch = ikJson.value("stretch", false);
            ikData.uniform = ikJson.value("uniform", false);
            skeletonData.ikConstraints.push_back(ikData);
        }
    }

    /* Transform constraints */
    if (j.contains("transform")) {
        for (const auto& transformJson : j["transform"]) {
            TransformConstraintData transformData;
            if (transformJson.contains("name")) transformData.name = transformJson["name"];
            transformData.order = transformJson.value("order", 0);
            transformData.skinRequired = transformJson.value("skin", false);
            if (transformJson.contains("bones")) transformData.bones = transformJson["bones"].get<std::vector<std::string>>();
            if (transformJson.contains("target")) transformData.target = transformJson["target"];
            transformData.mixRotate = transformJson.value("mixRotate", 1.0f);
            transformData.mixX = transformJson.value("mixX", 1.0f);
            transformData.mixY = transformJson.value("mixY", transformData.mixX);
            transformData.mixScaleX = transformJson.value("mixScaleX", 1.0f);
            transformData.mixScaleY = transformJson.value("mixScaleY", transformData.mixScaleX);
            transformData.mixShearY = transformJson.value("mixShearY", 1.0f);
            transformData.offsetRotation = transformJson.value("rotation", 0.0f);
            transformData.offsetX = transformJson.value("x", 0.0f);
            transformData.offsetY = transformJson.value("y", 0.0f);
            transformData.offsetScaleX = transformJson.value("scaleX", 0.0f);
            transformData.offsetScaleY = transformJson.value("scaleY", 0.0f);
            transformData.offsetShearY = transformJson.value("shearY", 0.0f);
            transformData.relative = transformJson.value("relative", false);
            transformData.local = transformJson.value("local", false);
            skeletonData.transformConstraints.push_back(transformData);
        }
    }

    /* Path constraints */
    if (j.contains("path")) {
        for (const auto& pathJson : j["path"]) {
            PathConstraintData pathData;
            if (pathJson.contains("name")) pathData.name = pathJson["name"];
            pathData.order = pathJson.value("order", 0);
            pathData.skinRequired = pathJson.value("skin", false);
            if (pathJson.contains("bones")) pathData.bones = pathJson["bones"].get<std::vector<std::string>>();
            if (pathJson.contains("target")) pathData.target = pathJson["target"];
            pathData.positionMode = positionModeMap.at(pathJson.value("positionMode", "percent"));
            pathData.spacingMode = spacingModeMap.at(pathJson.value("spacingMode", "length"));
            pathData.rotateMode = rotateModeMap.at(pathJson.value("rotateMode", "tangent"));
            pathData.offsetRotation = pathJson.value("rotation", 0.0f);
            pathData.position = pathJson.value("position", 0.0f);
            pathData.spacing = pathJson.value("spacing", 0.0f);
            pathData.mixRotate = pathJson.value("mixRotate", 1.0f);
            pathData.mixX = pathJson.value("mixX", 1.0f);
            pathData.mixY = pathJson.value("mixY", pathData.mixX);
            skeletonData.pathConstraints.push_back(pathData);
        }
    }

    /* Physics constraints */
    if (j.contains("physics")) {
        for (const auto& physicsJson : j["physics"]) {
            PhysicsConstraintData physicsData;
            if (physicsJson.contains("name")) physicsData.name = physicsJson["name"];
            physicsData.order = physicsJson.value("order", 0);
            physicsData.skinRequired = physicsJson.value("skin", false);
            if (physicsJson.contains("bone")) physicsData.bone = physicsJson["bone"];
            physicsData.x = physicsJson.value("x", 0.0f);
            physicsData.y = physicsJson.value("y", 0.0f);
            physicsData.rotate = physicsJson.value("rotate", 0.0f);
            physicsData.scaleX = physicsJson.value("scaleX", 0.0f);
            physicsData.shearX = physicsJson.value("shearX", 0.0f);
            physicsData.limit = physicsJson.value("limit", 5000.0f);
            physicsData.fps = physicsJson.value("fps", 60.0f);
            physicsData.inertia = physicsJson.value("inertia", 1.0f);
            physicsData.strength = physicsJson.value("strength", 100.0f);
            physicsData.damping = physicsJson.value("damping", 1.0f);
            physicsData.mass = physicsJson.value("mass", 1.0f);
            physicsData.wind = physicsJson.value("wind", 0.0f);
            physicsData.gravity = physicsJson.value("gravity", 0.0f);
            physicsData.mix = physicsJson.value("mix", 1.0f);
            physicsData.inertiaGlobal = physicsJson.value("inertiaGlobal", false);
            physicsData.strengthGlobal = physicsJson.value("strengthGlobal", false);
            physicsData.dampingGlobal = physicsJson.value("dampingGlobal", false);
            physicsData.massGlobal = physicsJson.value("massGlobal", false);
            physicsData.windGlobal = physicsJson.value("windGlobal", false);
            physicsData.gravityGlobal = physicsJson.value("gravityGlobal", false);
            physicsData.mixGlobal = physicsJson.value("mixGlobal", false);
            skeletonData.physicsConstraints.push_back(physicsData);
        }
    }

    /* Skins */
    if (j.contains("skins")) {
        for (const auto& skinJson : j["skins"]) {
            Skin skinData;
            skinData.name = skinJson.value("name", "");
            if (skinJson.contains("bones")) skinData.bones = skinJson["bones"].get<std::vector<std::string>>();
            if (skinJson.contains("ik")) skinData.ik = skinJson["ik"].get<std::vector<std::string>>();
            if (skinJson.contains("transform")) skinData.transform = skinJson["transform"].get<std::vector<std::string>>();
            if (skinJson.contains("path")) skinData.path = skinJson["path"].get<std::vector<std::string>>();
            if (skinJson.contains("physics")) skinData.physics = skinJson["physics"].get<std::vector<std::string>>();
            if (skinJson.contains("attachments")) {
                for (const auto& [slotName, slotAttachments] : skinJson["attachments"].items()) {
                    skinData.attachments[slotName] = {};
                    for (const auto& [attachmentName, attachmentJson] : slotAttachments.items()) {
                        Attachment attachment;
                        attachment.name = attachmentJson.value("name", attachmentName);
                        attachment.path = attachmentJson.value("path", attachment.name);
                        attachment.type = attachmentTypeMap.at(attachmentJson.value("type", "region"));
                        switch (attachment.type) {
                            case AttachmentType_Region: {
                                RegionAttachment region;
                                region.x = attachmentJson.value("x", 0.0f);
                                region.y = attachmentJson.value("y", 0.0f);
                                region.rotation = attachmentJson.value("rotation", 0.0f);
                                region.scaleX = attachmentJson.value("scaleX", 1.0f);
                                region.scaleY = attachmentJson.value("scaleY", 1.0f);
                                region.width = attachmentJson.value("width", 32.0f);
                                region.height = attachmentJson.value("height", 32.0f);
                                if (attachmentJson.contains("color")) region.color = stringToColor(attachmentJson["color"], true);
                                if (attachmentJson.contains("sequence")) region.sequence = readSequence(attachmentJson["sequence"]);
                                attachment.data = region;
                                break; 
                            }
                            case AttachmentType_Mesh: {
                                MeshAttachment mesh;
                                mesh.width = attachmentJson.value("width", 32.0f);
                                mesh.height = attachmentJson.value("height", 32.0f);
                                if (attachmentJson.contains("color")) mesh.color = stringToColor(attachmentJson["color"], true);
                                if (attachmentJson.contains("sequence")) mesh.sequence = readSequence(attachmentJson["sequence"]);
                                mesh.hullLength = attachmentJson.value("hull", 0);
                                mesh.triangles = attachmentJson.value("triangles", std::vector<unsigned short>{});
                                mesh.edges = attachmentJson.value("edges", std::vector<unsigned short>{});
                                mesh.uvs = attachmentJson.value("uvs", std::vector<float>{});
                                mesh.vertices = attachmentJson.value("vertices", std::vector<float>{});
                                attachment.data = mesh;
                                break; 
                            }
                            case AttachmentType_Linkedmesh: {
                                LinkedmeshAttachment linkedMesh;
                                linkedMesh.width = attachmentJson.value("width", 32.0f);
                                linkedMesh.height = attachmentJson.value("height", 32.0f);
                                if (attachmentJson.contains("color")) linkedMesh.color = stringToColor(attachmentJson["color"], true);
                                if (attachmentJson.contains("sequence")) linkedMesh.sequence = readSequence(attachmentJson["sequence"]);
                                linkedMesh.parentMesh = attachmentJson["parent"];
                                linkedMesh.timelines = attachmentJson.value("timelines", 1);
                                if (attachmentJson.contains("skin")) linkedMesh.skin = attachmentJson["skin"];
                                attachment.data = linkedMesh;
                                break; 
                            }
                            case AttachmentType_Boundingbox: {
                                BoundingboxAttachment boundingBox;
                                boundingBox.vertexCount = attachmentJson.value("vertexCount", 0);
                                if (attachmentJson.contains("color")) boundingBox.color = stringToColor(attachmentJson["color"], true);
                                boundingBox.vertices = attachmentJson.value("vertices", std::vector<float>{});
                                attachment.data = boundingBox;
                                break; 
                            }
                            case AttachmentType_Path: {
                                PathAttachment path;
                                path.vertexCount = attachmentJson.value("vertexCount", 0);
                                path.closed = attachmentJson.value("closed", false);
                                path.constantSpeed = attachmentJson.value("constantSpeed", true);
                                if (attachmentJson.contains("color")) path.color = stringToColor(attachmentJson["color"], true);
                                path.vertices = attachmentJson.value("vertices", std::vector<float>{});
                                path.lengths = attachmentJson.value("lengths", std::vector<float>{});
                                attachment.data = path;
                                break; 
                            }
                            case AttachmentType_Point: {
                                PointAttachment point;
                                point.x = attachmentJson.value("x", 0.0f);
                                point.y = attachmentJson.value("y", 0.0f);
                                point.rotation = attachmentJson.value("rotation", 0.0f);
                                if (attachmentJson.contains("color")) point.color = stringToColor(attachmentJson["color"], true);
                                attachment.data = point;
                                break; 
                            }
                            case AttachmentType_Clipping: {
                                ClippingAttachment clipping;
                                clipping.vertexCount = attachmentJson.value("vertexCount", 0);
                                if (attachmentJson.contains("color")) clipping.color = stringToColor(attachmentJson["color"], true);
                                if (attachmentJson.contains("end")) clipping.endSlot = attachmentJson["end"];
                                clipping.vertices = attachmentJson.value("vertices", std::vector<float>{});
                                attachment.data = clipping;
                                break; 
                            }
                        }
                        skinData.attachments[slotName][attachmentName] = attachment;
                    }
                }
            }
            skeletonData.skins.push_back(skinData);
        }
    }

    /* Events */
    if (j.contains("events")) {
        for (const auto& [eventName, eventJson] : j["events"].items()) {
            EventData eventData;
            eventData.name = eventName;
            eventData.intValue = eventJson.value("int", 0);
            eventData.floatValue = eventJson.value("float", 0.0f);
            if (eventJson.contains("string")) eventData.stringValue = eventJson["string"];
            if (eventJson.contains("audio")) {
                eventData.audioPath = eventJson["audio"];
                eventData.volume = eventJson.value("volume", 1.0f);
                eventData.balance = eventJson.value("balance", 0.0f);
            }
            skeletonData.events.push_back(eventData);
        }
    }

    /* Animations */
    if (j.contains("animations")) {
        for (const auto& [animationName, animationJson] : j["animations"].items()) {
            Animation animationData;
            animationData.name = animationName;
            if (animationJson.contains("slots")) {
                for (const auto& [slotName, slotJson] : animationJson["slots"].items()) {
                    MultiTimeline slotTimeline;
                    if (slotJson.contains("attachment")) {
                        for (const auto& frameJson : slotJson["attachment"]) {
                            TimelineFrame frame;
                            frame.time = frameJson.value("time", 0.0f);
                            if (frameJson.contains("name") && !frameJson["name"].is_null()) frame.str1 = frameJson["name"];
                            slotTimeline["attachment"].push_back(frame);
                        }
                    }
                    if (slotJson.contains("rgba")) {
                        for (const auto& frameJson : slotJson["rgba"]) {
                            TimelineFrame frame;
                            frame.time = frameJson.value("time", 0.0f);
                            if (frameJson.contains("color")) frame.color1 = stringToColor(frameJson["color"], true);
                            readCurve(frameJson, frame);
                            slotTimeline["rgba"].push_back(frame);
                        }
                    }
                    if (slotJson.contains("rgb")) {
                        for (const auto& frameJson : slotJson["rgb"]) {
                            TimelineFrame frame;
                            frame.time = frameJson.value("time", 0.0f);
                            if (frameJson.contains("color")) frame.color1 = stringToColor(frameJson["color"], false);
                            readCurve(frameJson, frame);
                            slotTimeline["rgb"].push_back(frame);
                        }
                    }
                    if (slotJson.contains("alpha")) {
                        readTimeline(slotJson["alpha"], slotTimeline["alpha"], 1, "value", "", 0.0f);
                    }
                    if (slotJson.contains("rgba2")) {
                        for (const auto& frameJson : slotJson["rgba2"]) {
                            TimelineFrame frame;
                            frame.time = frameJson.value("time", 0.0f);
                            if (frameJson.contains("light")) frame.color1 = stringToColor(frameJson["light"], true);
                            if (frameJson.contains("dark")) frame.color2 = stringToColor(frameJson["dark"], false);
                            readCurve(frameJson, frame);
                            slotTimeline["rgba2"].push_back(frame);
                        }
                    }
                    if (slotJson.contains("rgb2")) {
                        for (const auto& frameJson : slotJson["rgb2"]) {
                            TimelineFrame frame;
                            frame.time = frameJson.value("time", 0.0f);
                            if (frameJson.contains("light")) frame.color1 = stringToColor(frameJson["light"], false);
                            if (frameJson.contains("dark")) frame.color2 = stringToColor(frameJson["dark"], false);
                            readCurve(frameJson, frame);
                            slotTimeline["rgb2"].push_back(frame);
                        }
                    }
                    animationData.slots[slotName] = slotTimeline;
                }
            }
            if (animationJson.contains("bones")) {
                for (const auto& [boneName, boneJson] : animationJson["bones"].items()) {
                    MultiTimeline boneTimeline;
                    if (boneJson.contains("rotate")) {
                        readTimeline(boneJson["rotate"], boneTimeline["rotate"], 1, "value", "", 0.0f);
                    }
                    if (boneJson.contains("translate")) {
                        readTimeline(boneJson["translate"], boneTimeline["translate"], 2, "x", "y", 0.0f);
                    }
                    if (boneJson.contains("translatex")) {
                        readTimeline(boneJson["translatex"], boneTimeline["translatex"], 1, "value", "", 0.0f);
                    }
                    if (boneJson.contains("translatey")) {
                        readTimeline(boneJson["translatey"], boneTimeline["translatey"], 1, "value", "", 0.0f);
                    }
                    if (boneJson.contains("scale")) {
                        readTimeline(boneJson["scale"], boneTimeline["scale"], 2, "x", "y", 1.0f);
                    }
                    if (boneJson.contains("scalex")) {
                        readTimeline(boneJson["scalex"], boneTimeline["scalex"], 1, "value", "", 1.0f);
                    }
                    if (boneJson.contains("scaley")) {
                        readTimeline(boneJson["scaley"], boneTimeline["scaley"], 1, "value", "", 1.0f);
                    }
                    if (boneJson.contains("shear")) {
                        readTimeline(boneJson["shear"], boneTimeline["shear"], 2, "x", "y", 0.0f);
                    }
                    if (boneJson.contains("shearx")) {
                        readTimeline(boneJson["shearx"], boneTimeline["shearx"], 1, "value", "", 0.0f);
                    }
                    if (boneJson.contains("sheary")) {
                        readTimeline(boneJson["sheary"], boneTimeline["sheary"], 1, "value", "", 0.0f);
                    }
                    if (boneJson.contains("inherit")) {
                        for (const auto& frameJson : boneJson["inherit"]) {
                            TimelineFrame frame;
                            frame.time = frameJson.value("time", 0.0f);
                            frame.inherit = inheritMap.at(frameJson.value("inherit", "normal"));
                            boneTimeline["inherit"].push_back(frame);
                        }
                    }
                    animationData.bones[boneName] = boneTimeline;
                }
            }
            if (animationJson.contains("ik")) {
                for (const auto& [ikName, ikJson] : animationJson["ik"].items()) {
                    Timeline ikTimeline; 
                    for (const auto& frameJson : ikJson) {
                        TimelineFrame frame;
                        frame.time = frameJson.value("time", 0.0f);
                        frame.value1 = frameJson.value("mix", 1.0f);
                        frame.value2 = frameJson.value("softness", 0.0f);
                        frame.bendPositive = frameJson.value("bendPositive", true); 
                        frame.compress = frameJson.value("compress", false);
                        frame.stretch = frameJson.value("stretch", false);
                        readCurve(frameJson, frame);
                        ikTimeline.push_back(frame);
                    }
                    animationData.ik[ikName] = ikTimeline;
                }
            }
            if (animationJson.contains("transform")) {
                for (const auto& [transformName, transformJson] : animationJson["transform"].items()) {
                    Timeline transformTimeline; 
                    for (const auto& frameJson : transformJson) {
                        TimelineFrame frame;
                        frame.time = frameJson.value("time", 0.0f);
                        frame.value1 = frameJson.value("mixRotate", 1.0f);
                        frame.value2 = frameJson.value("mixX", 1.0f); 
                        frame.value3 = frameJson.value("mixY", frame.value2);
                        frame.value4 = frameJson.value("mixScaleX", 1.0f);
                        frame.value5 = frameJson.value("mixScaleY", frame.value4);
                        frame.value6 = frameJson.value("mixShearY", 1.0f);
                        readCurve(frameJson, frame);
                        transformTimeline.push_back(frame);
                    }
                    animationData.transform[transformName] = transformTimeline;
                }
            }
            if (animationJson.contains("path")) {
                for (const auto& [pathName, pathJson] : animationJson["path"].items()) {
                    MultiTimeline pathTimeline;
                    if (pathJson.contains("position")) {
                        readTimeline(pathJson["position"], pathTimeline["position"], 1, "value", "", 0.0f);
                    }
                    if (pathJson.contains("spacing")) {
                        readTimeline(pathJson["spacing"], pathTimeline["spacing"], 1, "value", "", 0.0f);
                    }
                    if (pathJson.contains("mix")) {
                        for (const auto& frameJson : pathJson["mix"]) {
                            TimelineFrame frame;
                            frame.time = frameJson.value("time", 0.0f);
                            frame.value1 = frameJson.value("mixRotate", 1.0f);
                            frame.value2 = frameJson.value("mixX", 1.0f);
                            frame.value3 = frameJson.value("mixY", frame.value2);
                            readCurve(frameJson, frame);
                            pathTimeline["mix"].push_back(frame);
                        }
                    }
                    animationData.path[pathName] = pathTimeline;
                }
            }
            if (animationJson.contains("physics")) {
                for (const auto& [physicsName, physicsJson] : animationJson["physics"].items()) {
                    MultiTimeline physicsTimeline;
                    if (physicsJson.contains("reset")) {
                        for (const auto& frameJson : physicsJson["reset"]) {
                            TimelineFrame frame;
                            frame.time = frameJson.value("time", 0.0f);
                            physicsTimeline["reset"].push_back(frame);
                        }
                    }
                    if (physicsJson.contains("inertia")) {
                        readTimeline(physicsJson["inertia"], physicsTimeline["inertia"], 1, "value", "", 0.0f);
                    }
                    if (physicsJson.contains("strength")) {
                        readTimeline(physicsJson["strength"], physicsTimeline["strength"], 1, "value", "", 0.0f);
                    }
                    if (physicsJson.contains("damping")) {
                        readTimeline(physicsJson["damping"], physicsTimeline["damping"], 1, "value", "", 0.0f);
                    }
                    if (physicsJson.contains("mass")) {
                        readTimeline(physicsJson["mass"], physicsTimeline["mass"], 1, "value", "", 0.0f);
                    }
                    if (physicsJson.contains("wind")) {
                        readTimeline(physicsJson["wind"], physicsTimeline["wind"], 1, "value", "", 0.0f);
                    }
                    if (physicsJson.contains("gravity")) {
                        readTimeline(physicsJson["gravity"], physicsTimeline["gravity"], 1, "value", "", 0.0f);
                    }
                    if (physicsJson.contains("mix")) {
                        readTimeline(physicsJson["mix"], physicsTimeline["mix"], 1, "value", "", 1.0f);
                    }
                    animationData.physics[physicsName] = physicsTimeline;
                }
            }
            if (animationJson.contains("attachments")) {
                for (const auto& [skinName, skinJson] : animationJson["attachments"].items()) {
                    for (const auto& [slotName, slotJson] : skinJson.items()) {
                        for (const auto& [attachmentName, attachmentJson] : slotJson.items()) {
                            MultiTimeline attachmentTimeline;
                            if (attachmentJson.contains("deform")) {
                                for (const auto& frameJson : attachmentJson["deform"]) {
                                    TimelineFrame frame;
                                    frame.time = frameJson.value("time", 0.0f);
                                    if (frameJson.contains("vertices")) {
                                        frame.int1 = frameJson.value("offset", 0);
                                        frame.vertices = frameJson["vertices"].get<std::vector<float>>();
                                    }
                                    readCurve(frameJson, frame);
                                    attachmentTimeline["deform"].push_back(frame);
                                }
                            }
                            if (attachmentJson.contains("sequence")) {
                                float lastDelay = 0.0f; 
                                for (const auto& frameJson : attachmentJson["sequence"]) {
                                    TimelineFrame frame;
                                    frame.time = frameJson.value("time", 0.0f);
                                    frame.value1 = frameJson.value("delay", lastDelay);
                                    lastDelay = frame.value1;
                                    frame.int1 = frameJson.value("index", 0);
                                    frame.sequenceMode = sequenceModeMap.at(frameJson.value("mode", "hold"));
                                    attachmentTimeline["sequence"].push_back(frame);
                                }
                            }
                            animationData.attachments[skinName][slotName][attachmentName] = attachmentTimeline;
                        }
                    }
                }
            }
            if (animationJson.contains("drawOrder")) {
                for (const auto& frameJson : animationJson["drawOrder"]) {
                    TimelineFrame frame;
                    frame.time = frameJson.value("time", 0.0f);
                    if (frameJson.contains("offsets")) {
                        for (const auto& offsetJson : frameJson["offsets"]) {
                            frame.offsets.push_back({offsetJson["slot"], offsetJson.value("offset", 0)});
                        }
                    }
                    animationData.drawOrder.push_back(frame);
                }
            }
            if (animationJson.contains("events")) {
                for (const auto& frameJson : animationJson["events"]) {
                    TimelineFrame frame;
                    frame.time = frameJson.value("time", 0.0f);
                    if (frameJson.contains("name")) frame.str1 = frameJson["name"]; 
                    int eventIndex = -1; 
                    for (size_t i = 0; i < skeletonData.events.size(); i++) {
                        if (skeletonData.events[i].name == frame.str1.value()) {
                            eventIndex = static_cast<int>(i);
                            break;
                        }
                    }
                    EventData eventData = skeletonData.events[eventIndex]; 
                    frame.int1 = frameJson.value("int", eventData.intValue);
                    frame.value1 = frameJson.value("float", eventData.floatValue);
                    if (frameJson.contains("string")) frame.str2 = frameJson["string"]; 
                    else frame.str2 = eventData.stringValue;
                    if (eventData.audioPath) {
                        frame.value2 = frameJson.value("volume", 1.0f);
                        frame.value3 = frameJson.value("balance", 0.0f);
                    }
                    animationData.events.push_back(frame);
                }
            }
            skeletonData.animations.push_back(animationData);
        }
    }

    return skeletonData;
}

}
