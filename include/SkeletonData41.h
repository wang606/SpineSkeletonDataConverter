#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include "common.h"
#include "json.hpp"
using Json = nlohmann::ordered_json;

namespace spine41 {

typedef std::optional<std::string> OptStr;
typedef std::vector<unsigned char> Binary; 

/* enum */

enum Inherit {
    Inherit_Normal = 0,
    Inherit_OnlyTranslation,
    Inherit_NoRotationOrReflection,
    Inherit_NoScale,
    Inherit_NoScaleOrReflection
}; 

enum BlendMode {
    BlendMode_Normal = 0,
    BlendMode_Additive,
    BlendMode_Multiply,
    BlendMode_Screen
};

enum PositionMode {
    PositionMode_Fixed = 0,
    PositionMode_Percent
};

enum SpacingMode {
    SpacingMode_Length = 0,
    SpacingMode_Fixed,
    SpacingMode_Percent,
    SpacingMode_Proportional
};

enum RotateMode {
    RotateMode_Tangent = 0,
    RotateMode_Chain,
    RotateMode_ChainScale
};

enum AttachmentType {
    AttachmentType_Region,
    AttachmentType_Boundingbox,
    AttachmentType_Mesh,
    AttachmentType_Linkedmesh,
    AttachmentType_Path,
    AttachmentType_Point,
    AttachmentType_Clipping
};

enum SequenceMode {
    hold = 0,
    once = 1,
    loop = 2,
    pingpong = 3,
    onceReverse = 4,
    loopReverse = 5,
    pingpongReverse = 6
}; 

enum BoneTimelineType {
    BONE_ROTATE = 0,
    BONE_TRANSLATE = 1,
    BONE_TRANSLATEX = 2,
    BONE_TRANSLATEY = 3,
    BONE_SCALE = 4,
    BONE_SCALEX = 5,
    BONE_SCALEY = 6,
    BONE_SHEAR = 7,
    BONE_SHEARX = 8,
    BONE_SHEARY = 9
}; 

enum SlotTimelineType {
    SLOT_ATTACHMENT = 0,
    SLOT_RGBA = 1,
    SLOT_RGB = 2,
    SLOT_RGBA2 = 3,
    SLOT_RGB2 = 4,
    SLOT_ALPHA = 5
};

enum AttachmentTimelineType {
    ATTACHMENT_DEFORM = 0,
    ATTACHMENT_SEQUENCE = 1
};

enum PathTimelineType {
    PATH_POSITION = 0,
    PATH_SPACING = 1,
    PATH_MIX = 2
};

enum CurveType {
    CURVE_LINEAR = 0,
    CURVE_STEPPED = 1,
    CURVE_BEZIER = 2
};

/* structs */

struct Color {
    unsigned char r = 0xff, g = 0xff, b = 0xff, a = 0xff;
    bool operator==(const Color& other) const {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }
    bool operator!=(const Color& other) const {
        return !(*this == other);
    }
};

struct Sequence {
    int count = 0; 
    int start = 1; 
    int digits = 0; 
    int setupIndex = 0; 
}; 

typedef std::optional<Color> OptColor; 
typedef std::optional<Sequence> OptSequence;

struct RegionAttachment {
    float x = 0.0f, y = 0.0f, rotation = 0.0f, scaleX = 1.0f, scaleY = 1.0f, width = 32.0f, height = 32.0f; 
    OptColor color = std::nullopt; 
    OptSequence sequence = std::nullopt; 
}; 

struct MeshAttachment {
    float width = 32.0f, height = 32.0f; 
    OptColor color = std::nullopt;
    OptSequence sequence = std::nullopt; 
    int hullLength = 0; 
    std::vector<float> uvs; 
    std::vector<unsigned short> triangles; 
    std::vector<unsigned short> edges; 
    std::vector<float> vertices; 
}; 

struct LinkedmeshAttachment {
    float width = 32.0f, height = 32.0f; 
    OptColor color = std::nullopt;
    OptSequence sequence = std::nullopt; 
    std::string parentMesh; 
    int timelines = 1; 
    std::string skin; 
}; 

struct BoundingboxAttachment {
    int vertexCount = 0; 
    std::vector<float> vertices;
    OptColor color = std::nullopt;
}; 

struct PathAttachment {
    int vertexCount = 0; 
    std::vector<float> vertices;
    std::vector<float> lengths;
    bool closed = false;
    bool constantSpeed = true;
    OptColor color = std::nullopt;
}; 

struct PointAttachment {
    float x = 0.0f, y = 0.0f, rotation = 0.0f;
    OptColor color = std::nullopt;
}; 

struct ClippingAttachment {
    int vertexCount = 0; 
    std::vector<float> vertices;
    OptStr endSlot = std::nullopt;
    OptColor color = std::nullopt;
}; 

struct Attachment {
    std::string name; 
    std::string path; 
    AttachmentType type; 
    std::variant<
        RegionAttachment,
        BoundingboxAttachment,
        MeshAttachment,
        LinkedmeshAttachment,
        PathAttachment,
        PointAttachment,
        ClippingAttachment
    > data;
}; 

struct TimelineFrame {
    float time = 0.0f; 
    OptStr str1 = std::nullopt, str2 = std::nullopt; 
    int int1 = 0; 
    float value1 = 0.0f, value2 = 0.0f, value3 = 0.0f, value4 = 0.0f, value5 = 0.0f, value6 = 0.0f; 
    OptColor color1 = std::nullopt, color2 = std::nullopt;
    CurveType curveType = CurveType::CURVE_LINEAR; 
    std::vector<float> curve; 

    Inherit inherit = Inherit::Inherit_Normal;
    SequenceMode sequenceMode = SequenceMode::hold;
    bool bendPositive = true, compress = false, stretch = false; 
    std::vector<float> vertices; 
    std::vector<std::pair<std::string, int>> offsets; 
}; 

typedef std::vector<TimelineFrame> Timeline; 
typedef std::map<std::string, Timeline> MultiTimeline; 

/* data */

struct BoneData {
    OptStr name = std::nullopt; 
    OptStr parent = std::nullopt;
    float length = 0; 
    float x = 0, y = 0, rotation = 0, scaleX = 1, scaleY = 1, shearX = 0, shearY = 0; 
    Inherit inherit = Inherit_Normal;
    bool skinRequired = false; 
    OptColor color = std::nullopt; // Color { 0x9b, 0x9b, 0x9b, 0xff };
}; 

struct SlotData {
    OptStr name = std::nullopt; 
    OptStr bone = std::nullopt; 
    OptColor color = std::nullopt;
    OptColor darkColor = std::nullopt;
    OptStr attachmentName = std::nullopt; 
    BlendMode blendMode = BlendMode_Normal; 
}; 

struct IKConstraintData {
    OptStr name = std::nullopt;
    size_t order = 0;
    bool skinRequired = false;
    std::vector<std::string> bones; 
    OptStr target = std::nullopt; 
    bool bendPositive = true; 
    bool compress = false; 
    bool stretch = false; 
    bool uniform = false; 
    float mix = 1.0f; 
    float softness = 0.0f; 
};

struct TransformConstraintData {
    OptStr name = std::nullopt;
    size_t order = 0;
    bool skinRequired = false;
    std::vector<std::string> bones; 
    OptStr target = std::nullopt;
    float mixRotate = 1.0f, mixX = 1.0f, mixY = 1.0f, mixScaleX = 1.0f, mixScaleY = 1.0f, mixShearY = 1.0f; 
    float offsetRotation = 0.0f, offsetX = 0.0f, offsetY = 0.0f, offsetScaleX = 0.0f, offsetScaleY = 0.0f, offsetShearY = 0.0f;
    bool relative = false, local = false; 
};

struct PathConstraintData {
    OptStr name = std::nullopt;
    size_t order = 0;
    bool skinRequired = false;
    std::vector<std::string> bones;
    OptStr target = std::nullopt;
    PositionMode positionMode = PositionMode::PositionMode_Percent;
    SpacingMode spacingMode = SpacingMode::SpacingMode_Length;
    RotateMode rotateMode = RotateMode::RotateMode_Tangent;
    float offsetRotation = 0.0f;
    float position = 0.0f, spacing = 0.0f;
    float mixRotate = 1.0f, mixX = 1.0f, mixY = 1.0f;
};

struct Skin {
    std::string name = "";
    std::map<std::string, std::map<std::string, Attachment>> attachments;
    std::vector<std::string> bones; 
    std::vector<std::string> ik;
    std::vector<std::string> transform;
    std::vector<std::string> path;
}; 

struct EventData {
    std::string name = ""; 
    int intValue = 0;
    float floatValue = 0.0f;
    OptStr stringValue = std::nullopt;
    OptStr audioPath = std::nullopt;
    float volume = 1.0f;
    float balance = 0.0f;
}; 

struct Animation {
    std::string name = "";
    std::map<std::string, MultiTimeline> slots; 
    std::map<std::string, MultiTimeline> bones;
    std::map<std::string, Timeline> ik;
    std::map<std::string, Timeline> transform;
    std::map<std::string, MultiTimeline> path;
    std::map<std::string, std::map<std::string, std::map<std::string, MultiTimeline>>> attachments;
    Timeline drawOrder;
    Timeline events;
};

struct SkeletonData {
    uint64_t hash = 0; 
    OptStr version = std::nullopt; 
    float x = 0.0f, y = 0.0f, width = 0.0f, height = 0.0f; 
    bool nonessential = false;
    float fps = 30.0f; 
    OptStr imagesPath = std::nullopt; 
    OptStr audioPath = std::nullopt; 
    std::vector<std::string> strings; 
    std::vector<BoneData> bones; 
    std::vector<SlotData> slots; 
    std::vector<Skin> skins; 
    std::vector<EventData> events; 
    std::vector<Animation> animations; 
    std::vector<IKConstraintData> ikConstraints; 
    std::vector<TransformConstraintData> transformConstraints; 
    std::vector<PathConstraintData> pathConstraints; 
}; 

SkeletonData readBinaryData(const Binary&);
Binary writeBinaryData(SkeletonData&);
SkeletonData readJsonData(const Json&);
Json writeJsonData(const SkeletonData&);

}
