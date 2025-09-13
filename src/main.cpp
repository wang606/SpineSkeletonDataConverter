#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <regex>
#include "common.h"

// Include all version headers
#include "SkeletonData37.h"
#include "SkeletonData38.h"
#include "SkeletonData40.h"
#include "SkeletonData41.h"
#include "SkeletonData42.h"

enum class SpineVersion {
    Version37 = 0,
    Version38 = 1,
    Version40 = 2,
    Version41 = 3,
    Version42 = 4,
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
    bool help = false;
};

SpineVersion detectSpineVersion(const std::string& filePath) {
    try {
        std::ifstream ifs(filePath, std::ios::binary);
        if (!ifs) return SpineVersion::Invalid;
        
        const size_t headerSize = 64;
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
            
            if (majorMinor == "3.7") {
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
        case SpineVersion::Version37: return "3.7";
        case SpineVersion::Version38: return "3.8";
        case SpineVersion::Version40: return "4.0";
        case SpineVersion::Version41: return "4.1";
        case SpineVersion::Version42: return "4.2";
        default: return "Unknown";
    }
}

bool convertFile(const std::string& inputFile, const std::string& outputFile, 
                FileFormat inputFormat, FileFormat outputFormat, SpineVersion version) {
    
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
        
        // Convert based on version
        switch (version) {
            case SpineVersion::Version37: {
                spine37::SkeletonData skelData;
                if (inputFormat == FileFormat::Skel) {
                    skelData = spine37::readBinaryData(binaryData);
                } else {
                    skelData = spine37::readJsonData(jsonData);
                }
                
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
                spine38::SkeletonData skelData;
                if (inputFormat == FileFormat::Skel) {
                    skelData = spine38::readBinaryData(binaryData);
                } else {
                    skelData = spine38::readJsonData(jsonData);
                }
                
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
                spine40::SkeletonData skelData;
                if (inputFormat == FileFormat::Skel) {
                    skelData = spine40::readBinaryData(binaryData);
                } else {
                    skelData = spine40::readJsonData(jsonData);
                }
                
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
                spine41::SkeletonData skelData;
                if (inputFormat == FileFormat::Skel) {
                    skelData = spine41::readBinaryData(binaryData);
                } else {
                    skelData = spine41::readJsonData(jsonData);
                }
                
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
                spine42::SkeletonData skelData;
                if (inputFormat == FileFormat::Skel) {
                    skelData = spine42::readBinaryData(binaryData);
                } else {
                    skelData = spine42::readJsonData(jsonData);
                }
                
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
                std::cerr << "Error: Unsupported Spine version\n";
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
    std::cout << "Options:\n";
    std::cout << "  --in-json    Input file is in JSON format\n";
    std::cout << "  --in-skel    Input file is in SKEL (binary) format\n";
    std::cout << "  --out-json   Output file should be in JSON format\n";
    std::cout << "  --out-skel   Output file should be in SKEL (binary) format\n";
    std::cout << "  --help       Show this help message\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << programName << " input.skel output.json --in-skel --out-json\n";
    std::cout << "  " << programName << " input.json output.skel --in-json --out-skel\n\n";
    std::cout << "Supported Spine versions: 3.7, 3.8, 4.0, 4.1, 4.2\n";
    std::cout << "Version detection is automatic based on file content.\n";
}

ConversionOptions parseArguments(int argc, char* argv[]) {
    ConversionOptions options;
    
    if (argc < 3) {
        options.help = true;
        return options;
    }
    
    options.inputFile = argv[1];
    options.outputFile = argv[2];
    
    for (int i = 3; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--in-json") {
            options.inputFormat = FileFormat::Json;
        } else if (arg == "--in-skel") {
            options.inputFormat = FileFormat::Skel;
        } else if (arg == "--out-json") {
            options.outputFormat = FileFormat::Json;
        } else if (arg == "--out-skel") {
            options.outputFormat = FileFormat::Skel;
        } else if (arg == "--help") {
            options.help = true;
        } else {
            std::cerr << "Warning: Unknown option: " << arg << "\n";
        }
    }
    
    // Auto-detect formats if not specified
    if (options.inputFormat == FileFormat::Unknown) {
        std::string inputExt = std::filesystem::path(options.inputFile).extension().string();
        if (inputExt == ".json") {
            options.inputFormat = FileFormat::Json;
        } else if (inputExt == ".skel") {
            options.inputFormat = FileFormat::Skel;
        }
    }
    
    if (options.outputFormat == FileFormat::Unknown) {
        std::string outputExt = std::filesystem::path(options.outputFile).extension().string();
        if (outputExt == ".json") {
            options.outputFormat = FileFormat::Json;
        } else if (outputExt == ".skel") {
            options.outputFormat = FileFormat::Skel;
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
    
    // Validate formats are specified
    if (options.inputFormat == FileFormat::Unknown) {
        std::cerr << "Error: Could not determine input file format. Please specify --in-json or --in-skel\n";
        return 1;
    }
    
    if (options.outputFormat == FileFormat::Unknown) {
        std::cerr << "Error: Could not determine output file format. Please specify --out-json or --out-skel\n";
        return 1;
    }
    
    // Detect Spine version
    SpineVersion version = detectSpineVersion(options.inputFile);
    
    if (version == SpineVersion::Invalid) {
        std::cerr << "Error: Could not detect Spine version from input file\n";
        return 1;
    }
    
    std::cout << "Detected Spine version: " << getVersionString(version) << "\n";
    std::cout << "Converting from " << (options.inputFormat == FileFormat::Json ? "JSON" : "SKEL") 
              << " to " << (options.outputFormat == FileFormat::Json ? "JSON" : "SKEL") << "...\n";
    
    if (convertFile(options.inputFile, options.outputFile, options.inputFormat, options.outputFormat, version)) {
        std::cout << "Conversion completed successfully!\n";
        std::cout << "Output file: " << options.outputFile << "\n";
        return 0;
    } else {
        std::cerr << "Conversion failed!\n";
        return 1;
    }
}