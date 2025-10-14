#include "SkeletonData.h"

// json reader

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

// json writer

std::string colorToString(const Color& color, bool hasAlpha) {
    char buffer[9];
    snprintf(buffer, sizeof(buffer), "%02x%02x%02x", 
             static_cast<int>(color.r), 
             static_cast<int>(color.g), 
             static_cast<int>(color.b));
    if (hasAlpha) {
        snprintf(buffer + 6, 3, "%02x", static_cast<int>(color.a));
    }
    return std::string(buffer);
}

// binary reader

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

Color readColor(DataInput* input, bool hasAlpha) {
    Color color; 
    color.r = readByte(input); 
    color.g = readByte(input); 
    color.b = readByte(input); 
    color.a = hasAlpha ? readByte(input) : 255; 
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

// binary writer

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

void writeColor(Binary& binary, const Color& color, bool hasAlpha) {
    writeByte(binary, color.r);
    writeByte(binary, color.g);
    writeByte(binary, color.b);
    if (hasAlpha) writeByte(binary, color.a);
}

void writeVarint(Binary& binary, int value, bool optimizePositive) {
    unsigned int unsignedValue;
    if (!optimizePositive)
        unsignedValue = (value << 1) ^ (value >> 31);
    else
        unsignedValue = value;
    while (unsignedValue > 0x7F) {
        writeByte(binary, (unsigned char)((unsignedValue & 0x7F) | 0x80));
        unsignedValue >>= 7;
    }
    writeByte(binary, (unsigned char)(unsignedValue & 0x7F));
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
        throw std::runtime_error("String reference not found: " + *string);
    }
    writeVarint(binary, index, true);
}

// json writer

std::string formatNumber(double v) {
    if (std::floor(v) == v) {
        return std::to_string((long long)v);
    }
    std::ostringstream oss;
    if (std::fabs(v) > 1e6 || (std::fabs(v) > 0 && std::fabs(v) < 0.001)) {
        double absV = std::fabs(v);
        if (absV < 0.01) {
            oss << std::uppercase << std::scientific << std::setprecision(1) << v;
        } else {
            oss << std::uppercase << std::scientific << std::setprecision(2) << v;
        }
        return oss.str();
    }
    if (std::fabs(v) >= 1.0) {
        oss << std::fixed << std::setprecision(2) << v;
    } else {
        oss << std::fixed << std::setprecision(5) << v;
    }
    return oss.str();
}

void dumpJsonToOss(const Json& j, std::ostringstream& oss) {
    if (j.is_array()) {
        oss << "["; 
        for (size_t i = 0; i < j.size(); i++) {
            dumpJsonToOss(j[i], oss);
            if (i + 1 < j.size()) oss << ",";
        }
        oss << "]";
    } else if (j.is_object()) {
        oss << "{";
        bool first = true;
        for (auto it = j.begin(); it != j.end(); ++it) {
            if (!first) oss << ",";
            oss << Json(it.key()).dump() << ":";
            dumpJsonToOss(it.value(), oss);
            first = false;
        }
        oss << "}";
    } else if (j.is_number_float()) {
        oss << formatNumber(j.get<double>());
    } else {
        oss << j.dump(); 
    }
}

std::string dumpJson(const Json& j) {
    std::ostringstream oss;
    dumpJsonToOss(j, oss);
    return oss.str();
}

static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string encodeBase64(const std::vector<uint8_t>& data) {
    std::string ret;
    int val = 0, valb = -6;
    for (uint8_t c : data) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            ret.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) ret.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (ret.size() % 4) ret.push_back('=');
    return ret;
}

std::vector<uint8_t> decodeBase64(const std::string& s) {
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[base64_chars[i]] = i;
    std::vector<uint8_t> out;
    int val = 0, valb = -8;
    for (uint8_t c : s) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(uint8_t((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

// uint64_t -> Base64 (省略多余的 '=')
std::string uint64ToBase64(uint64_t value) {
    std::vector<uint8_t> bytes(8);
    for (int i = 0; i < 8; i++) {
        bytes[7 - i] = (value >> (i * 8)) & 0xFF; // Big endian
    }
    std::string b64 = encodeBase64(bytes);
    // 去掉末尾的 '='
    while (!b64.empty() && b64.back() == '=') b64.pop_back();
    return b64;
}

// Base64 -> uint64_t
uint64_t base64ToUint64(const std::string& str) {
    std::string padded = str;
    while (padded.size() % 4 != 0) padded.push_back('='); // 补齐长度
    std::vector<uint8_t> bytes = decodeBase64(padded);
    if (bytes.size() > 8) {
        bytes = std::vector<uint8_t>(bytes.end() - 8, bytes.end()); // 取最后8个字节
    }
    uint64_t value = 0;
    for (size_t i = 0; i < bytes.size(); i++) {
        value = (value << 8) | bytes[i];
    }
    return value;
}
