#include "SkeletonData.h"
#include <cmath>
#include <utility>

namespace {

constexpr float epsilon = 0.0001f;
constexpr float max3xRotateDelta = 179.0f;

struct BezierPoint {
    float x = 0.0f;
    float y = 0.0f;
};

float normalizeRotationDelta(float delta) {
    delta = std::fmod(delta, 360.0f);
    if (delta > 180.0f) delta -= 360.0f;
    if (delta < -180.0f) delta += 360.0f;
    return delta;
}

float resolveAmbiguousHalfTurn(float delta, float previousDelta) {
    if (std::fabs(std::fabs(delta) - 180.0f) > epsilon) return delta;
    if (previousDelta > 0.0f) return 180.0f;
    if (previousDelta < 0.0f) return -180.0f;
    return 180.0f;
}

void convertRotateTimeline3xTo4x(Timeline& timeline) {
    if (timeline.size() < 2) return;

    float unwrappedValue = timeline[0].value1;
    float previousDelta = 0.0f;
    for (size_t i = 1; i < timeline.size(); ++i) {
        float delta = normalizeRotationDelta(timeline[i].value1 - timeline[i - 1].value1);
        delta = resolveAmbiguousHalfTurn(delta, previousDelta);
        unwrappedValue += delta;
        timeline[i].value1 = unwrappedValue;
        if (std::fabs(delta) > epsilon) previousDelta = delta;
    }
}

BezierPoint lerp(const BezierPoint& a, const BezierPoint& b, float t) {
    return {
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t
    };
}

BezierPoint evaluateBezier(const BezierPoint& p0, const BezierPoint& p1, const BezierPoint& p2, const BezierPoint& p3, float t) {
    BezierPoint p01 = lerp(p0, p1, t);
    BezierPoint p12 = lerp(p1, p2, t);
    BezierPoint p23 = lerp(p2, p3, t);
    BezierPoint p012 = lerp(p01, p12, t);
    BezierPoint p123 = lerp(p12, p23, t);
    return lerp(p012, p123, t);
}

TimelineFrame makeRotateFrame(float time, float value) {
    TimelineFrame frame;
    frame.time = time;
    frame.value1 = value;
    frame.curveType = CurveType::CURVE_LINEAR;
    return frame;
}

bool findBezierTForValue(const BezierPoint& p0, const BezierPoint& p1, const BezierPoint& p2, const BezierPoint& p3, float value, float& result) {
    constexpr int samples = 100;
    float previousT = 0.0f;
    float previousY = p0.y - value;

    for (int i = 1; i <= samples; ++i) {
        float currentT = static_cast<float>(i) / samples;
        float currentY = evaluateBezier(p0, p1, p2, p3, currentT).y - value;
        if (std::fabs(currentY) <= epsilon) {
            result = currentT;
            return result > epsilon && result < 1.0f - epsilon;
        }
        if (previousY * currentY < 0.0f) {
            float low = previousT;
            float high = currentT;
            for (int step = 0; step < 24; ++step) {
                float mid = (low + high) * 0.5f;
                float midY = evaluateBezier(p0, p1, p2, p3, mid).y - value;
                if (previousY * midY <= 0.0f) {
                    high = mid;
                    currentY = midY;
                } else {
                    low = mid;
                    previousY = midY;
                }
            }
            result = (low + high) * 0.5f;
            return result > epsilon && result < 1.0f - epsilon;
        }
        previousT = currentT;
        previousY = currentY;
    }

    return false;
}

bool splitLinearRotateSegment(const TimelineFrame& start, const TimelineFrame& end, float value, TimelineFrame& leftStart, TimelineFrame& mid) {
    float delta = end.value1 - start.value1;
    if (std::fabs(delta) <= epsilon) return false;
    float ratio = (value - start.value1) / delta;
    if (ratio <= epsilon || ratio >= 1.0f - epsilon) return false;

    leftStart = start;
    leftStart.curveType = CurveType::CURVE_LINEAR;
    leftStart.curve.clear();
    mid = makeRotateFrame(start.time + (end.time - start.time) * ratio, value);
    return true;
}

bool splitBezierRotateSegment(const TimelineFrame& start, const TimelineFrame& end, float value, TimelineFrame& leftStart, TimelineFrame& mid) {
    if (start.curve.size() < 4) return splitLinearRotateSegment(start, end, value, leftStart, mid);

    BezierPoint p0{start.time, start.value1};
    BezierPoint p1{start.curve[0], start.curve[1]};
    BezierPoint p2{start.curve[2], start.curve[3]};
    BezierPoint p3{end.time, end.value1};

    float t = 0.0f;
    if (!findBezierTForValue(p0, p1, p2, p3, value, t)) return false;

    BezierPoint p01 = lerp(p0, p1, t);
    BezierPoint p12 = lerp(p1, p2, t);
    BezierPoint p23 = lerp(p2, p3, t);
    BezierPoint p012 = lerp(p01, p12, t);
    BezierPoint p123 = lerp(p12, p23, t);
    BezierPoint p0123 = lerp(p012, p123, t);

    if (p0123.x <= start.time + epsilon || p0123.x >= end.time - epsilon) return false;

    leftStart = start;
    leftStart.curveType = CurveType::CURVE_BEZIER;
    leftStart.curve = {p01.x, p01.y, p012.x, p012.y};

    mid = makeRotateFrame(p0123.x, p0123.y);
    mid.curveType = CurveType::CURVE_BEZIER;
    mid.curve = {p123.x, p123.y, p23.x, p23.y};
    return true;
}

bool splitRotateSegment(const TimelineFrame& start, const TimelineFrame& end, float value, TimelineFrame& leftStart, TimelineFrame& mid) {
    switch (start.curveType) {
        case CurveType::CURVE_LINEAR:
            return splitLinearRotateSegment(start, end, value, leftStart, mid);
        case CurveType::CURVE_BEZIER:
            return splitBezierRotateSegment(start, end, value, leftStart, mid);
        case CurveType::CURVE_STEPPED:
            return false;
    }
    return false;
}

void convertRotateTimeline4xTo3x(Timeline& timeline) {
    if (timeline.size() < 2) return;

    Timeline converted;
    converted.reserve(timeline.size() * 2);
    converted.push_back(timeline.front());

    for (size_t i = 1; i < timeline.size(); ++i) {
        converted.back().curveType = timeline[i - 1].curveType;
        converted.back().curve = timeline[i - 1].curve;
        TimelineFrame target = timeline[i];

        // Spine 3.x rotate timelines always choose the shortest path between adjacent keys
        // and treat an exact -180 degree delta as +180. Spine 4.x can represent
        // absolute/unwrapped values such as 360, 540, or -180 in one segment.
        //
        // For linear and monotonic Bezier segments we split the original 4.x segment and
        // use De Casteljau for Bezier control points, so inserted helper keys preserve the
        // original change curve. Stepped segments do not need helper keys because the value
        // changes instantaneously at the next key. Some non-monotonic/multi-turn Bezier
        // motion still cannot be represented losslessly in 3.x without sampling, and
        // sampling would approximate rather than preserve the authored curve.
        while (true) {
            float delta = target.value1 - converted.back().value1;
            if (delta <= max3xRotateDelta + epsilon && delta >= -max3xRotateDelta - epsilon) break;

            float splitValue = converted.back().value1 + (delta > 0.0f ? max3xRotateDelta : -max3xRotateDelta);
            TimelineFrame leftStart;
            TimelineFrame mid;
            if (!splitRotateSegment(converted.back(), target, splitValue, leftStart, mid)) {
                break;
            }

            converted.back() = leftStart;
            converted.push_back(mid);
        }

        converted.push_back(target);
    }

    timeline = std::move(converted);
}

void convertRotateTimelines(SkeletonData& skeleton, void (*convertTimeline)(Timeline&)) {
    for (auto& animation : skeleton.animations) {
        for (auto& [boneName, multiTimeline] : animation.bones) {
            auto it = multiTimeline.find("rotate");
            if (it != multiTimeline.end()) {
                convertTimeline(it->second);
            }
        }
    }
}

}

void convertRotateTimeline3xTo4x(SkeletonData& skeleton) {
    convertRotateTimelines(skeleton, convertRotateTimeline3xTo4x);
}

void convertRotateTimeline4xTo3x(SkeletonData& skeleton) {
    convertRotateTimelines(skeleton, convertRotateTimeline4xTo3x);
}
