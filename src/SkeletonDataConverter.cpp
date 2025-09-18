#include "SkeletonData.h"
#include <cmath>
#include <algorithm>

// 将3.x版本的相对贝塞尔控制点转换为4.x版本的绝对坐标
void convertCurve3xTo4x(std::vector<float>& curve, float time1, float value1, float time2, float value2) {
    if (curve.size() < 4) return;
    
    // 3.x版本使用相对坐标 (0-1 范围)
    // 4.x版本使用绝对时间和值坐标
    
    float cx1_rel = curve[0];  // 3.x相对控制点1
    float cy1_rel = curve[1];
    float cx2_rel = curve[2];  // 3.x相对控制点2  
    float cy2_rel = curve[3];
    
    // 转换为4.x的绝对坐标
    float timeRange = time2 - time1;
    float valueRange = value2 - value1;
    
    curve[0] = time1 + cx1_rel * timeRange;   // 绝对时间1
    curve[1] = value1 + cy1_rel * valueRange; // 绝对值1
    curve[2] = time1 + cx2_rel * timeRange;   // 绝对时间2
    curve[3] = value1 + cy2_rel * valueRange; // 绝对值2
}

// 将4.x版本的绝对贝塞尔控制点转换为3.x版本的相对坐标
void convertCurve4xTo3x(std::vector<float>& curve, float time1, float value1, float time2, float value2) {
    if (curve.size() < 4) return;
    
    // 4.x版本使用绝对时间和值坐标
    // 3.x版本使用相对坐标 (0-1 范围)
    
    float cx1_abs = curve[0];  // 4.x绝对控制点1
    float cy1_abs = curve[1];
    float cx2_abs = curve[2];  // 4.x绝对控制点2
    float cy2_abs = curve[3];
    
    // 转换为3.x的相对坐标
    float timeRange = time2 - time1;
    float valueRange = value2 - value1;
    
    if (timeRange != 0.0f) {
        curve[0] = (cx1_abs - time1) / timeRange;   // 相对时间1
        curve[2] = (cx2_abs - time1) / timeRange;   // 相对时间2
    } else {
        curve[0] = 0.0f;
        curve[2] = 1.0f;
    }
    
    if (valueRange != 0.0f) {
        curve[1] = (cy1_abs - value1) / valueRange; // 相对值1
        curve[3] = (cy2_abs - value1) / valueRange; // 相对值2
    } else {
        curve[1] = 0.0f;
        curve[3] = 1.0f;
    }
    
    // 确保相对坐标在0-1范围内
    curve[0] = std::clamp(curve[0], 0.0f, 1.0f);
    curve[1] = std::clamp(curve[1], 0.0f, 1.0f);
    curve[2] = std::clamp(curve[2], 0.0f, 1.0f);
    curve[3] = std::clamp(curve[3], 0.0f, 1.0f);
}

void convertSkeletonData3xTo4x(SkeletonData& skelData) {
    // 转换动画中的曲线数据
    for (auto& animation : skelData.animations) {
        // 处理骨骼动画
        for (auto& [boneName, boneTimelines] : animation.bones) {
            for (auto& [timelineType, timeline] : boneTimelines) {
                for (size_t i = 0; i < timeline.size(); ++i) {
                    auto& frame = timeline[i];
                    
                    if (frame.curveType == CurveType::CURVE_BEZIER && !frame.curve.empty()) {
                        float time1 = frame.time;
                        float time2 = (i + 1 < timeline.size()) ? timeline[i + 1].time : frame.time + 1.0f;
                        
                        // 根据时间轴类型确定值范围
                        float value1 = 0.0f, value2 = 0.0f;
                        
                        if (timelineType == "rotate") {
                            value1 = frame.value1;
                            value2 = (i + 1 < timeline.size()) ? timeline[i + 1].value1 : frame.value1;
                        } else if (timelineType == "translate" || timelineType == "scale" || timelineType == "shear") {
                            value1 = frame.value1; // 可能需要处理X和Y分量
                            value2 = (i + 1 < timeline.size()) ? timeline[i + 1].value1 : frame.value1;
                        }
                        
                        convertCurve3xTo4x(frame.curve, time1, value1, time2, value2);
                    }
                }
            }
        }
        
        // 处理插槽动画
        for (auto& [slotName, slotTimelines] : animation.slots) {
            for (auto& [timelineType, timeline] : slotTimelines) {
                for (size_t i = 0; i < timeline.size(); ++i) {
                    auto& frame = timeline[i];
                    
                    if (frame.curveType == CurveType::CURVE_BEZIER && !frame.curve.empty()) {
                        float time1 = frame.time;
                        float time2 = (i + 1 < timeline.size()) ? timeline[i + 1].time : frame.time + 1.0f;
                        
                        float value1 = 0.0f, value2 = 0.0f;
                        if (timelineType == "alpha") {
                            value1 = frame.value1;
                            value2 = (i + 1 < timeline.size()) ? timeline[i + 1].value1 : frame.value1;
                        }
                        
                        convertCurve3xTo4x(frame.curve, time1, value1, time2, value2);
                    }
                }
            }
        }
        
        // 处理IK约束动画
        for (auto& [ikName, ikTimeline] : animation.ik) {
            for (size_t i = 0; i < ikTimeline.size(); ++i) {
                auto& frame = ikTimeline[i];
                
                if (frame.curveType == CurveType::CURVE_BEZIER && !frame.curve.empty()) {
                    float time1 = frame.time;
                    float time2 = (i + 1 < ikTimeline.size()) ? ikTimeline[i + 1].time : frame.time + 1.0f;
                    float value1 = frame.value1; // mix值
                    float value2 = (i + 1 < ikTimeline.size()) ? ikTimeline[i + 1].value1 : frame.value1;
                    
                    convertCurve3xTo4x(frame.curve, time1, value1, time2, value2);
                }
            }
        }
        
        // 处理变换约束动画
        for (auto& [transformName, transformTimeline] : animation.transform) {
            for (size_t i = 0; i < transformTimeline.size(); ++i) {
                auto& frame = transformTimeline[i];
                
                if (frame.curveType == CurveType::CURVE_BEZIER && !frame.curve.empty()) {
                    float time1 = frame.time;
                    float time2 = (i + 1 < transformTimeline.size()) ? transformTimeline[i + 1].time : frame.time + 1.0f;
                    float value1 = frame.value1; // mixRotate值
                    float value2 = (i + 1 < transformTimeline.size()) ? transformTimeline[i + 1].value1 : frame.value1;
                    
                    convertCurve3xTo4x(frame.curve, time1, value1, time2, value2);
                }
            }
        }
        
        // 处理路径约束动画
        for (auto& [pathName, pathTimelines] : animation.path) {
            for (auto& [timelineType, timeline] : pathTimelines) {
                for (size_t i = 0; i < timeline.size(); ++i) {
                    auto& frame = timeline[i];
                    
                    if (frame.curveType == CurveType::CURVE_BEZIER && !frame.curve.empty()) {
                        float time1 = frame.time;
                        float time2 = (i + 1 < timeline.size()) ? timeline[i + 1].time : frame.time + 1.0f;
                        float value1 = frame.value1;
                        float value2 = (i + 1 < timeline.size()) ? timeline[i + 1].value1 : frame.value1;
                        
                        convertCurve3xTo4x(frame.curve, time1, value1, time2, value2);
                    }
                }
            }
        }
    }
}

void convertSkeletonData4xTo3x(SkeletonData& skelData) {
    // 转换动画中的曲线数据
    for (auto& animation : skelData.animations) {
        // 处理骨骼动画
        for (auto& [boneName, boneTimelines] : animation.bones) {
            for (auto& [timelineType, timeline] : boneTimelines) {
                for (size_t i = 0; i < timeline.size(); ++i) {
                    auto& frame = timeline[i];
                    
                    if (frame.curveType == CurveType::CURVE_BEZIER && !frame.curve.empty()) {
                        float time1 = frame.time;
                        float time2 = (i + 1 < timeline.size()) ? timeline[i + 1].time : frame.time + 1.0f;
                        
                        // 根据时间轴类型确定值范围
                        float value1 = 0.0f, value2 = 0.0f;
                        
                        if (timelineType == "rotate") {
                            value1 = frame.value1;
                            value2 = (i + 1 < timeline.size()) ? timeline[i + 1].value1 : frame.value1;
                        } else if (timelineType == "translate" || timelineType == "scale" || timelineType == "shear") {
                            value1 = frame.value1;
                            value2 = (i + 1 < timeline.size()) ? timeline[i + 1].value1 : frame.value1;
                        }
                        
                        convertCurve4xTo3x(frame.curve, time1, value1, time2, value2);
                    }
                }
            }
        }
        
        // 处理插槽动画
        for (auto& [slotName, slotTimelines] : animation.slots) {
            for (auto& [timelineType, timeline] : slotTimelines) {
                for (size_t i = 0; i < timeline.size(); ++i) {
                    auto& frame = timeline[i];
                    
                    if (frame.curveType == CurveType::CURVE_BEZIER && !frame.curve.empty()) {
                        float time1 = frame.time;
                        float time2 = (i + 1 < timeline.size()) ? timeline[i + 1].time : frame.time + 1.0f;
                        
                        float value1 = 0.0f, value2 = 0.0f;
                        if (timelineType == "alpha") {
                            value1 = frame.value1;
                            value2 = (i + 1 < timeline.size()) ? timeline[i + 1].value1 : frame.value1;
                        }
                        
                        convertCurve4xTo3x(frame.curve, time1, value1, time2, value2);
                    }
                }
            }
        }
        
        // 处理IK约束动画
        for (auto& [ikName, ikTimeline] : animation.ik) {
            for (size_t i = 0; i < ikTimeline.size(); ++i) {
                auto& frame = ikTimeline[i];
                
                if (frame.curveType == CurveType::CURVE_BEZIER && !frame.curve.empty()) {
                    float time1 = frame.time;
                    float time2 = (i + 1 < ikTimeline.size()) ? ikTimeline[i + 1].time : frame.time + 1.0f;
                    float value1 = frame.value1;
                    float value2 = (i + 1 < ikTimeline.size()) ? ikTimeline[i + 1].value1 : frame.value1;
                    
                    convertCurve4xTo3x(frame.curve, time1, value1, time2, value2);
                }
            }
        }
        
        // 处理变换约束动画
        for (auto& [transformName, transformTimeline] : animation.transform) {
            for (size_t i = 0; i < transformTimeline.size(); ++i) {
                auto& frame = transformTimeline[i];
                
                if (frame.curveType == CurveType::CURVE_BEZIER && !frame.curve.empty()) {
                    float time1 = frame.time;
                    float time2 = (i + 1 < transformTimeline.size()) ? transformTimeline[i + 1].time : frame.time + 1.0f;
                    float value1 = frame.value1;
                    float value2 = (i + 1 < transformTimeline.size()) ? transformTimeline[i + 1].value1 : frame.value1;
                    
                    convertCurve4xTo3x(frame.curve, time1, value1, time2, value2);
                }
            }
        }
        
        // 处理路径约束动画
        for (auto& [pathName, pathTimelines] : animation.path) {
            for (auto& [timelineType, timeline] : pathTimelines) {
                for (size_t i = 0; i < timeline.size(); ++i) {
                    auto& frame = timeline[i];
                    
                    if (frame.curveType == CurveType::CURVE_BEZIER && !frame.curve.empty()) {
                        float time1 = frame.time;
                        float time2 = (i + 1 < timeline.size()) ? timeline[i + 1].time : frame.time + 1.0f;
                        float value1 = frame.value1;
                        float value2 = (i + 1 < timeline.size()) ? timeline[i + 1].value1 : frame.value1;
                        
                        convertCurve4xTo3x(frame.curve, time1, value1, time2, value2);
                    }
                }
            }
        }
    }
}
