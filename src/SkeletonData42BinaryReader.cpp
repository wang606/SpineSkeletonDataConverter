#include "SkeletonData42.h"
using namespace spine42;

struct DataInput {
    const unsigned char* cursor;
    const unsigned char* end;
};

unsigned char readByte(DataInput* input) {
    return *input->cursor++;
}

signed char readSByte(DataInput* input) {
    return (signed char)readByte(input);
}

bool readBoolean(DataInput* input) {
    return readByte(input) != 0;
}

int readInt(DataInput* input) {
    int result = readByte(input);
    result <<= 8;
    result |= readByte(input);
    result <<= 8;
    result |= readByte(input);
    result <<= 8;
    result |= readByte(input);
    return result;
}

Color readColor(DataInput* input) {
    Color color; 
    color.r = readByte(input); 
    color.g = readByte(input); 
    color.b = readByte(input); 
    color.a = readByte(input); 
    return color; 
}

int readVarint(DataInput* input, bool optimizePositive) {
    unsigned char b = readByte(input); 
    int value = b & 0x7F; 
    if (b & 0x80) {
        b = readByte(input); 
        value |= (b & 0x7F) << 7; 
        if (b & 0x80) {
            b = readByte(input); 
            value |= (b & 0x7F) << 14; 
            if (b & 0x80) {
                b = readByte(input); 
                value |= (b & 0x7F) << 21; 
                if (b & 0x80) value |= (readByte(input) & 0x7F) << 28; 
            }
        }
    }
    if (!optimizePositive) value = (((unsigned int)value >> 1) ^ -(value & 1));
    return value;
}

float readFloat(DataInput* input) {
    union {
        int intValue; 
        float floatValue; 
    } intToFloat; 
    intToFloat.intValue = readInt(input);
    return intToFloat.floatValue;
}

OptStr readString(DataInput* input) {
    int length = readVarint(input, true); 
    if (length == 0) return std::nullopt;
    std::string string; 
    string.resize(length - 1); 
    memcpy(string.data(), input->cursor, length - 1); 
    input->cursor += length - 1; 
    return string; 
}

OptStr readStringRef(DataInput* input, SkeletonData* skeletonData) {
    int index = readVarint(input, true); 
    if (index == 0) return std::nullopt; 
    else return skeletonData->strings[index - 1]; 
}

Sequence readSequence(DataInput* input) {
    Sequence sequence; 
    sequence.count = readVarint(input, true);
    sequence.start = readVarint(input, true);
    sequence.digits = readVarint(input, true);
    sequence.setupIndex = readVarint(input, true);
    return sequence;
}

void readFloatArray(DataInput* input, int n, std::vector<float>& array) {
    array.resize(n, 0);
    for (int i = 0; i < n; i++)
        array[i] = readFloat(input);
}

void readShortArray(DataInput* input, int n, std::vector<unsigned short>& array) {
    array.resize(n, 0);
    for (int i = 0; i < n; i++)
        array[i] = (short) readVarint(input, true); 
}

int readVertices(DataInput* input, std::vector<float>& vertices, std::vector<int>& bones, bool weighted) {
    int vertexCount = readVarint(input, true); 
    int verticesLength = vertexCount << 1; 
    if (!weighted) {
        readFloatArray(input, verticesLength, vertices);
        return verticesLength; 
    }
    vertices.reserve(verticesLength * 3 * 3);
    bones.reserve(verticesLength * 3);
    for (int i = 0; i < vertexCount; i++) {
        int boneCount = readVarint(input, true); 
        bones.push_back(boneCount);
        for (int ii = 0; ii < boneCount; ii++) {
            bones.push_back(readVarint(input, true));
            vertices.push_back(readFloat(input));
            vertices.push_back(readFloat(input));
            vertices.push_back(readFloat(input));
        }
    }
    return verticesLength; 
}

Attachment readAttachment(DataInput* input, Skin* skin, int slotIndex, const std::string& attachmentName, SkeletonData* skeletonData, bool nonessential) {
    Attachment attachment; 
    int flags = readByte(input); 
    attachment.name = (flags & 8) != 0 ? readStringRef(input, skeletonData) : attachmentName; 
    AttachmentType type = static_cast<AttachmentType>(flags & 0x7);
    switch (type) {
        case AttachmentType_Region: {
            attachment.path = (flags & 16) != 0 ? readStringRef(input, skeletonData) : attachment.name;
            
            attachment.x = readFloat(input);
            attachment.y = readFloat(input);
            attachment.scaleX = readFloat(input);
            attachment.scaleY = readFloat(input);
            attachment.rotation = readFloat(input);
            attachment.width = readFloat(input);
            attachment.height = readFloat(input);
            attachment.color = readColor(input);
            return attachment;
        }
    }
}

std::optional<Skin> readSkin(DataInput* input, bool defaultSkin, SkeletonData* skeletonData, bool nonessential) {
    Skin skin; 
    int slotCount = 0; 
    if (defaultSkin) {
        slotCount = readVarint(input, true); 
        if (slotCount == 0) return std::nullopt; 
        skin.name = "default"; 
    } else {
        skin.name = readString(input);
        if (nonessential) skin.color = readColor(input); 
        for (int i = 0, n = readVarint(input, true); i < n; i++) {
            int boneIndex = readVarint(input, true); 
            if (boneIndex >= (int) skeletonData->bones.size()) return std::nullopt; 
            skin.bones.push_back(boneIndex);
        }
        for (int i = 0, n = readVarint(input, true); i < n; i++) {
            int ikIndex = readVarint(input, true); 
            if (ikIndex >= (int) skeletonData->ikConstraints.size()) return std::nullopt; 
            skin.constraints.push_back(&skeletonData->ikConstraints[ikIndex]);
        }
        for (int i = 0, n = readVarint(input, true); i < n; i++) {
            int transformIndex = readVarint(input, true); 
            if (transformIndex >= (int) skeletonData->transformConstraints.size()) return std::nullopt; 
            skin.constraints.push_back(&skeletonData->transformConstraints[transformIndex]);
        }
        for (int i = 0, n = readVarint(input, true); i < n; i++) {
            int pathIndex = readVarint(input, true);
            if (pathIndex >= (int) skeletonData->pathConstraints.size()) return std::nullopt;
            skin.constraints.push_back(&skeletonData->pathConstraints[pathIndex]);
        }
        for (int i = 0, n = readVarint(input, true); i < n; i++) {
            int physicsIndex = readVarint(input, true);
            if (physicsIndex >= (int) skeletonData->physicsConstraints.size()) return std::nullopt;
            skin.constraints.push_back(&skeletonData->physicsConstraints[physicsIndex]);
        }
        slotCount = readVarint(input, true);
    }
    for (int i = 0; i < slotCount; i++) {
        int slotIndex = readVarint(input, true); 

    }
}

SkeletonData readBinaryData(const Binary& binary) {
    SkeletonData skeletonData;
    DataInput input; 
    input.cursor = binary.data(); 
    input.end = binary.data() + binary.size();

    char buffer[16] = {0}; 
    skeletonData.lowHash = readInt(&input); 
    skeletonData.highHash = readInt(&input); 
    std::string hashString = ""; 
    snprintf(buffer, 16, "%x", skeletonData.highHash);
    hashString += buffer;
    snprintf(buffer, 16, "%x", skeletonData.lowHash);
    hashString += buffer;
    skeletonData.hash = hashString;
    skeletonData.version = readString(&input).value();

    skeletonData.x = readFloat(&input); 
    skeletonData.y = readFloat(&input);
    skeletonData.width = readFloat(&input);
    skeletonData.height = readFloat(&input);
    skeletonData.referenceScale = readFloat(&input); 

    skeletonData.nonessential = readBoolean(&input); 

    if (skeletonData.nonessential) {
        skeletonData.fps = readFloat(&input); 
        skeletonData.imagesPath = readString(&input).value(); 
        skeletonData.audioPath = readString(&input).value();
    }

    int numStrings = readVarint(&input, true);
    for (int i = 0; i < numStrings; i++)
        skeletonData.strings.push_back(readString(&input).value());
    
    /* Bones */
    int numBones = readVarint(&input, true);
    skeletonData.bones.resize(numBones); 
    for (int i = 0; i < numBones; i++) {
        skeletonData.bones[i].index = i; 
        skeletonData.bones[i].name = readString(&input);
        skeletonData.bones[i].parent = i == 0 ? 0 : readVarint(&input, true);
        skeletonData.bones[i].rotation = readFloat(&input);
        skeletonData.bones[i].x = readFloat(&input);
        skeletonData.bones[i].y = readFloat(&input);
        skeletonData.bones[i].scaleX = readFloat(&input);
        skeletonData.bones[i].scaleY = readFloat(&input);
        skeletonData.bones[i].shearX = readFloat(&input);
        skeletonData.bones[i].shearY = readFloat(&input);
        skeletonData.bones[i].length = readFloat(&input);
        skeletonData.bones[i].inherit = static_cast<Inherit>(readVarint(&input, true));
        skeletonData.bones[i].skinRequired = readBoolean(&input);
        if (skeletonData.nonessential) {
            skeletonData.bones[i].color = readColor(&input);
            skeletonData.bones[i].icon = readString(&input);
            skeletonData.bones[i].visible = readBoolean(&input);
        }
    }

    /* Slots */
    int slotCount = readVarint(&input, true);
    skeletonData.slots.resize(slotCount);
    for (int i = 0; i < slotCount; i++) {
        skeletonData.slots[i].index = i;
        skeletonData.slots[i].name = readString(&input);
        skeletonData.slots[i].boneData = readVarint(&input, true);
        skeletonData.slots[i].color = readColor(&input);
        unsigned char a = readByte(&input); 
        unsigned char r = readByte(&input);
        unsigned char g = readByte(&input);
        unsigned char b = readByte(&input);
        if (!(r == 0xff && g == 0xff && b == 0xff && a == 0xff)) {
            skeletonData.slots[i].darkColor = Color{r, g, b, a};
            skeletonData.slots[i].hasDarkColor = true; 
        }
        skeletonData.slots[i].attachmentName = readStringRef(&input, &skeletonData);
        skeletonData.slots[i].blendMode = static_cast<BlendMode>(readVarint(&input, true));
        if (skeletonData.nonessential) {
            skeletonData.slots[i].visible = readBoolean(&input);
        }
    }

    /* IK constraints */
    int ikConstraintsCount = readVarint(&input, true);
    skeletonData.ikConstraints.resize(ikConstraintsCount);
    for (int i = 0; i < ikConstraintsCount; i++) {
        skeletonData.ikConstraints[i].name = readString(&input);
        skeletonData.ikConstraints[i].order = readVarint(&input, true);
        int bonesCount = readVarint(&input, true);
        skeletonData.ikConstraints[i].bones.resize(bonesCount, 0);
        for (int ii = 0; ii < bonesCount; ii++)
            skeletonData.ikConstraints[i].bones[ii] = readVarint(&input, true);
        skeletonData.ikConstraints[i].target = readVarint(&input, true);
        int flags = readByte(&input);
        skeletonData.ikConstraints[i].skinRequired = (flags & 1) != 0; 
        skeletonData.ikConstraints[i].bendDirection = (flags & 2) != 0 ? -1 : 1;
        skeletonData.ikConstraints[i].compress = (flags & 4) != 0;
        skeletonData.ikConstraints[i].stretch = (flags & 8) != 0;
        skeletonData.ikConstraints[i].uniform = (flags & 16) != 0;
        if ((flags & 32) != 0) skeletonData.ikConstraints[i].mix = (flags & 64) != 0 ? readFloat(&input) : 1;
        if ((flags & 128) != 0) skeletonData.ikConstraints[i].softness = readFloat(&input);
    }

    /* Transform constraints */
    int transformConstraintsCount = readVarint(&input, true);
    skeletonData.transformConstraints.resize(transformConstraintsCount);
    for (int i = 0; i < transformConstraintsCount; i++) {
        skeletonData.transformConstraints[i].name = readString(&input);
        skeletonData.transformConstraints[i].order = readVarint(&input, true);
        int bonesCount = readVarint(&input, true);
        skeletonData.transformConstraints[i].bones.resize(bonesCount, 0);
        for (int ii = 0; ii < bonesCount; ii++)
            skeletonData.transformConstraints[i].bones[ii] = readVarint(&input, true);
        skeletonData.transformConstraints[i].target = readVarint(&input, true);
        int flags = readByte(&input);
        skeletonData.transformConstraints[i].skinRequired = (flags & 1) != 0;
        skeletonData.transformConstraints[i].local = (flags & 2) != 0; 
        skeletonData.transformConstraints[i].relative = (flags & 4) != 0;
        if ((flags & 8) != 0) skeletonData.transformConstraints[i].offsetRotation = readFloat(&input);
        if ((flags & 16) != 0) skeletonData.transformConstraints[i].offsetX = readFloat(&input);
        if ((flags & 32) != 0) skeletonData.transformConstraints[i].offsetY = readFloat(&input);
        if ((flags & 64) != 0) skeletonData.transformConstraints[i].offsetScaleX = readFloat(&input);
        if ((flags & 128) != 0) skeletonData.transformConstraints[i].offsetScaleY = readFloat(&input);
        flags = readByte(&input); 
        if ((flags & 1) != 0) skeletonData.transformConstraints[i].offsetShearY = readFloat(&input);
        if ((flags & 2) != 0) skeletonData.transformConstraints[i].mixRotate = readFloat(&input);
        if ((flags & 4) != 0) skeletonData.transformConstraints[i].mixX = readFloat(&input);
        if ((flags & 8) != 0) skeletonData.transformConstraints[i].mixY = readFloat(&input);
        if ((flags & 16) != 0) skeletonData.transformConstraints[i].mixScaleX = readFloat(&input);
        if ((flags & 32) != 0) skeletonData.transformConstraints[i].mixScaleY = readFloat(&input);
        if ((flags & 64) != 0) skeletonData.transformConstraints[i].mixShearY = readFloat(&input);
    }

    /* Path constraints */
    int pathConstraintsCount = readVarint(&input, true);
    skeletonData.pathConstraints.resize(pathConstraintsCount);
    for (int i = 0; i < pathConstraintsCount; i++) {
        skeletonData.pathConstraints[i].name = readString(&input);
        skeletonData.pathConstraints[i].order = readVarint(&input, true);
        skeletonData.pathConstraints[i].skinRequired = readBoolean(&input);
        int bonesCount = readVarint(&input, true);
        skeletonData.pathConstraints[i].bones.resize(bonesCount, 0);
        for (int ii = 0; ii < bonesCount; ii++)
            skeletonData.pathConstraints[i].bones[ii] = readVarint(&input, true);
        skeletonData.pathConstraints[i].target = readVarint(&input, true);
        int flags = readByte(&input);
        skeletonData.pathConstraints[i].positionMode = (PositionMode) (flags & 1);
        skeletonData.pathConstraints[i].spacingMode = (SpacingMode) ((flags >> 1) & 3);
        skeletonData.pathConstraints[i].rotateMode = (RotateMode) ((flags >> 3) & 3);
        if ((flags & 128) != 0) skeletonData.pathConstraints[i].offsetRotation = readFloat(&input);
        skeletonData.pathConstraints[i].position = readFloat(&input);
        skeletonData.pathConstraints[i].spacing = readFloat(&input);
        skeletonData.pathConstraints[i].mixRotate = readFloat(&input);
        skeletonData.pathConstraints[i].mixX = readFloat(&input);
        skeletonData.pathConstraints[i].mixY = readFloat(&input);
    }

    /* Physics constraints */
    int physicsConstraintsCount = readVarint(&input, true);
    skeletonData.physicsConstraints.resize(physicsConstraintsCount);
    for (int i = 0; i < physicsConstraintsCount; i++) {
        skeletonData.physicsConstraints[i].name = readString(&input);
        skeletonData.physicsConstraints[i].order = readVarint(&input, true);
        skeletonData.physicsConstraints[i].bone = readVarint(&input, true);
        int flags = readByte(&input);
        skeletonData.physicsConstraints[i].skinRequired = (flags & 1) != 0;
        if ((flags & 2) != 0) skeletonData.physicsConstraints[i].x = readFloat(&input);
        if ((flags & 4) != 0) skeletonData.physicsConstraints[i].y = readFloat(&input);
        if ((flags & 8) != 0) skeletonData.physicsConstraints[i].rotate = readFloat(&input);
        if ((flags & 16) != 0) skeletonData.physicsConstraints[i].scaleX = readFloat(&input);
        if ((flags & 32) != 0) skeletonData.physicsConstraints[i].shearX = readFloat(&input);
        skeletonData.physicsConstraints[i].limit = (flags & 64) != 0 ? readFloat(&input) : 5000;
        skeletonData.physicsConstraints[i].step = 1.0f / readByte(&input);
        skeletonData.physicsConstraints[i].inertia = readFloat(&input);
        skeletonData.physicsConstraints[i].strength = readFloat(&input);
        skeletonData.physicsConstraints[i].damping = readFloat(&input);
        skeletonData.physicsConstraints[i].massInverse = (flags & 128) != 0 ? readFloat(&input) : 1;
        skeletonData.physicsConstraints[i].wind = readFloat(&input);
        skeletonData.physicsConstraints[i].gravity = readFloat(&input);
        flags = readByte(&input);
        if ((flags & 1) != 0) skeletonData.physicsConstraints[i].inertiaGlobal = true;
        if ((flags & 2) != 0) skeletonData.physicsConstraints[i].strengthGlobal = true;
        if ((flags & 4) != 0) skeletonData.physicsConstraints[i].dampingGlobal = true;
        if ((flags & 8) != 0) skeletonData.physicsConstraints[i].massGlobal = true;
        if ((flags & 16) != 0) skeletonData.physicsConstraints[i].windGlobal = true;
        if ((flags & 32) != 0) skeletonData.physicsConstraints[i].gravityGlobal = true;
        if ((flags & 64) != 0) skeletonData.physicsConstraints[i].mixGlobal = true;
        skeletonData.physicsConstraints[i].mix = (flags & 128) != 0 ? readFloat(&input) : 1;
    }

    /* Default skin */

    return skeletonData;
}