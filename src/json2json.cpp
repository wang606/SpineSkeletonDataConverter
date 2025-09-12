#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
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

std::string generateOutputFileName(const std::string& inputPath) {
    fs::path path(inputPath);
    std::string stem = path.stem().string();
    fs::path directory = path.parent_path();
    
    return (directory / (stem + ".json.json")).string();
}

void testJsonReadWrite() {
    const std::string dataDir = "d:/Projects/SpineSkeletonDataConverter/data/42";
    int successCount = 0;
    int totalCount = 0;
    std::vector<std::string> failedFiles;
    
    std::cout << "Starting Spine Skeleton Data JSON read/write test..." << std::endl;
    std::cout << "Using SkeletonData42JsonReader and SkeletonData42JsonWriter" << std::endl;
    std::cout << "Scanning directory: " << dataDir << std::endl;
    std::cout << "Looking for files matching pattern: */export/*.json (excluding .skel.json and .json.json)" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    try {
        for (const auto& entry : fs::recursive_directory_iterator(dataDir)) {
            if (entry.is_regular_file()) {
                std::string filePath = entry.path().string();
                std::string fileName = entry.path().filename().string();
                
                // Check if the file path matches the pattern */export/*.json
                // and ends with .json but not with .skel.json or .json.json
                bool isInExportDir = filePath.find("\\export\\") != std::string::npos || filePath.find("/export/") != std::string::npos;
                bool isValidJson = fileName.ends_with(".json") && !fileName.ends_with(".skel.json") && !fileName.ends_with(".json.json");
                
                if (isInExportDir && isValidJson) {
                    totalCount++;
                    
                    std::cout << "\n[" << totalCount << "] Processing: " << fileName << std::endl;
                    std::cout << "Full path: " << filePath << std::endl;
                    
                    // Load JSON from file
                    Json inputJson;
                    if (!loadJsonFromFile(filePath, inputJson)) {
                        std::cerr << "❌ Failed to load JSON file" << std::endl;
                        failedFiles.push_back(filePath + " (load failed)");
                        continue;
                    }
                    
                    try {
                        // Read JSON data into SkeletonData using SkeletonData42JsonReader
                        spine42::SkeletonData skeletonData = spine42::readJsonData(inputJson);
                        std::cout << "✅ Successfully read skeleton data" << std::endl;
                        
                        // Write SkeletonData back to JSON using SkeletonData42JsonWriter
                        Json outputJson = spine42::writeJsonData(skeletonData);
                        std::cout << "✅ Successfully converted skeleton data to JSON" << std::endl;
                        
                        // Generate output file name
                        std::string outputPath = generateOutputFileName(filePath);
                        
                        // Save JSON to file
                        if (saveJsonToFile(outputPath, outputJson)) {
                            std::cout << "✅ Successfully saved to: " << outputPath << std::endl;
                            successCount++;
                        } else {
                            std::cerr << "❌ Failed to save JSON file" << std::endl;
                            failedFiles.push_back(filePath + " (save failed)");
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
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "=== Test Results ===" << std::endl;
    std::cout << "Total files found: " << totalCount << std::endl;
    std::cout << "Successfully processed: " << successCount << std::endl;
    std::cout << "Failed: " << (totalCount - successCount) << std::endl;
    
    if (successCount == totalCount && totalCount > 0) {
        std::cout << "🎉 All tests passed!" << std::endl;
    } else if (successCount > 0) {
        std::cout << "⚠️  Some tests passed (" << successCount << "/" << totalCount << ")" << std::endl;
    } else {
        std::cout << "❌ All tests failed." << std::endl;
    }
    
    // Show failed files
    if (!failedFiles.empty()) {
        std::cout << "\nFailed files:" << std::endl;
        for (const auto& file : failedFiles) {
            std::cout << "  - " << file << std::endl;
        }
    }
    
    std::cout << std::string(60, '=') << std::endl;
}

int main() {
    try {
        testJsonReadWrite();
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\nPress Enter to exit...";
    std::cin.get();
    
    return 0;
}