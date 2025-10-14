#include "SkeletonData.h"

namespace spine36 {

void readCurve(const Json& j, TimelineFrame& frame) {
    if (j.contains("curve")) {
        if (j["curve"] == "stepped") {
            frame.curveType = CurveType::CURVE_STEPPED;
        } else {
            frame.curveType = CURVE_BEZIER; 
            frame.curve = j.value("curve", std::vector<float>{});
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
    if (skeleton.contains("hash")) {
        skeletonData.hashString = skeleton["hash"];
        skeletonData.hash = base64ToUint64(skeleton["hash"]);
    }
    if (skeleton.contains("spine")) skeletonData.version = skeleton["spine"];
    skeletonData.width = skeleton.value("width", 0.0f);
    skeletonData.height = skeleton.value("height", 0.0f);
    if (skeleton.contains("images")) skeletonData.imagesPath = skeleton["images"];
    skeletonData.nonessential = true; 

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
            boneData.inherit = inheritMap.at(boneJson.value("transform", "normal"));
            if (boneJson.contains("color")) boneData.color = stringToColor(boneJson["color"], true);
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
            skeletonData.slots.push_back(slotData);
        }
    }

    /* IK constraints */
    if (j.contains("ik")) {
        for (const auto& ikJson : j["ik"]) {
            IKConstraintData ikData;
            if (ikJson.contains("name")) ikData.name = ikJson["name"];
            ikData.order = ikJson.value("order", 0);
            if (ikJson.contains("bones")) ikData.bones = ikJson["bones"].get<std::vector<std::string>>();
            if (ikJson.contains("target")) ikData.target = ikJson["target"];
            ikData.mix = ikJson.value("mix", 1.0f);
            ikData.bendPositive = ikJson.value("bendPositive", true);
            skeletonData.ikConstraints.push_back(ikData);
        }
    }

    /* Transform constraints */
    if (j.contains("transform")) {
        for (const auto& transformJson : j["transform"]) {
            TransformConstraintData transformData;
            if (transformJson.contains("name")) transformData.name = transformJson["name"];
            transformData.order = transformJson.value("order", 0);
            if (transformJson.contains("bones")) transformData.bones = transformJson["bones"].get<std::vector<std::string>>();
            if (transformJson.contains("target")) transformData.target = transformJson["target"];
            transformData.mixRotate = transformJson.value("rotateMix", 1.0f);
            transformData.mixX = transformJson.value("translateMix", 1.0f);
            transformData.mixY = transformJson.value("translateMix", 1.0f);
            transformData.mixScaleX = transformJson.value("scaleMix", 1.0f);
            transformData.mixScaleY = transformJson.value("scaleMix", 1.0f);
            transformData.mixShearY = transformJson.value("shearMix", 1.0f);
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
            if (pathJson.contains("bones")) pathData.bones = pathJson["bones"].get<std::vector<std::string>>();
            if (pathJson.contains("target")) pathData.target = pathJson["target"];
            pathData.positionMode = positionModeMap.at(pathJson.value("positionMode", "percent"));
            pathData.spacingMode = spacingModeMap.at(pathJson.value("spacingMode", "length"));
            pathData.rotateMode = rotateModeMap.at(pathJson.value("rotateMode", "tangent"));
            pathData.offsetRotation = pathJson.value("rotation", 0.0f);
            pathData.position = pathJson.value("position", 0.0f);
            pathData.spacing = pathJson.value("spacing", 0.0f);
            pathData.mixRotate = pathJson.value("rotateMix", 1.0f);
            pathData.mixX = pathJson.value("translateMix", 1.0f);
            pathData.mixY = pathJson.value("translateMix", 1.0f);
            skeletonData.pathConstraints.push_back(pathData);
        }
    }

    /* Skins */
    if (j.contains("skins")) {
        for (const auto& [skinName, skinJson] : j["skins"].items()) {
            Skin skinData;
            skinData.name = skinName; 
            for (const auto& [slotName, slotAttachments] : skinJson.items()) {
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
                            attachment.data = region;
                            break; 
                        }
                        case AttachmentType_Mesh: {
                            MeshAttachment mesh;
                            mesh.width = attachmentJson.value("width", 32.0f);
                            mesh.height = attachmentJson.value("height", 32.0f);
                            if (attachmentJson.contains("color")) mesh.color = stringToColor(attachmentJson["color"], true);
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
                            linkedMesh.parentMesh = attachmentJson["parent"];
                            linkedMesh.timelines = attachmentJson.value("deform", 1);
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
                    if (slotJson.contains("color")) {
                        for (const auto& frameJson : slotJson["color"]) {
                            TimelineFrame frame;
                            frame.time = frameJson.value("time", 0.0f);
                            if (frameJson.contains("color")) frame.color1 = stringToColor(frameJson["color"], true);
                            readCurve(frameJson, frame);
                            slotTimeline["rgba"].push_back(frame);
                        }
                    }
                    if (slotJson.contains("twoColor")) {
                        for (const auto& frameJson : slotJson["twoColor"]) {
                            TimelineFrame frame;
                            frame.time = frameJson.value("time", 0.0f);
                            if (frameJson.contains("light")) frame.color1 = stringToColor(frameJson["light"], true);
                            if (frameJson.contains("dark")) frame.color2 = stringToColor(frameJson["dark"], false);
                            readCurve(frameJson, frame);
                            slotTimeline["rgba2"].push_back(frame);
                        }
                    }
                    animationData.slots[slotName] = slotTimeline;
                }
            }
            if (animationJson.contains("bones")) {
                for (const auto& [boneName, boneJson] : animationJson["bones"].items()) {
                    MultiTimeline boneTimeline;
                    if (boneJson.contains("rotate")) {
                        readTimeline(boneJson["rotate"], boneTimeline["rotate"], 1, "angle", "", 0.0f);
                    }
                    if (boneJson.contains("translate")) {
                        readTimeline(boneJson["translate"], boneTimeline["translate"], 2, "x", "y", 0.0f);
                    }
                    if (boneJson.contains("scale")) {
                        readTimeline(boneJson["scale"], boneTimeline["scale"], 2, "x", "y", 0.0f);
                    }
                    if (boneJson.contains("shear")) {
                        readTimeline(boneJson["shear"], boneTimeline["shear"], 2, "x", "y", 0.0f);
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
                        frame.bendPositive = frameJson.value("bendPositive", true); 
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
                        frame.value1 = frameJson.value("rotateMix", 1.0f);
                        frame.value2 = frameJson.value("translateMix", 1.0f); 
                        frame.value3 = frame.value2; 
                        frame.value4 = frameJson.value("scaleMix", 1.0f);
                        frame.value5 = frame.value4; 
                        frame.value6 = frameJson.value("shearMix", 1.0f);
                        readCurve(frameJson, frame);
                        transformTimeline.push_back(frame);
                    }
                    animationData.transform[transformName] = transformTimeline;
                }
            }
            if (animationJson.contains("paths")) {
                for (const auto& [pathName, pathJson] : animationJson["paths"].items()) {
                    MultiTimeline pathTimeline;
                    if (pathJson.contains("position")) {
                        readTimeline(pathJson["position"], pathTimeline["position"], 1, "position", "", 0.0f);
                    }
                    if (pathJson.contains("spacing")) {
                        readTimeline(pathJson["spacing"], pathTimeline["spacing"], 1, "spacing", "", 0.0f);
                    }
                    if (pathJson.contains("mix")) {
                        for (const auto& frameJson : pathJson["mix"]) {
                            TimelineFrame frame;
                            frame.time = frameJson.value("time", 0.0f);
                            frame.value1 = frameJson.value("rotateMix", 1.0f);
                            frame.value2 = frameJson.value("translateMix", 1.0f);
                            frame.value3 = frameJson.value("translateMix", 1.0f);
                            readCurve(frameJson, frame);
                            pathTimeline["mix"].push_back(frame);
                        }
                    }
                    animationData.path[pathName] = pathTimeline;
                }
            }
            if (animationJson.contains("deform")) {
                for (const auto& [skinName, skinJson] : animationJson["deform"].items()) {
                    for (const auto& [slotName, slotJson] : skinJson.items()) {
                        for (const auto& [attachmentName, attachmentJson] : slotJson.items()) {
                            Timeline attachmentTimeline;
                            for (const auto& frameJson : attachmentJson) {
                                TimelineFrame frame;
                                frame.time = frameJson.value("time", 0.0f);
                                if (frameJson.contains("vertices")) {
                                    frame.int1 = frameJson.value("offset", 0);
                                    frame.vertices = frameJson["vertices"].get<std::vector<float>>();
                                }
                                readCurve(frameJson, frame);
                                attachmentTimeline.push_back(frame);
                            }
                            animationData.attachments[skinName][slotName][attachmentName]["deform"] = attachmentTimeline;
                        }
                    }
                }
            }
            if (animationJson.contains("drawOrder") || animationJson.contains("draworder")) {
                for (const auto& frameJson : animationJson.contains("drawOrder") ? animationJson["drawOrder"] : animationJson["draworder"]) {
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
                    animationData.events.push_back(frame);
                }
            }
            skeletonData.animations.push_back(animationData);
        }
    }

    return skeletonData;
}

}
