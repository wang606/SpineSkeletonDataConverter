#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <windows.h>

namespace fs = std::filesystem;

// 声明外部DLL函数
extern "C" {
    __declspec(dllimport) bool testLoadSkeletonJson37(const char* jsonPath, const char* atlasPath);
    __declspec(dllimport) bool testLoadSkeletonJson38(const char* jsonPath, const char* atlasPath);
    __declspec(dllimport) bool testLoadSkeletonJson40(const char* jsonPath, const char* atlasPath);
    __declspec(dllimport) bool testLoadSkeletonJson41(const char* jsonPath, const char* atlasPath);
    __declspec(dllimport) bool testLoadSkeletonJson42(const char* jsonPath, const char* atlasPath);
}

// Spine版本信息
struct SpineVersionInfo {
    std::string version;
    std::string dirName;
    bool (*testFunc)(const char*, const char*);
};

const std::vector<SpineVersionInfo> spineVersions = {
    {"3.7", "37", testLoadSkeletonJson37},
    {"3.8", "38", testLoadSkeletonJson38},
    {"4.0", "40", testLoadSkeletonJson40},
    {"4.1", "41", testLoadSkeletonJson41},
    {"4.2", "42", testLoadSkeletonJson42}
};

// 检测JSON文件的Spine版本
std::string detectSpineVersionFromJson(const std::string& jsonPath) {
    try {
        std::ifstream ifs(jsonPath);
        if (!ifs) return "";
        
        std::string content((std::istreambuf_iterator<char>(ifs)),
                           std::istreambuf_iterator<char>());
        
        // 查找版本信息，通常在skeleton字段中
        size_t skelPos = content.find("\"skeleton\"");
        if (skelPos != std::string::npos) {
            size_t versionPos = content.find("\"spine\"", skelPos);
            if (versionPos != std::string::npos) {
                size_t colonPos = content.find(":", versionPos);
                if (colonPos != std::string::npos) {
                    size_t quoteStart = content.find("\"", colonPos);
                    if (quoteStart != std::string::npos) {
                        size_t quoteEnd = content.find("\"", quoteStart + 1);
                        if (quoteEnd != std::string::npos) {
                            std::string version = content.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                            // 提取主要版本号
                            if (version.find("3.7") == 0) return "3.7";
                            if (version.find("3.8") == 0) return "3.8";
                            if (version.find("4.0") == 0) return "4.0";
                            if (version.find("4.1") == 0) return "4.1";
                            if (version.find("4.2") == 0) return "4.2";
                        }
                    }
                }
            }
        }
    }
    catch (...) {
        // 忽略读取错误
    }
    
    return "";
}

// 验证JSON文件的基本结构
bool validateJsonStructure(const std::string& jsonPath) {
    try {
        std::ifstream file(jsonPath);
        if (!file) {
            std::cerr << "Error: Cannot open JSON file: " << jsonPath << std::endl;
            return false;
        }
        
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        
        // 基本检查：包含关键字段
        bool hasRequiredFields = content.find("skeleton") != std::string::npos && 
                                content.find("bones") != std::string::npos && 
                                content.find("slots") != std::string::npos;
        
        if (!hasRequiredFields) {
            std::cerr << "Error: JSON file missing required fields (skeleton, bones, slots)" << std::endl;
        }
        
        return hasRequiredFields;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: Exception while validating JSON structure: " << e.what() << std::endl;
        return false;
    }
    catch (...) {
        std::cerr << "Error: Unknown exception while validating JSON structure" << std::endl;
        return false;
    }
}

// 使用Spine运行时验证JSON文件
bool validateWithSpineRuntime(const std::string& jsonPath, const std::string& atlasPath, const std::string& version) {
    // 找到对应版本的测试函数
    auto it = std::find_if(spineVersions.begin(), spineVersions.end(),
        [&version](const SpineVersionInfo& v) { return v.version == version; });
    
    if (it == spineVersions.end()) {
        std::cerr << "Error: Unsupported Spine version: " << version << std::endl;
        return false;
    }
    
    // 调用验证函数
    try {
        bool result = it->testFunc(jsonPath.c_str(), atlasPath.c_str());
        if (!result) {
            std::cerr << "Error: Spine runtime validation failed" << std::endl;
        }
        return result;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: Exception during Spine runtime validation: " << e.what() << std::endl;
        return false;
    }
    catch (...) {
        std::cerr << "Error: Unknown exception during Spine runtime validation" << std::endl;
        return false;
    }
}

void printUsage(const char* programName) {
    std::cout << "Spine JSON Validator" << std::endl;
    std::cout << "Usage: " << programName << " <json_file> <atlas_file>" << std::endl;
    std::cout << std::endl;
    std::cout << "Arguments:" << std::endl;
    std::cout << "  json_file   Path to the Spine JSON file" << std::endl;
    std::cout << "  atlas_file  Path to the corresponding Atlas file" << std::endl;
    std::cout << std::endl;
    std::cout << "This tool validates Spine JSON files using the appropriate Spine runtime library." << std::endl;
    std::cout << "The Spine version is automatically detected from the JSON file." << std::endl;
}

int main(int argc, char* argv[]) {
    // 设置控制台为UTF-8编码
    SetConsoleOutputCP(CP_UTF8);
    
    // 检查参数
    if (argc != 3) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string jsonPath = argv[1];
    std::string atlasPath = argv[2];
    
    // 检查文件是否存在
    if (!fs::exists(jsonPath)) {
        std::cerr << "Error: JSON file not found: " << jsonPath << std::endl;
        return 1;
    }
    
    if (!fs::exists(atlasPath)) {
        std::cerr << "Error: Atlas file not found: " << atlasPath << std::endl;
        return 1;
    }
    
    std::cout << "Validating Spine files:" << std::endl;
    std::cout << "  JSON:  " << jsonPath << std::endl;
    std::cout << "  Atlas: " << atlasPath << std::endl;
    std::cout << std::endl;
    
    try {
        // 1. 检测Spine版本
        std::string version = detectSpineVersionFromJson(jsonPath);
        if (version.empty()) {
            std::cerr << "Error: Failed to detect Spine version from JSON file" << std::endl;
            return 1;
        }
        
        std::cout << "Detected Spine version: " << version << std::endl;
        
        // 2. 验证JSON结构
        std::cout << "Validating JSON structure... ";
        if (!validateJsonStructure(jsonPath)) {
            std::cout << "FAILED" << std::endl;
            return 1;
        }
        std::cout << "OK" << std::endl;
        
        // 3. 使用Spine运行时验证
        std::cout << "Validating with Spine runtime... ";
        if (!validateWithSpineRuntime(jsonPath, atlasPath, version)) {
            std::cout << "FAILED" << std::endl;
            return 1;
        }
        std::cout << "OK" << std::endl;
        
        std::cout << std::endl << "✓ Validation successful!" << std::endl;
        return 0;
        
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Error: Unknown error occurred" << std::endl;
        return 1;
    }
}
