#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "skel2json.h"

// 版本枚举
enum class SpineVersion {
    V3_7 = 0,
    V3_8,
    V4_0,
    V4_1,
    V4_2,
    Invalid
};

// 版本检测函数
SpineVersion detectSpineVersionFromSkel(const std::string& skelPath) {
    const std::vector<std::string> validSpineVersions = {"3.7", "3.8", "4.0", "4.1", "4.2"};
    
    try {
        std::ifstream ifs(skelPath, std::ios::binary);
        if (!ifs) return SpineVersion::Invalid;
        
        const size_t headerSize = 64;
        char buffer[headerSize] = {0};
        ifs.read(buffer, headerSize);
        std::string data(buffer, ifs.gcount());
        
        for (int i = 0; i < validSpineVersions.size(); ++i) {
            if (data.find(validSpineVersions[i]) != std::string::npos) {
                return static_cast<SpineVersion>(i);
            }
        }
    }
    catch (...) {
        // File reading error
        std::cerr << "Error: Failed to read SKEL file: " << skelPath << "\n";
    }
    
    return SpineVersion::Invalid;
}

// 根据版本调用对应的转换函数
bool convertSkelToJson(const std::string& skelPath, const std::string& jsonPath, SpineVersion version) {
    switch (version) {
        case SpineVersion::V3_7:
            return spine37::skel2json37(skelPath, jsonPath);
        case SpineVersion::V3_8:
            return spine38::skel2json38(skelPath, jsonPath);
        case SpineVersion::V4_0:
            return spine40::skel2json40(skelPath, jsonPath);
        case SpineVersion::V4_1:
            return spine41::skel2json41(skelPath, jsonPath);
        case SpineVersion::V4_2:
            return spine42::skel2json42(skelPath, jsonPath);
        default:
            return false;
    }
}

// 获取版本字符串
std::string getVersionString(SpineVersion version) {
    switch (version) {
        case SpineVersion::V3_7: return "3.7";
        case SpineVersion::V3_8: return "3.8";
        case SpineVersion::V4_0: return "4.0";
        case SpineVersion::V4_1: return "4.1";
        case SpineVersion::V4_2: return "4.2";
        default: return "Unknown";
    }
}

// 显示使用帮助
void showUsage(const char* programName) {
    std::cout << "Spine Skeleton Data Converter\n";
    std::cout << "Usage: " << programName << " <input_skel_path> <output_json_path>\n";
    std::cout << "\n";
    std::cout << "This tool automatically detects Spine version (3.7, 3.8, 4.0, 4.1, 4.2)\n";
    std::cout << "and converts .skel files to .json format.\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "  " << programName << " skeleton.skel output.json\n";
    std::cout << "  " << programName << " C:\\spine\\character.skel C:\\output\\character.json\n";
}

int main(int argc, char* argv[]) {
    // 检查命令行参数
    if (argc != 3) {
        showUsage(argv[0]);
        return 1;
    }

    std::string skelPath = argv[1];
    std::string jsonPath = argv[2];

    // 检查输入文件是否存在
    std::ifstream testFile(skelPath);
    if (!testFile) {
        std::cerr << "Error: Input file does not exist: " << skelPath << std::endl;
        return 1;
    }
    testFile.close();

    // 检测Spine版本
    std::cout << "Detecting Spine version..." << std::endl;
    SpineVersion version = detectSpineVersionFromSkel(skelPath);
    
    if (version == SpineVersion::Invalid) {
        std::cerr << "Error: Unable to detect Spine version or unsupported version." << std::endl;
        std::cerr << "Supported versions: 3.7, 3.8, 4.0, 4.1, 4.2" << std::endl;
        return 1;
    }

    std::cout << "Detected Spine version: " << getVersionString(version) << std::endl;

    // 执行转换
    std::cout << "Converting " << skelPath << " to " << jsonPath << "..." << std::endl;
    
    bool success = convertSkelToJson(skelPath, jsonPath, version);
    
    if (success) {
        std::cout << "Conversion completed successfully!" << std::endl;
        return 0;
    } else {
        std::cerr << "Error: Conversion failed!" << std::endl;
        return 1;
    }
}
