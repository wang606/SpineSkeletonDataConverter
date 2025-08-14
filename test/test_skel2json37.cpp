// Test function for Spine 3.7 runtime validation
#include <spine/spine.h>
#include <iostream>
#include <string>

// 提供默认扩展实现
namespace spine {
    SpineExtension* getDefaultExtension() {
        static spine::DefaultSpineExtension defaultExtension;
        return &defaultExtension;
    }
}

extern "C" __declspec(dllexport) bool testLoadSkeletonJson37(const char* jsonPath, const char* atlasPath) {
    try {
        // Create atlas
        spine::Atlas* atlas = new spine::Atlas(atlasPath, nullptr);
        if (!atlas) {
            std::cerr << "Failed to load atlas: " << atlasPath << std::endl;
            return false;
        }

        // Create attachment loader
        spine::AtlasAttachmentLoader attachmentLoader(atlas);
        
        // Create skeleton json
        spine::SkeletonJson json(&attachmentLoader);
        
        // Load skeleton data
        spine::SkeletonData* skeletonData = json.readSkeletonDataFile(jsonPath);
        if (!skeletonData) {
            std::cerr << "Failed to load skeleton data: " << jsonPath << std::endl;
            delete atlas;
            return false;
        }

        // Create skeleton
        spine::Skeleton* skeleton = new spine::Skeleton(skeletonData);
        if (!skeleton) {
            std::cerr << "Failed to create skeleton" << std::endl;
            delete skeletonData;
            delete atlas;
            return false;
        }

        // Basic validation - check if skeleton has bones and slots
        bool valid = skeleton->getBones().size() > 0 && skeleton->getSlots().size() > 0;

        // Cleanup
        delete skeleton;
        delete skeletonData;
        delete atlas;

        return valid;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception during validation: " << e.what() << std::endl;
        return false;
    }
    catch (...) {
        std::cerr << "Unknown exception during validation" << std::endl;
        return false;
    }
}
