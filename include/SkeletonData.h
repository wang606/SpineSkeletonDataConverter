#ifndef SKELETONDATA_H
#define SKELETONDATA_H

#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include "json.hpp"
using Json = nlohmann::ordered_json;

typedef std::optional<std::string> OptStr;
typedef std::vector<unsigned char> Binary; 

struct DataInput {
    const unsigned char* cursor;
    const unsigned char* end;
};

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
    BONE_SHEARY = 9,
    BONE_INHERIT = 10
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

enum PhysicsTimelineType {
    PHYSICS_INERTIA = 0,
    PHYSICS_STRENGTH = 1,
    PHYSICS_DAMPING = 2,
    PHYSICS_MASS = 4,
    PHYSICS_WIND = 5,
    PHYSICS_GRAVITY = 6,
    PHYSICS_MIX = 7,
    PHYSICS_RESET = 8
};

enum CurveType {
    CURVE_LINEAR = 0,
    CURVE_STEPPED = 1,
    CURVE_BEZIER = 2
};

/* constants */

// json reader

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

// json writer

static const std::map<Inherit, std::string> inheritString = {
    { Inherit_Normal, "normal" },
    { Inherit_OnlyTranslation, "onlyTranslation" },
    { Inherit_NoRotationOrReflection, "noRotationOrReflection" },
    { Inherit_NoScale, "noScale" },
    { Inherit_NoScaleOrReflection, "noScaleOrReflection" }
};

static const std::map<BlendMode, std::string> blendModeString = {
    { BlendMode_Normal, "normal" },
    { BlendMode_Additive, "additive" },
    { BlendMode_Multiply, "multiply" },
    { BlendMode_Screen, "screen" }
};

static const std::map<PositionMode, std::string> positionModeString = {
    { PositionMode_Fixed, "fixed" },
    { PositionMode_Percent, "percent" }
};

static const std::map<SpacingMode, std::string> spacingModeString = {
    { SpacingMode_Length, "length" },
    { SpacingMode_Fixed, "fixed" },
    { SpacingMode_Percent, "percent" },
    { SpacingMode_Proportional, "proportional" }
};

static const std::map<RotateMode, std::string> rotateModeString = {
    { RotateMode_Tangent, "tangent" },
    { RotateMode_Chain, "chain" },
    { RotateMode_ChainScale, "chainScale" }
};

static const std::map<AttachmentType, std::string> attachmentTypeString = {
    { AttachmentType_Region, "region" },
    { AttachmentType_Boundingbox, "boundingbox" },
    { AttachmentType_Mesh, "mesh" },
    { AttachmentType_Linkedmesh, "linkedmesh" },
    { AttachmentType_Path, "path" },
    { AttachmentType_Point, "point" },
    { AttachmentType_Clipping, "clipping" }
};

static const std::map<SequenceMode, std::string> sequenceModeString = {
    { SequenceMode::hold, "hold" },
    { SequenceMode::once, "once" },
    { SequenceMode::loop, "loop" },
    { SequenceMode::pingpong, "pingpong" },
    { SequenceMode::onceReverse, "onceReverse" },
    { SequenceMode::loopReverse, "loopReverse" },
    { SequenceMode::pingpongReverse, "pingpongReverse" }
};

// binary writer

static const std::map<std::string, SlotTimelineType> slotTimelineTypeMap = {
    {"attachment", SLOT_ATTACHMENT}, 
    {"rgba", SLOT_RGBA}, 
    {"rgb", SLOT_RGB}, 
    {"rgba2", SLOT_RGBA2}, 
    {"rgb2", SLOT_RGB2}, 
    {"alpha", SLOT_ALPHA}
};

static const std::map<std::string, BoneTimelineType> boneTimelineTypeMap = {
    {"rotate", BONE_ROTATE}, 
    {"translate", BONE_TRANSLATE}, 
    {"translatex", BONE_TRANSLATEX}, 
    {"translatey", BONE_TRANSLATEY}, 
    {"scale", BONE_SCALE}, 
    {"scalex", BONE_SCALEX}, 
    {"scaley", BONE_SCALEY}, 
    {"shear", BONE_SHEAR}, 
    {"shearx", BONE_SHEARX}, 
    {"sheary", BONE_SHEARY}, 
    {"inherit", BONE_INHERIT}
};

static const std::map<std::string, PathTimelineType> pathTimelineTypeMap = {
    {"position", PATH_POSITION}, 
    {"spacing", PATH_SPACING}, 
    {"mix", PATH_MIX}
};

static const std::map<std::string, PhysicsTimelineType> physicsTimelineTypeMap = {
    {"inertia", PHYSICS_INERTIA}, 
    {"strength", PHYSICS_STRENGTH}, 
    {"damping", PHYSICS_DAMPING}, 
    {"mass", PHYSICS_MASS}, 
    {"wind", PHYSICS_WIND}, 
    {"gravity", PHYSICS_GRAVITY}, 
    {"mix", PHYSICS_MIX}, 
    {"reset", PHYSICS_RESET}
};

static const std::map<std::string, AttachmentTimelineType> attachmentTimelineTypeMap = {
    {"deform", ATTACHMENT_DEFORM}, 
    {"sequence", ATTACHMENT_SEQUENCE}
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
    int skinIndex = -1; // temporary field used for binary format reading
    OptStr skin = std::nullopt; 
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

/*
SlotTimelineTypes:
    "attachment", "rgba", "rgb", "rgba2", "rgb2", "alpha"

BoneTimelineTypes: 
    "rotate", "translate", "translatex", "translatey", "scale", "scalex", "scaley", "shear", "shearx", "sheary", "inherit"

PathTimelineTypes:
    "position", "spacing", "mix"

PhysicsTimelineTypes:
    "inertia", "strength", "damping", "mass", "wind", "gravity", "mix", "reset"

AttachmentTimelineTypes:
    "deform", "sequence"
*/

/* data */

struct BoneData {
    OptStr name = std::nullopt; 
    OptStr parent = std::nullopt;
    float length = 0; 
    float x = 0, y = 0, rotation = 0, scaleX = 1, scaleY = 1, shearX = 0, shearY = 0; 
    Inherit inherit = Inherit_Normal;
    bool skinRequired = false; 
    OptColor color = std::nullopt; // Color { 0x9b, 0x9b, 0x9b, 0xff };
    OptStr icon = std::nullopt; 
    bool visible = true; 
}; 

struct SlotData {
    OptStr name = std::nullopt; 
    OptStr bone = std::nullopt; 
    OptColor color = std::nullopt;
    OptColor darkColor = std::nullopt;
    OptStr attachmentName = std::nullopt; 
    BlendMode blendMode = BlendMode_Normal; 
    bool visible = true; 
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

struct PhysicsConstraintData {
    OptStr name = std::nullopt;
    size_t order = 0;
    bool skinRequired = false;
    OptStr bone = std::nullopt;
    float x = 0.0f, y = 0.0f, rotate = 0.0f, scaleX = 0.0f, shearX = 0.0f, limit = 5000.0f;
    float fps = 60.0f, inertia = 1.0f, strength = 100.0f, damping = 1.0f, mass = 1.0f, wind = 0.0f, gravity = 0.0f, mix = 1.0f;
    bool inertiaGlobal = false, strengthGlobal = false, dampingGlobal = false, massGlobal = false, windGlobal = false, gravityGlobal = false, mixGlobal = false;
};

struct Skin {
    std::string name = "";
    std::map<std::string, std::map<std::string, Attachment>> attachments;
    std::vector<std::string> bones; 
    std::vector<std::string> ik;
    std::vector<std::string> transform;
    std::vector<std::string> path;
    std::vector<std::string> physics;
    OptColor color = std::nullopt;
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
    std::map<std::string, MultiTimeline> physics;
    std::map<std::string, std::map<std::string, std::map<std::string, MultiTimeline>>> attachments;
    Timeline drawOrder;
    Timeline events;
};

struct SkeletonData {
    uint64_t hash = 0; 
    OptStr hashString = std::nullopt;  // used for Spine 3.8 below
    OptStr version = std::nullopt; 
    float x = 0.0f, y = 0.0f, width = 0.0f, height = 0.0f; 
    float referenceScale = 100.0f; 
    bool nonessential = false;
    float fps = 30.0f; 
    OptStr imagesPath = std::nullopt; 
    OptStr audioPath = std::nullopt; 
    std::vector<std::string> strings; 
    std::vector<BoneData> bones; 
    std::vector<SlotData> slots; 
    std::vector<IKConstraintData> ikConstraints; 
    std::vector<TransformConstraintData> transformConstraints; 
    std::vector<PathConstraintData> pathConstraints; 
    std::vector<PhysicsConstraintData> physicsConstraints; 
    std::vector<Skin> skins; 
    std::vector<EventData> events; 
    std::vector<Animation> animations; 
}; 

/* common functions */

Color stringToColor(const std::string& str, bool hasAlpha); 
std::string colorToString(const Color& color, bool hasAlpha); 

unsigned char readByte(DataInput*); 
signed char readSByte(DataInput*);
bool readBoolean(DataInput*); 
int readInt(DataInput*); 
Color readColor(DataInput*, bool hasAlpha = true); 
int readVarint(DataInput*, bool optimizePositive); 
float readFloat(DataInput*); 
OptStr readString(DataInput*); 
OptStr readStringRef(DataInput*, SkeletonData*); 

void writeByte(Binary&, unsigned char); 
void writeSByte(Binary&, signed char);
void writeBoolean(Binary&, bool); 
void writeInt(Binary&, int);
void writeColor(Binary&, const Color&, bool hasAlpha = true); 
void writeVarint(Binary&, int, bool optimizePositive);
void writeFloat(Binary&, float); 
void writeString(Binary&, const OptStr&);
void writeStringRef(Binary&, const OptStr&, const SkeletonData&);

std::string dumpJson(const Json&); 
std::string uint64ToBase64(uint64_t);
uint64_t base64ToUint64(const std::string&);

namespace spine36 {
    SkeletonData readBinaryData(const Binary&);
    Binary writeBinaryData(SkeletonData&);
    SkeletonData readJsonData(const Json&);
    Json writeJsonData(const SkeletonData&);
}

namespace spine37 {
    SkeletonData readBinaryData(const Binary&);
    Binary writeBinaryData(SkeletonData&);
    SkeletonData readJsonData(const Json&);
    Json writeJsonData(const SkeletonData&);
}
namespace spine38 {
    SkeletonData readBinaryData(const Binary&);
    Binary writeBinaryData(SkeletonData&);
    SkeletonData readJsonData(const Json&);
    Json writeJsonData(const SkeletonData&);
}
namespace spine40 {
    SkeletonData readBinaryData(const Binary&);
    Binary writeBinaryData(SkeletonData&);
    SkeletonData readJsonData(const Json&);
    Json writeJsonData(const SkeletonData&);
}
namespace spine41 {
    SkeletonData readBinaryData(const Binary&);
    Binary writeBinaryData(SkeletonData&);
    SkeletonData readJsonData(const Json&);
    Json writeJsonData(const SkeletonData&);
}
namespace spine42 {
    SkeletonData readBinaryData(const Binary&);
    Binary writeBinaryData(SkeletonData&);
    SkeletonData readJsonData(const Json&);
    Json writeJsonData(const SkeletonData&);
}

void convertCurve3xTo4x(SkeletonData& skeleton);
void convertCurve4xTo3x(SkeletonData& skeleton);

#endif // SKELETONDATA_H