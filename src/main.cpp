#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <regex>

#include "SkeletonData.h"

enum class SpineVersion {
    Version35 = 0,
    Version36 = 1,
    Version37 = 2,
    Version38 = 3,
    Version40 = 4,
    Version41 = 5,
    Version42 = 6,
    Invalid = -1
};

enum class FileFormat {
    Json,
    Skel,
    Unknown
};

struct ConversionOptions {
    std::string inputFile;
    std::string outputFile;
    FileFormat inputFormat = FileFormat::Unknown;
    FileFormat outputFormat = FileFormat::Unknown;
    SpineVersion outputVersion = SpineVersion::Invalid;
    std::string outputVersionString; // 完整的版本号字符串，如 "4.2.11"
    bool help = false;
};

bool is3xVersion(SpineVersion version) {
    return version == SpineVersion::Version35 || version == SpineVersion::Version36 || version == SpineVersion::Version37 || version == SpineVersion::Version38;
}

bool is4xVersion(SpineVersion version) {
    return version == SpineVersion::Version40 || version == SpineVersion::Version41 || version == SpineVersion::Version42;
}

SpineVersion detectSpineVersion(const std::string& filePath) {
    try {
        std::ifstream ifs(filePath, std::ios::binary);
        if (!ifs) return SpineVersion::Invalid;
        
        const size_t headerSize = 256;
        char buffer[headerSize] = {0};
        ifs.read(buffer, headerSize);
        std::string data(buffer, ifs.gcount());
        
        // Use regex to find version pattern x.x.x
        std::regex versionRegex(R"((\d+)\.(\d+)\.(\d+))");
        std::smatch match;
        
        if (std::regex_search(data, match, versionRegex)) {
            std::string majorVersion = match[1].str();
            std::string minorVersion = match[2].str();
            std::string majorMinor = majorVersion + "." + minorVersion;
            
            if (majorMinor == "3.5") {
                return SpineVersion::Version35;
            } else if (majorMinor == "3.6") {
                return SpineVersion::Version36;
            } else if (majorMinor == "3.7") {
                return SpineVersion::Version37;
            } else if (majorMinor == "3.8") {
                return SpineVersion::Version38;
            } else if (majorMinor == "4.0") {
                return SpineVersion::Version40;
            } else if (majorMinor == "4.1") {
                return SpineVersion::Version41;
            } else if (majorMinor == "4.2") {
                return SpineVersion::Version42;
            }
        }
    }
    catch (...) {
        std::cerr << "Error: Failed to read file: " << filePath << "\n";
    }
    
    return SpineVersion::Invalid;
}

std::string getVersionString(SpineVersion version) {
    switch (version) {
        case SpineVersion::Version35: return "3.5";
        case SpineVersion::Version36: return "3.6";
        case SpineVersion::Version37: return "3.7";
        case SpineVersion::Version38: return "3.8";
        case SpineVersion::Version40: return "4.0";
        case SpineVersion::Version41: return "4.1";
        case SpineVersion::Version42: return "4.2";
        default: return "Unknown";
    }
}

SpineVersion parseVersionString(const std::string& versionStr) {
    // 强制要求完整的三段式版本号 (x.y.z)
    std::regex versionRegex(R"(^(\d+)\.(\d+)\.(\d+)$)");
    std::smatch match;
    
    if (std::regex_match(versionStr, match, versionRegex)) {
        std::string majorVersion = match[1].str();
        std::string minorVersion = match[2].str();
        std::string majorMinor = majorVersion + "." + minorVersion;
        
        if (majorMinor == "3.5") return SpineVersion::Version35;
        else if (majorMinor == "3.6") return SpineVersion::Version36;
        else if (majorMinor == "3.7") return SpineVersion::Version37;
        else if (majorMinor == "3.8") return SpineVersion::Version38;
        else if (majorMinor == "4.0") return SpineVersion::Version40;
        else if (majorMinor == "4.1") return SpineVersion::Version41;
        else if (majorMinor == "4.2") return SpineVersion::Version42;
    }
    
    return SpineVersion::Invalid;
}

bool convertFile(const std::string& inputFile, const std::string& outputFile, 
                FileFormat inputFormat, FileFormat outputFormat, 
                SpineVersion inputVersion, SpineVersion outputVersion, 
                const std::string& outputVersionString) {
    
    try {
        // Read input file
        std::vector<unsigned char> binaryData;
        Json jsonData;
        
        if (inputFormat == FileFormat::Skel) {
            std::ifstream ifs(inputFile, std::ios::binary);
            if (!ifs) {
                std::cerr << "Error: Cannot open input file: " << inputFile << "\n";
                return false;
            }
            binaryData.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        } else if (inputFormat == FileFormat::Json) {
            std::ifstream ifs(inputFile);
            if (!ifs) {
                std::cerr << "Error: Cannot open input file: " << inputFile << "\n";
                return false;
            }
            ifs >> jsonData;
        }
        
        // Read data using input version
        SkeletonData skelData;
        switch (inputVersion) {
            case SpineVersion::Version35: {
                if (inputFormat == FileFormat::Skel) {
                    skelData = spine35::readBinaryData(binaryData);
                } else {
                    skelData = spine35::readJsonData(jsonData);
                }
                break;
            }
            case SpineVersion::Version36: {
                if (inputFormat == FileFormat::Skel) {
                    skelData = spine36::readBinaryData(binaryData);
                } else {
                    skelData = spine36::readJsonData(jsonData);
                }
                break;
            }
            case SpineVersion::Version37: {
                if (inputFormat == FileFormat::Skel) {
                    skelData = spine37::readBinaryData(binaryData);
                } else {
                    skelData = spine37::readJsonData(jsonData);
                }
                break;
            }
            case SpineVersion::Version38: {
                if (inputFormat == FileFormat::Skel) {
                    skelData = spine38::readBinaryData(binaryData);
                } else {
                    skelData = spine38::readJsonData(jsonData);
                }
                break;
            }
            case SpineVersion::Version40: {
                if (inputFormat == FileFormat::Skel) {
                    skelData = spine40::readBinaryData(binaryData);
                } else {
                    skelData = spine40::readJsonData(jsonData);
                }
                break;
            }
            case SpineVersion::Version41: {
                if (inputFormat == FileFormat::Skel) {
                    skelData = spine41::readBinaryData(binaryData);
                } else {
                    skelData = spine41::readJsonData(jsonData);
                }
                break;
            }
            case SpineVersion::Version42: {
                if (inputFormat == FileFormat::Skel) {
                    skelData = spine42::readBinaryData(binaryData);
                } else {
                    skelData = spine42::readJsonData(jsonData);
                }
                break;
            }
            default:
                std::cerr << "Error: Unsupported input Spine version\n";
                return false;
        }
        
        // 如果指定了输出版本字符串，则设置版本号
        if (!outputVersionString.empty()) {
            skelData.version = outputVersionString;
        }
        
        // 跨版本转换处理
        if (inputVersion != outputVersion) {
            bool inputIs3x = is3xVersion(inputVersion);
            bool inputIs4x = is4xVersion(inputVersion);
            bool outputIs3x = is3xVersion(outputVersion);
            bool outputIs4x = is4xVersion(outputVersion);
            
            if (inputIs3x && outputIs4x) {
                std::cout << "Performing 3.x to 4.x conversion...\n";
                convertCurve3xTo4x(skelData);
            } else if (inputIs4x && outputIs3x) {
                std::cout << "Performing 4.x to 3.x conversion...\n";
                convertCurve4xTo3x(skelData);
            }
        }
        
        // Write data using output version
        switch (outputVersion) {
            case SpineVersion::Version35: {
                if (outputFormat == FileFormat::Skel) {
                    auto outputData = spine35::writeBinaryData(skelData);
                    std::ofstream ofs(outputFile, std::ios::binary);
                    if (!ofs) {
                        std::cerr << "Error: Cannot create output file: " << outputFile << "\n";
                        return false;
                    }
                    ofs.write(reinterpret_cast<const char*>(outputData.data()), outputData.size());
                } else {
                    auto outputJson = spine35::writeJsonData(skelData);
                    std::ofstream ofs(outputFile);
                    if (!ofs) {
                        std::cerr << "Error: Cannot create output file: " << outputFile << "\n";
                        return false;
                    }
                    ofs << dumpJson(outputJson);
                }
                break;
            }
            case SpineVersion::Version36: {
                if (outputFormat == FileFormat::Skel) {
                    auto outputData = spine36::writeBinaryData(skelData);
                    std::ofstream ofs(outputFile, std::ios::binary);
                    if (!ofs) {
                        std::cerr << "Error: Cannot create output file: " << outputFile << "\n";
                        return false;
                    }
                    ofs.write(reinterpret_cast<const char*>(outputData.data()), outputData.size());
                } else {
                    auto outputJson = spine36::writeJsonData(skelData);
                    std::ofstream ofs(outputFile);
                    if (!ofs) {
                        std::cerr << "Error: Cannot create output file: " << outputFile << "\n";
                        return false;
                    }
                    ofs << dumpJson(outputJson);
                }
                break;
            }
            case SpineVersion::Version37: {
                if (outputFormat == FileFormat::Skel) {
                    auto outputData = spine37::writeBinaryData(skelData);
                    std::ofstream ofs(outputFile, std::ios::binary);
                    if (!ofs) {
                        std::cerr << "Error: Cannot create output file: " << outputFile << "\n";
                        return false;
                    }
                    ofs.write(reinterpret_cast<const char*>(outputData.data()), outputData.size());
                } else {
                    auto outputJson = spine37::writeJsonData(skelData);
                    std::ofstream ofs(outputFile);
                    if (!ofs) {
                        std::cerr << "Error: Cannot create output file: " << outputFile << "\n";
                        return false;
                    }
                    ofs << dumpJson(outputJson);
                }
                break;
            }
            case SpineVersion::Version38: {
                if (outputFormat == FileFormat::Skel) {
                    auto outputData = spine38::writeBinaryData(skelData);
                    std::ofstream ofs(outputFile, std::ios::binary);
                    if (!ofs) {
                        std::cerr << "Error: Cannot create output file: " << outputFile << "\n";
                        return false;
                    }
                    ofs.write(reinterpret_cast<const char*>(outputData.data()), outputData.size());
                } else {
                    auto outputJson = spine38::writeJsonData(skelData);
                    std::ofstream ofs(outputFile);
                    if (!ofs) {
                        std::cerr << "Error: Cannot create output file: " << outputFile << "\n";
                        return false;
                    }
                    ofs << dumpJson(outputJson);
                }
                break;
            }
            case SpineVersion::Version40: {
                if (outputFormat == FileFormat::Skel) {
                    auto outputData = spine40::writeBinaryData(skelData);
                    std::ofstream ofs(outputFile, std::ios::binary);
                    if (!ofs) {
                        std::cerr << "Error: Cannot create output file: " << outputFile << "\n";
                        return false;
                    }
                    ofs.write(reinterpret_cast<const char*>(outputData.data()), outputData.size());
                } else {
                    auto outputJson = spine40::writeJsonData(skelData);
                    std::ofstream ofs(outputFile);
                    if (!ofs) {
                        std::cerr << "Error: Cannot create output file: " << outputFile << "\n";
                        return false;
                    }
                    ofs << dumpJson(outputJson);
                }
                break;
            }
            case SpineVersion::Version41: {
                if (outputFormat == FileFormat::Skel) {
                    auto outputData = spine41::writeBinaryData(skelData);
                    std::ofstream ofs(outputFile, std::ios::binary);
                    if (!ofs) {
                        std::cerr << "Error: Cannot create output file: " << outputFile << "\n";
                        return false;
                    }
                    ofs.write(reinterpret_cast<const char*>(outputData.data()), outputData.size());
                } else {
                    auto outputJson = spine41::writeJsonData(skelData);
                    std::ofstream ofs(outputFile);
                    if (!ofs) {
                        std::cerr << "Error: Cannot create output file: " << outputFile << "\n";
                        return false;
                    }
                    ofs << dumpJson(outputJson);
                }
                break;
            }
            case SpineVersion::Version42: {
                if (outputFormat == FileFormat::Skel) {
                    auto outputData = spine42::writeBinaryData(skelData);
                    std::ofstream ofs(outputFile, std::ios::binary);
                    if (!ofs) {
                        std::cerr << "Error: Cannot create output file: " << outputFile << "\n";
                        return false;
                    }
                    ofs.write(reinterpret_cast<const char*>(outputData.data()), outputData.size());
                } else {
                    auto outputJson = spine42::writeJsonData(skelData);
                    std::ofstream ofs(outputFile);
                    if (!ofs) {
                        std::cerr << "Error: Cannot create output file: " << outputFile << "\n";
                        return false;
                    }
                    ofs << dumpJson(outputJson);
                }
                break;
            }
            default:
                std::cerr << "Error: Unsupported output Spine version\n";
                return false;
        }
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error during conversion: " << e.what() << "\n";
        return false;
    }
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " <input_file> <output_file> [options]\n\n";
    std::cout << "Supported file formats:\n";
    std::cout << "  .json       Spine JSON format\n";
    std::cout << "  .skel       Spine binary (SKEL) format\n\n";
    std::cout << "Options:\n";
    std::cout << "  -v          Output version (must be complete: x.y.z format)\n";
    std::cout << "  --help      Show this help message\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << programName << " input.skel output.json\n";
    std::cout << "  " << programName << " input.json output.skel\n";
    std::cout << "  " << programName << " input37.json output42.skel -v 4.2.11\n\n";
    std::cout << "Supported Spine versions: 3.5.x, 3.6.x, 3.7.x, 3.8.x, 4.0.x, 4.1.x, 4.2.x\n";
    std::cout << "Note: Version must be specified in complete x.y.z format (e.g., 4.2.11, not 4.2)\n";
    std::cout << "Input version detection is automatic based on file content.\n";
    std::cout << "Output version defaults to input version unless specified with -v.\n";
}

ConversionOptions parseArguments(int argc, char* argv[]) {
    ConversionOptions options;
    
    if (argc < 3) {
        options.help = true;
        return options;
    }
    
    options.inputFile = argv[1];
    options.outputFile = argv[2];
    
    // 自动检测文件格式
    std::string inputExt = std::filesystem::path(options.inputFile).extension().string();
    std::string outputExt = std::filesystem::path(options.outputFile).extension().string();
    
    if (inputExt == ".json") {
        options.inputFormat = FileFormat::Json;
    } else if (inputExt == ".skel") {
        options.inputFormat = FileFormat::Skel;
    } else {
        std::cerr << "Error: Unsupported input file extension: " << inputExt << "\n";
        std::cerr << "Supported extensions: .json, .skel\n";
        options.help = true;
        return options;
    }
    
    if (outputExt == ".json") {
        options.outputFormat = FileFormat::Json;
    } else if (outputExt == ".skel") {
        options.outputFormat = FileFormat::Skel;
    } else {
        std::cerr << "Error: Unsupported output file extension: " << outputExt << "\n";
        std::cerr << "Supported extensions: .json, .skel\n";
        options.help = true;
        return options;
    }
    
    // 解析其他参数
    for (int i = 3; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-v") {
            if (i + 1 < argc) {
                std::string versionStr = argv[++i];
                options.outputVersionString = versionStr; // 保存完整版本号字符串
                options.outputVersion = parseVersionString(versionStr);
                if (options.outputVersion == SpineVersion::Invalid) {
                    std::cerr << "Error: Invalid output version: " << versionStr << "\n";
                    std::cerr << "Please specify complete version number (e.g., 3.7.94, 4.2.11)\n";
                    std::cerr << "Supported major versions: 3.5.x, 3.6.x, 3.7.x, 3.8.x, 4.0.x, 4.1.x, 4.2.x\n";
                    options.help = true;
                }
            } else {
                std::cerr << "Error: -v requires a version argument\n";
                options.help = true;
            }
        } else if (arg == "--help") {
            options.help = true;
        } else {
            std::cerr << "Warning: Unknown option: " << arg << "\n";
        }
    }
    
    return options;
}

int main(int argc, char* argv[]) {
    ConversionOptions options = parseArguments(argc, argv);
    
    if (options.help) {
        printUsage(argv[0]);
        return 0;
    }
    
    // Validate input file exists
    if (!std::filesystem::exists(options.inputFile)) {
        std::cerr << "Error: Input file does not exist: " << options.inputFile << "\n";
        return 1;
    }
    
    // 检查是否是skeleton文件转换
    if ((options.inputFormat == FileFormat::Json || options.inputFormat == FileFormat::Skel) &&
        (options.outputFormat == FileFormat::Json || options.outputFormat == FileFormat::Skel)) {
        
        // Detect input Spine version
        SpineVersion inputVersion = detectSpineVersion(options.inputFile);
        
        if (inputVersion == SpineVersion::Invalid) {
            std::cerr << "Error: Could not detect Spine version from input file\n";
            return 1;
        }
        
        // Use output version if specified, otherwise use input version
        SpineVersion outputVersion = (options.outputVersion != SpineVersion::Invalid) ? options.outputVersion : inputVersion;
        std::string outputVersionString = options.outputVersionString; // 使用用户指定的完整版本号
        
        std::cout << "Detected input Spine version: " << getVersionString(inputVersion) << "\n";
        if (inputVersion != outputVersion) {
            std::cout << "Converting to output Spine version: " << getVersionString(outputVersion);
            if (!outputVersionString.empty()) {
                std::cout << " (" << outputVersionString << ")";
            }
            std::cout << "\n";
        }
        std::cout << "Converting from " << (options.inputFormat == FileFormat::Json ? "JSON" : "SKEL") 
                  << " to " << (options.outputFormat == FileFormat::Json ? "JSON" : "SKEL") << "...\n";
        
        if (convertFile(options.inputFile, options.outputFile, options.inputFormat, options.outputFormat, inputVersion, outputVersion, outputVersionString)) {
            std::cout << "Conversion completed successfully!\n";
            std::cout << "Output file: " << options.outputFile << "\n";
            return 0;
        } else {
            std::cerr << "Conversion failed!\n";
            return 1;
        }
    } else {
        std::cerr << "Error: Invalid file format combination\n";
        std::cerr << "Supported conversions:\n";
        std::cerr << "  - .json <-> .skel (skeleton data conversion)\n";
        return 1;
    }
}