#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <iomanip>
#include "SkeletonData42.h"
#include "common.h"

namespace fs = std::filesystem;

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

std::string generateOutputFileName(const std::string& inputPath) {
    fs::path path(inputPath);
    std::string stem = path.stem().string();
    fs::path directory = path.parent_path();
    
    // Remove .skel suffix if it exists
    if (stem.ends_with(".skel")) {
        stem = stem.substr(0, stem.length() - 5);
    }
    
    return (directory / (stem + ".skel2json.json")).string();
}

void testSkelToJson() {
    const std::string dataDir = "d:/Projects/SpineSkeletonDataConverter/data/42";
    int successCount = 0;
    int totalCount = 0;
    std::vector<std::string> failedFiles;
    
    std::cout << "Starting Spine Skeleton Data skel2json conversion test..." << std::endl;
    std::cout << "Using SkeletonData42BinaryReader and SkeletonData42JsonWriter" << std::endl;
    std::cout << "Scanning directory: " << dataDir << std::endl;
    std::cout << "Looking for files matching pattern: *.skel files" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    try {
        for (const auto& entry : fs::recursive_directory_iterator(dataDir)) {
            if (entry.is_regular_file()) {
                std::string filePath = entry.path().string();
                std::string fileName = entry.path().filename().string();
                
                // Check if the file ends with .skel
                if (fileName.ends_with(".skel")) {
                    totalCount++;
                    
                    std::cout << "\n[" << totalCount << "] Processing: " << fileName << std::endl;
                    std::cout << "Full path: " << filePath << std::endl;
                    
                    // Load binary data from .skel file
                    spine42::Binary binaryData;
                    if (!loadBinaryFromFile(filePath, binaryData)) {
                        std::cerr << "❌ Failed to load binary file" << std::endl;
                        failedFiles.push_back(filePath + " (load failed)");
                        continue;
                    }
                    
                    std::cout << "✅ Successfully loaded binary data (" << binaryData.size() << " bytes)" << std::endl;
                    
                    try {
                        // Read binary data into SkeletonData using SkeletonData42BinaryReader
                        spine42::SkeletonData skeletonData = spine42::readBinaryData(binaryData);
                        std::cout << "✅ Successfully read skeleton data from binary" << std::endl;
                        
                        // Write SkeletonData to JSON using SkeletonData42JsonWriter
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
    std::cout << "=== Skel2Json Test Results ===" << std::endl;
    std::cout << "Total .skel files found: " << totalCount << std::endl;
    std::cout << "Successfully converted: " << successCount << std::endl;
    std::cout << "Failed: " << (totalCount - successCount) << std::endl;
    
    if (successCount == totalCount && totalCount > 0) {
        std::cout << "🎉 All conversions passed!" << std::endl;
    } else if (successCount > 0) {
        std::cout << "⚠️  Some conversions passed (" << successCount << "/" << totalCount << ")" << std::endl;
        std::cout << "Success rate: " << std::fixed << std::setprecision(1) 
                  << (100.0 * successCount / totalCount) << "%" << std::endl;
    } else {
        std::cout << "❌ All conversions failed." << std::endl;
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
        testSkelToJson();
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\nPress Enter to exit...";
    std::cin.get();
    
    return 0;
}
