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

int readVertices(DataInput* input, std::vector<float>& vertices, bool weighted) {
    int vertexCount = readVarint(input, true); 
    if (!weighted) {
        readFloatArray(input, vertexCount << 1, vertices);
    } else {
        for (int i = 0; i < vertexCount; i++) {
            int boneCount = readVarint(input, true); 
            vertices.push_back(boneCount);
            for (int ii = 0; ii < boneCount; ii++) {
                vertices.push_back(readVarint(input, true));
                vertices.push_back(readFloat(input));
                vertices.push_back(readFloat(input));
                vertices.push_back(readFloat(input));
            }
        }
    }
    return vertexCount; 
}

SkeletonData readBinaryData(const Binary& binary) {
    SkeletonData skeletonData;
    DataInput input; 
    input.cursor = binary.data(); 
    input.end = binary.data() + binary.size();

    uint64_t lowHash = (uint64_t) readInt(&input); 
    uint64_t highHash = (uint64_t) readInt(&input);
    skeletonData.hash = highHash << 32 | lowHash;
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
    for (int i = 0; i < numBones; i++) {
        BoneData boneData; 
        boneData.name = readString(&input); 
        if (i != 0) boneData.parent = skeletonData.bones[readVarint(&input, true)].name;
        boneData.rotation = readFloat(&input);
        boneData.x = readFloat(&input);
        boneData.y = readFloat(&input);
        boneData.scaleX = readFloat(&input);
        boneData.scaleY = readFloat(&input);
        boneData.shearX = readFloat(&input);
        boneData.shearY = readFloat(&input);
        boneData.length = readFloat(&input);
        boneData.inherit = static_cast<Inherit>(readVarint(&input, true));
        boneData.skinRequired = readBoolean(&input);
        if (skeletonData.nonessential) {
            boneData.color = readColor(&input);
            boneData.icon = readString(&input).value();
            boneData.visible = readBoolean(&input);
        }
        skeletonData.bones.push_back(boneData);
    }

    /* Slots */
    int slotCount = readVarint(&input, true);
    for (int i = 0; i < slotCount; i++) {
        SlotData slotData; 
        slotData.name = readString(&input);
        slotData.bone = skeletonData.bones[readVarint(&input, true)].name;
        slotData.color = readColor(&input);
        unsigned char a = readByte(&input);
        unsigned char r = readByte(&input);
        unsigned char g = readByte(&input);
        unsigned char b = readByte(&input);
        if (!(r == 0xff && g == 0xff && b == 0xff && a == 0xff)) {
            slotData.darkColor = Color{ r, g, b, a }; 
        }
        slotData.attachmentName = readStringRef(&input, &skeletonData);
        slotData.blendMode = static_cast<BlendMode>(readVarint(&input, true));
        if (skeletonData.nonessential) slotData.visible = readBoolean(&input);
        skeletonData.slots.push_back(slotData);
    }

    // TODO

    return skeletonData;
}