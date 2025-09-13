#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <iomanip>
#include "SkeletonData42.h"
#include "common.h"

namespace fs = std::filesystem;

bool loadJsonFromFile(const std::string& filePath, Json& json) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return false;
    }
    
    try {
        file >> json;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse JSON from file " << filePath << ": " << e.what() << std::endl;
        return false;
    }
}

bool saveBinaryToFile(const std::string& filePath, const spine42::Binary& binary) {
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to create file: " << filePath << std::endl;
        return false;
    }
    
    try {
        file.write(reinterpret_cast<const char*>(binary.data()), binary.size());
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to write binary data to file " << filePath << ": " << e.what() << std::endl;
        return false;
    }
}

bool loadBinaryFromFile(const std::string& filePath, spine42::Binary& binary) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return false;
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    binary.resize(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(binary.data()), size)) {
        std::cerr << "Failed to read binary data from file: " << filePath << std::endl;
        return false;
    }
    
    return true;
}

bool saveJsonToFile(const std::string& filePath, const Json& json) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to create file: " << filePath << std::endl;
        return false;
    }
    
    try {
        file << dumpJson(json); // Use custom dumpJson function from common.cpp
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to write JSON to file " << filePath << ": " << e.what() << std::endl;
        return false;
    }
}

std::string generateBinaryFileName(const std::string& inputPath) {
    fs::path path(inputPath);
    std::string stem = path.stem().string();
    fs::path directory = path.parent_path();
    
    return (directory / (stem + ".json.skel")).string();
}

std::string generateFinalJsonFileName(const std::string& inputPath) {
    fs::path path(inputPath);
    std::string stem = path.stem().string();
    fs::path directory = path.parent_path();
    
    return (directory / (stem + ".json.skel.json")).string();
}

bool isPlainJsonFile(const std::string& fileName) {
    // Check if file ends with .json and has only one dot (excluding the extension)
    if (!fileName.ends_with(".json")) {
        return false;
    }
    
    // Remove .json suffix
    std::string nameWithoutJson = fileName.substr(0, fileName.length() - 5);
    
    // Check if there are any other dots in the remaining name
    return nameWithoutJson.find('.') == std::string::npos;
}

bool isInExportDirectory(const std::string& filePath) {
    // Convert to filesystem path for easier manipulation
    fs::path path(filePath);
    
    // Convert to string with forward slashes for consistent matching
    std::string pathStr = path.generic_string();
    
    // Check if the path matches the pattern: data/42/*/export/
    // We need to find "data/42/" followed by any directory, then "/export/"
    size_t data42Pos = pathStr.find("data/42/");
    if (data42Pos == std::string::npos) {
        return false;
    }
    
    // Find the next "/export/" after "data/42/"
    size_t searchStart = data42Pos + 8; // Length of "data/42/"
    size_t exportPos = pathStr.find("/export/", searchStart);
    if (exportPos == std::string::npos) {
        return false;
    }
    
    // Make sure there's exactly one directory level between "data/42/" and "/export/"
    std::string middlePart = pathStr.substr(searchStart, exportPos - searchStart);
    // Check that the middle part doesn't contain additional slashes (only one directory level)
    return middlePart.find('/') == std::string::npos && !middlePart.empty();
}

void testJsonToBinaryToJson() {
    const std::string dataDir = "d:/Projects/SpineSkeletonDataConverter/data/42";
    int successCount = 0;
    int totalCount = 0;
    std::vector<std::string> failedFiles;
    
    std::cout << "Starting Spine Skeleton Data JSON->Binary->JSON round-trip test..." << std::endl;
    std::cout << "Using SkeletonData42JsonReader, SkeletonData42BinaryWriter, SkeletonData42BinaryReader, SkeletonData42JsonWriter" << std::endl;
    std::cout << "Scanning directory: " << dataDir << std::endl;
    std::cout << "Looking for plain .json files in data/42/*/export/ directories" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    try {
        for (const auto& entry : fs::recursive_directory_iterator(dataDir)) {
            if (entry.is_regular_file()) {
                std::string filePath = entry.path().string();
                std::string fileName = entry.path().filename().string();
                
                // Check if it's a plain .json file (no additional extensions) AND in export directory
                if (isPlainJsonFile(fileName) && isInExportDirectory(filePath)) {
                    totalCount++;
                    
                    std::cout << "\n[" << totalCount << "] Processing: " << fileName << std::endl;
                    std::cout << "Full path: " << filePath << std::endl;
                    
                    try {
                        // Step 1: Load JSON from file
                        Json inputJson;
                        if (!loadJsonFromFile(filePath, inputJson)) {
                            std::cerr << "❌ Failed to load JSON file" << std::endl;
                            failedFiles.push_back(filePath + " (JSON load failed)");
                            continue;
                        }
                        std::cout << "✅ Successfully loaded JSON data" << std::endl;
                        
                        // Step 2: Convert JSON to SkeletonData using JsonReader
                        spine42::SkeletonData skeletonData = spine42::readJsonData(inputJson);
                        std::cout << "✅ Successfully parsed JSON to SkeletonData" << std::endl;
                        
                        // Step 3: Convert SkeletonData to Binary using BinaryWriter
                        spine42::Binary binaryData = spine42::writeBinaryData(skeletonData);
                        std::cout << "✅ Successfully converted SkeletonData to binary (" << binaryData.size() << " bytes)" << std::endl;
                        
                        // Step 4: Save binary data to .json.skel file
                        std::string binaryPath = generateBinaryFileName(filePath);
                        if (!saveBinaryToFile(binaryPath, binaryData)) {
                            std::cerr << "❌ Failed to save binary file" << std::endl;
                            failedFiles.push_back(filePath + " (binary save failed)");
                            continue;
                        }
                        std::cout << "✅ Successfully saved binary to: " << binaryPath << std::endl;
                        
                        // Step 5: Load binary data back
                        spine42::Binary loadedBinaryData;
                        if (!loadBinaryFromFile(binaryPath, loadedBinaryData)) {
                            std::cerr << "❌ Failed to reload binary file" << std::endl;
                            failedFiles.push_back(filePath + " (binary reload failed)");
                            continue;
                        }
                        std::cout << "✅ Successfully reloaded binary data (" << loadedBinaryData.size() << " bytes)" << std::endl;
                        
                        // Step 6: Convert Binary back to SkeletonData using BinaryReader
                        spine42::SkeletonData reloadedSkeletonData = spine42::readBinaryData(loadedBinaryData);
                        std::cout << "✅ Successfully parsed binary to SkeletonData" << std::endl;
                        
                        // Step 7: Convert SkeletonData back to JSON using JsonWriter
                        Json outputJson = spine42::writeJsonData(reloadedSkeletonData);
                        std::cout << "✅ Successfully converted SkeletonData to JSON" << std::endl;
                        
                        // Step 8: Save final JSON to .json.skel.json file
                        std::string finalJsonPath = generateFinalJsonFileName(filePath);
                        if (saveJsonToFile(finalJsonPath, outputJson)) {
                            std::cout << "✅ Successfully saved final JSON to: " << finalJsonPath << std::endl;
                            successCount++;
                        } else {
                            std::cerr << "❌ Failed to save final JSON file" << std::endl;
                            failedFiles.push_back(filePath + " (final JSON save failed)");
                        }
                        
                    } catch (const std::exception& e) {
                        std::cerr << "❌ Error processing: " << e.what() << std::endl;
                        failedFiles.push_back(filePath + " (processing error: " + e.what() + ")");
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error scanning directory: " << e.what() << std::endl;
    }
    
    // Print results
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "=== JSON->Binary->JSON Round-trip Test Results ===" << std::endl;
    std::cout << "Total plain .json files found in data/42/*/export/: " << totalCount << std::endl;
    std::cout << "Successfully converted: " << successCount << std::endl;
    std::cout << "Failed: " << (totalCount - successCount) << std::endl;
    
    if (successCount == totalCount && totalCount > 0) {
        std::cout << "🎉 All round-trip conversions passed!" << std::endl;
    } else if (successCount > 0) {
        std::cout << "⚠️  Some round-trip conversions passed (" << successCount << "/" << totalCount << ")" << std::endl;
        std::cout << "Success rate: " << std::fixed << std::setprecision(1) 
                  << (100.0 * successCount / totalCount) << "%" << std::endl;
    } else {
        std::cout << "❌ All round-trip conversions failed." << std::endl;
    }
    
    // Show failed files
    if (!failedFiles.empty()) {
        std::cout << "\nFailed files:" << std::endl;
        for (const auto& file : failedFiles) {
            std::cout << "  - " << file << std::endl;
        }
    }
    
    std::cout << "\nYou can now use json_diff.py to compare .json and .json.skel.json files:" << std::endl;
    std::cout << "python tools/json_diff.py original.json original.json.skel.json" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
}

int main() {
    try {
        testJsonToBinaryToJson();
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\nPress Enter to exit...";
    std::cin.get();
    
    return 0;
}