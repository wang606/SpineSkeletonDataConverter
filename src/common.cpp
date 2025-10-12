#include "common.h"

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
            oss << "\"" << it.key() << "\":";
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
