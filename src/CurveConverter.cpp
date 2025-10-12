#include "SkeletonData.h"

enum class CurveYType {
    R1, G1, B1, A1, R2, G2, B2, V1, V2, V3, V4, V5, V6, ZeroOne
}; 

const static std::vector<size_t> CurveYTypeBezierIndex = {
    0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 3, 4, 5, 0
};

struct BezierCurve {
    float cx1, cy1, cx2, cy2, x1, y1, x2, y2; 
}; 

void convertBezierCurve3xTo4x(BezierCurve& bezier) {
    float timeRange = bezier.x2 - bezier.x1;
    float valueRange = bezier.y2 - bezier.y1;
    bezier.cx1 = bezier.x1 + bezier.cx1 * timeRange; 
    bezier.cy1 = bezier.y1 + bezier.cy1 * valueRange; 
    bezier.cx2 = bezier.x1 + bezier.cx2 * timeRange; 
    bezier.cy2 = bezier.y1 + bezier.cy2 * valueRange;
}

void convertBezierCurve4xTo3x(BezierCurve& bezier) {
    float timeRange = bezier.x2 - bezier.x1;
    float valueRange = bezier.y2 - bezier.y1;
    bezier.cx1 = timeRange ? (bezier.cx1 - bezier.x1) / timeRange : 0.0f;
    bezier.cy1 = valueRange ? (bezier.cy1 - bezier.y1) / valueRange : 0.0f;
    bezier.cx2 = timeRange ? (bezier.cx2 - bezier.x1) / timeRange : 1.0f;
    bezier.cy2 = valueRange ? (bezier.cy2 - bezier.y1) / valueRange : 1.0f;
}

void convertTimelineCurve3xTo4x(Timeline& timeline, std::vector<CurveYType> curveYTypes) {
    for (size_t i = 0; i + 1 < timeline.size(); i++) {
        auto& frame = timeline[i];
        if (frame.curveType == CurveType::CURVE_BEZIER) {
            float cx1 = frame.curve[0];
            float cy1 = frame.curve[1];
            float cx2 = frame.curve[2];
            float cy2 = frame.curve[3];
            for (CurveYType curveYType : curveYTypes) {
                size_t bezierIndex = CurveYTypeBezierIndex[(size_t)curveYType];
                if (frame.curve.size() < (bezierIndex + 1) * 4) {
                    frame.curve.resize((bezierIndex + 1) * 4);
                }
                BezierCurve bezier;
                bezier.cx1 = cx1;
                bezier.cy1 = cy1;
                bezier.cx2 = cx2;
                bezier.cy2 = cy2;
                bezier.x1 = frame.time;
                switch (curveYType) {
                    case CurveYType::R1: bezier.y1 = frame.color1.value().r / 255.0f; break;
                    case CurveYType::G1: bezier.y1 = frame.color1.value().g / 255.0f; break;
                    case CurveYType::B1: bezier.y1 = frame.color1.value().b / 255.0f; break;
                    case CurveYType::A1: bezier.y1 = frame.color1.value().a / 255.0f; break;
                    case CurveYType::R2: bezier.y1 = frame.color2.value().r / 255.0f; break;
                    case CurveYType::G2: bezier.y1 = frame.color2.value().g / 255.0f; break;
                    case CurveYType::B2: bezier.y1 = frame.color2.value().b / 255.0f; break;
                    case CurveYType::V1: bezier.y1 = frame.value1; break;
                    case CurveYType::V2: bezier.y1 = frame.value2; break;
                    case CurveYType::V3: bezier.y1 = frame.value3; break;
                    case CurveYType::V4: bezier.y1 = frame.value4; break;
                    case CurveYType::V5: bezier.y1 = frame.value5; break;
                    case CurveYType::V6: bezier.y1 = frame.value6; break;
                    case CurveYType::ZeroOne: bezier.y1 = 0.0f; break;
                }
                bezier.x2 = timeline[i + 1].time;
                switch (curveYType) {
                    case CurveYType::R1: bezier.y2 = timeline[i + 1].color1.value().r / 255.0f; break;
                    case CurveYType::G1: bezier.y2 = timeline[i + 1].color1.value().g / 255.0f; break;
                    case CurveYType::B1: bezier.y2 = timeline[i + 1].color1.value().b / 255.0f; break;
                    case CurveYType::A1: bezier.y2 = timeline[i + 1].color1.value().a / 255.0f; break;
                    case CurveYType::R2: bezier.y2 = timeline[i + 1].color2.value().r / 255.0f; break;
                    case CurveYType::G2: bezier.y2 = timeline[i + 1].color2.value().g / 255.0f; break;
                    case CurveYType::B2: bezier.y2 = timeline[i + 1].color2.value().b / 255.0f; break;
                    case CurveYType::V1: bezier.y2 = timeline[i + 1].value1; break;
                    case CurveYType::V2: bezier.y2 = timeline[i + 1].value2; break;
                    case CurveYType::V3: bezier.y2 = timeline[i + 1].value3; break;
                    case CurveYType::V4: bezier.y2 = timeline[i + 1].value4; break;
                    case CurveYType::V5: bezier.y2 = timeline[i + 1].value5; break;
                    case CurveYType::V6: bezier.y2 = timeline[i + 1].value6; break;
                    case CurveYType::ZeroOne: bezier.y2 = 1.0f; break;
                }
                convertBezierCurve3xTo4x(bezier);
                frame.curve[bezierIndex * 4 + 0] = bezier.cx1;
                frame.curve[bezierIndex * 4 + 1] = bezier.cy1;
                frame.curve[bezierIndex * 4 + 2] = bezier.cx2;
                frame.curve[bezierIndex * 4 + 3] = bezier.cy2;
            }
        }
    }
}

void convertTimelineCurve4xTo3x(Timeline& timeline, CurveYType curveYType) {
    for (size_t i = 0; i + 1 < timeline.size(); i++) {
        auto& frame = timeline[i];
        if (frame.curveType == CurveType::CURVE_BEZIER) {
            size_t bezierIndex = CurveYTypeBezierIndex[(size_t)curveYType];
            BezierCurve bezier;
            bezier.cx1 = frame.curve[bezierIndex * 4 + 0];
            bezier.cy1 = frame.curve[bezierIndex * 4 + 1];
            bezier.cx2 = frame.curve[bezierIndex * 4 + 2];
            bezier.cy2 = frame.curve[bezierIndex * 4 + 3];
            bezier.x1 = frame.time;
            switch (curveYType) {
                case CurveYType::R1: bezier.y1 = frame.color1.value().r / 255.0f; break;
                case CurveYType::V1: bezier.y1 = frame.value1; break;
                case CurveYType::ZeroOne: bezier.y1 = 0.0f; break;
                default: bezier.y1 = 0.0f; break; // Other types are not handled in 4x to 3x conversion
            }
            bezier.x2 = timeline[i + 1].time;
            switch (curveYType) {
                case CurveYType::R1: bezier.y2 = timeline[i + 1].color1.value().r / 255.0f; break;
                case CurveYType::V1: bezier.y2 = timeline[i + 1].value1; break;
                case CurveYType::ZeroOne: bezier.y2 = 1.0f; break;
                default: bezier.y2 = 1.0f; break; // Other types are not handled in 4x to 3x conversion
            }
            convertBezierCurve4xTo3x(bezier);
            frame.curve[0] = bezier.cx1;
            frame.curve[1] = bezier.cy1;
            frame.curve[2] = bezier.cx2;
            frame.curve[3] = bezier.cy2;
        }
    }
}

void convertCurve3xTo4x(SkeletonData& skeleton) {
    for (auto& animation : skeleton.animations) {
        for (auto& [slotName, multiTimeline] : animation.slots)
            for (auto& [timelineType, timeline] : multiTimeline)
                if (timelineType == "rgba")
                    convertTimelineCurve3xTo4x(timeline, { CurveYType::R1, CurveYType::G1, CurveYType::B1, CurveYType::A1 });
                else if (timelineType == "rgba2")
                    convertTimelineCurve3xTo4x(timeline, { CurveYType::R1, CurveYType::G1, CurveYType::B1, CurveYType::A1, CurveYType::R2, CurveYType::G2, CurveYType::B2 });
        for (auto& [boneName, multiTimeline] : animation.bones)
            for (auto& [timelineType, timeline] : multiTimeline)
                if (timelineType == "rotate")
                    convertTimelineCurve3xTo4x(timeline, { CurveYType::V1 });
                else if (timelineType == "translate" || timelineType == "scale" || timelineType == "shear")
                    convertTimelineCurve3xTo4x(timeline, { CurveYType::V1, CurveYType::V2 });
        for (auto& [ikName, timeline] : animation.ik)
            convertTimelineCurve3xTo4x(timeline, { CurveYType::V1, CurveYType::V2 });
        for (auto& [transformName, timeline] : animation.transform)
            convertTimelineCurve3xTo4x(timeline, { CurveYType::V1, CurveYType::V2, CurveYType::V3, CurveYType::V4, CurveYType::V5, CurveYType::V6 });
        for (auto& [pathName, multiTimeline] : animation.path)
            for (auto& [timelineType, timeline] : multiTimeline)
                if (timelineType == "position" || timelineType == "spacing")
                    convertTimelineCurve3xTo4x(timeline, { CurveYType::V1 });
                else if (timelineType == "mix")
                    convertTimelineCurve3xTo4x(timeline, { CurveYType::V1, CurveYType::V2, CurveYType::V3 });
        for (auto& [skinName, skin] : animation.attachments)
            for (auto& [slotName, slot] : skin)
                for (auto& [attachmentName, multiTimeline] : slot)
                    for (auto& [timelineType, timeline] : multiTimeline)
                        if (timelineType == "deform")
                            convertTimelineCurve3xTo4x(timeline, { CurveYType::ZeroOne });
    }
}

void convertCurve4xTo3x(SkeletonData& skeleton) {
    for (auto& animation : skeleton.animations) {
        for (auto& [slotName, multiTimeline] : animation.slots)
            for (auto& [timelineType, timeline] : multiTimeline)
                if (timelineType == "rgba" || timelineType == "rgba2" || timelineType == "rgb" || timelineType == "rgb2")
                    convertTimelineCurve4xTo3x(timeline, CurveYType::R1);
        for (auto& [boneName, multiTimeline] : animation.bones)
            for (auto& [timelineType, timeline] : multiTimeline)
                convertTimelineCurve4xTo3x(timeline, CurveYType::V1);
        for (auto& [ikName, timeline] : animation.ik)
            convertTimelineCurve4xTo3x(timeline, CurveYType::V1);
        for (auto& [transformName, timeline] : animation.transform)
            convertTimelineCurve4xTo3x(timeline, CurveYType::V1);
        for (auto& [pathName, multiTimeline] : animation.path)
            for (auto& [timelineType, timeline] : multiTimeline)
                convertTimelineCurve4xTo3x(timeline, CurveYType::V1);
        for (auto& [skinName, skin] : animation.attachments)
            for (auto& [slotName, slot] : skin)
                for (auto& [attachmentName, multiTimeline] : slot)
                    for (auto& [timelineType, timeline] : multiTimeline)
                        if (timelineType == "deform")
                            convertTimelineCurve4xTo3x(timeline, CurveYType::ZeroOne);
    }
}
