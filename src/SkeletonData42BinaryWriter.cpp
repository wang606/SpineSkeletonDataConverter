#include "SkeletonData42.h"
using namespace spine42;

void writeByte(Binary& binary, unsigned char value) {
    binary.push_back(value);
}

void writeSByte(Binary& binary, signed char value) {
    writeByte(binary, (unsigned char)value);
}

void writeBoolean(Binary& binary, bool value) {
    writeByte(binary, value ? 1 : 0);
}

void writeInt(Binary& binary, int value) {
    writeByte(binary, (unsigned char)(value >> 24));
    writeByte(binary, (unsigned char)(value >> 16));
    writeByte(binary, (unsigned char)(value >> 8));
    writeByte(binary, (unsigned char)value);
}

void writeColor(Binary& binary, const Color& color) {
    writeByte(binary, color.r);
    writeByte(binary, color.g);
    writeByte(binary, color.b);
    writeByte(binary, color.a);
}

void writeVarint(Binary& binary, int value, bool optimizePositive) {
    if (optimizePositive && value >= 0) {
        while (value > 0x7F) {
            writeByte(binary, (unsigned char)((value & 0x7F) | 0x80));
            value >>= 7;
        }
        writeByte(binary, (unsigned char)value);
    } else {
        unsigned int zigzag = (value << 1) ^ (value >> 31);
        while (zigzag > 0x7F) {
            writeByte(binary, (unsigned char)((zigzag & 0x7F) | 0x80));
            zigzag >>= 7;
        }
        writeByte(binary, (unsigned char)zigzag);
    }
}

void writeFloat(Binary& binary, float value) {
    union {
        float floatValue;
        int intValue;
    } floatToInt;
    floatToInt.floatValue = value;
    writeInt(binary, floatToInt.intValue);
}

void writeString(Binary& binary, const OptStr& string) {
    if (!string) {
        writeByte(binary, 0);
        return;
    }
    writeVarint(binary, string->length() + 1, true);
    binary.insert(binary.end(), string->begin(), string->end());
}

void writeStringRef(Binary& binary, const OptStr& string, const SkeletonData& skeletonData) {
    int index = 0;
    if (string) {
        for (size_t i = 0; i < skeletonData.strings.size(); i++) {
            if (skeletonData.strings[i] == *string) {
                index = i + 1;
                break;
            }
        }
    }
    writeVarint(binary, index, true);
}

void writeSequence(Binary& binary, const Sequence& sequence) {
    writeVarint(binary, sequence.count, true);
    writeVarint(binary, sequence.start, true);
    writeVarint(binary, sequence.digits, true);
    writeVarint(binary, sequence.setupIndex, true);
}

void writeFloatArray(Binary& binary, const std::vector<float>& array) {
    for (float value : array) {
        writeFloat(binary, value);
    }
}

void writeShortArray(Binary& binary, const std::vector<unsigned short>& array) {
    for (unsigned short value : array) {
        writeVarint(binary, value, true);
    }
}

void writeVertices(Binary& binary, const std::vector<float>& vertices, const std::vector<int>& bones, bool weighted) {
    if (!weighted) {
        int verticesLength = vertices.size(); 
        int vertexCount = verticesLength >> 1;
        writeVarint(binary, vertexCount, true);
        writeFloatArray(binary, vertices);
    } else {
        int vertexCount = 0; 
        int bonesIdx = 0; 
        int verticesIdx = 0; 
        while (bonesIdx < bones.size()) {
            int boneCount = bones[bonesIdx];
            vertexCount++; 
            bonesIdx += boneCount;
        }
        bonesIdx = 0; 
        for (int i = 0; i < vertexCount; i++) {
            int boneCount = bones[bonesIdx]; 
            writeVarint(binary, boneCount, true);
            bonesIdx++; 
            for (int ii = 0; ii < boneCount; ii++) {
                writeVarint(binary, bones[bonesIdx++], true);
                writeFloat(binary, vertices[verticesIdx++]);
                writeFloat(binary, vertices[verticesIdx++]);
                writeFloat(binary, vertices[verticesIdx++]);
            }
        }
    }
}

Binary writeBinaryData(const SkeletonData& skeletonData) {
    Binary binary;
    
    // TODO

    return binary;
}