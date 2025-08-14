#define SPINE_VERSION_STRING "4.2"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <spine/spine.h>

namespace fs = std::filesystem;
using namespace spine;

SpineExtension *spine::getDefaultExtension() {
	return new spine::DefaultSpineExtension();
}

bool testLoadSkeletonJson(const std::string& jsonPath, const std::string& atlasPath) {
    std::cout << "\n=== Testing JSON file: " << jsonPath << " ===" << std::endl;
    
    try {
        // 检查文件是否存在
        if (!fs::exists(jsonPath)) {
            std::cerr << "JSON file does not exist: " << jsonPath << std::endl;
            return false;
        }
        
        if (!fs::exists(atlasPath)) {
            std::cerr << "Atlas file does not exist: " << atlasPath << std::endl;
            return false;
        }
        
        // 加载Atlas
        std::cout << "Loading atlas: " << atlasPath << std::endl;
        Atlas* atlas = new Atlas(atlasPath.c_str(), nullptr);
        
        if (!atlas) {
            std::cerr << "Failed to load atlas: " << atlasPath << std::endl;
            return false;
        }
        
        std::cout << "Atlas loaded successfully with " << atlas->getPages().size() << " pages" << std::endl;
        
        // 创建SkeletonJson加载器
        SkeletonJson skeletonJson(atlas);
        
        // 加载骨架数据
        std::cout << "Loading skeleton data from JSON..." << std::endl;
        SkeletonData* skeletonData = skeletonJson.readSkeletonDataFile(jsonPath.c_str());
        
        if (!skeletonData) {
            std::cerr << "Failed to load skeleton data: " << skeletonJson.getError().buffer() << std::endl;
            delete atlas;
            return false;
        }
        
        std::cout << "Skeleton data loaded successfully!" << std::endl;
        std::cout << "  Skeleton name: " << (skeletonData->getName().length() > 0 ? skeletonData->getName().buffer() : "unnamed") << std::endl;
        std::cout << "  Spine version: " << skeletonData->getVersion().buffer() << std::endl;
        std::cout << "  Hash: " << skeletonData->getHash().buffer() << std::endl;
        std::cout << "  Bones count: " << skeletonData->getBones().size() << std::endl;
        std::cout << "  Slots count: " << skeletonData->getSlots().size() << std::endl;
        std::cout << "  Skins count: " << skeletonData->getSkins().size() << std::endl;
        std::cout << "  Animations count: " << skeletonData->getAnimations().size() << std::endl;
        
        // 列出所有骨骼
        std::cout << "\n  Bones:" << std::endl;
        for (size_t i = 0; i < skeletonData->getBones().size(); ++i) {
            BoneData* bone = skeletonData->getBones()[i];
            std::cout << "    [" << i << "] " << bone->getName().buffer();
            if (bone->getParent()) {
                std::cout << " (parent: " << bone->getParent()->getName().buffer() << ")";
            }
            std::cout << std::endl;
        }
        
        // 列出所有插槽
        std::cout << "\n  Slots:" << std::endl;
        for (size_t i = 0; i < skeletonData->getSlots().size(); ++i) {
            SlotData* slot = skeletonData->getSlots()[i];
            std::cout << "    [" << i << "] " << slot->getName().buffer() 
                      << " (bone: " << slot->getBoneData().getName().buffer() << ")" << std::endl;
        }
        
        // 列出所有皮肤
        std::cout << "\n  Skins:" << std::endl;
        for (size_t i = 0; i < skeletonData->getSkins().size(); ++i) {
            Skin* skin = skeletonData->getSkins()[i];
            std::cout << "    [" << i << "] " << skin->getName().buffer() << std::endl;
        }
        
        // 列出所有动画
        std::cout << "\n  Animations:" << std::endl;
        for (size_t i = 0; i < skeletonData->getAnimations().size(); ++i) {
            Animation* animation = skeletonData->getAnimations()[i];
            std::cout << "    [" << i << "] " << animation->getName().buffer() 
                      << " (duration: " << animation->getDuration() << "s)" << std::endl;
        }
        
        // 创建一个骨架实例来测试运行时
        std::cout << "\n  Creating skeleton instance..." << std::endl;
        Skeleton* skeleton = new Skeleton(skeletonData);
        
        if (!skeleton) {
            std::cerr << "Failed to create skeleton instance" << std::endl;
            delete atlas;
            return false;
        }
        
        std::cout << "  Skeleton instance created successfully!" << std::endl;
        
        // 测试设置到默认pose
        skeleton->setToSetupPose();
        std::cout << "  Set to setup pose successfully!" << std::endl;
        
        // 如果有动画，创建动画状态来测试
        if (skeletonData->getAnimations().size() > 0) {
            std::cout << "\n  Testing animation state..." << std::endl;
            AnimationStateData* stateData = new AnimationStateData(skeletonData);
            AnimationState* animationState = new AnimationState(stateData);
            
            // 设置第一个动画
            Animation* firstAnimation = skeletonData->getAnimations()[0];
            animationState->setAnimation(0, firstAnimation, true);
            
            std::cout << "  Animation state created and first animation set successfully!" << std::endl;
            
            // 模拟一些帧的更新
            std::cout << "  Testing animation updates..." << std::endl;
            for (int frame = 0; frame < 5; ++frame) {
                float deltaTime = 1.0f / 60.0f; // 60 FPS
                animationState->update(deltaTime);
                animationState->apply(*skeleton);
                skeleton->updateWorldTransform(Physics_Update);
                std::cout << "    Frame " << frame << " updated successfully" << std::endl;
            }
            
            delete animationState;
            delete stateData;
        }
        
        // 清理
        delete skeleton;
        delete atlas;
        
        std::cout << "✓ JSON file test completed successfully!" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        return false;
    }
}

extern "C" __declspec(dllexport) bool testLoadSkeletonJson42(const char* jsonPath, const char* atlasPath) {
    return testLoadSkeletonJson(std::string(jsonPath), std::string(atlasPath));
}

int main(int argc, char* argv[]) {
    std::cout << "=== Spine 4.2 JSON Loader Test ===" << std::endl;
    
    // 如果提供了命令行参数，测试特定文件
    if (argc == 3) {
        std::string jsonPath = argv[1];
        std::string atlasPath = argv[2];
        
        std::cout << "Testing specific files:" << std::endl;
        std::cout << "JSON: " << jsonPath << std::endl;
        std::cout << "Atlas: " << atlasPath << std::endl;
        
        if (testLoadSkeletonJson(jsonPath, atlasPath)) {
            std::cout << "✓ Test passed!" << std::endl;
            return 0;
        } else {
            std::cout << "✗ Test failed!" << std::endl;
            return 1;
        }
    }
    
    // 默认测试所有样本
    std::vector<std::string> testSamples = {
        "alien", 
        "celestial-circus", 
        "coin", 
        "goblins", 
        "hero", 
        "mix-and-match", 
        "owl", 
        "powerup", 
        "raptor", 
        "sack", 
        "snowglobe", 
        "spineboy", 
        "stretchyman", 
        "tank", 
        "vine"
    };
    
    std::string basePath = "./data/42/";
    
    int successCount = 0;
    int totalCount = 0;
    
    for (const std::string& sampleName : testSamples) {
        std::string jsonPath = basePath + sampleName + "/export/" + sampleName + "-pro.skel.json";
        std::string atlasPath = basePath + sampleName + "/export/" + sampleName + ".atlas";
        
        // 如果没有.atlas文件，尝试-pma.atlas
        if (!fs::exists(atlasPath)) {
            atlasPath = basePath + sampleName + "/export/" + sampleName + "-pma.atlas";
        }
        
        totalCount++;
        
        if (testLoadSkeletonJson(jsonPath, atlasPath)) {
            successCount++;
        } else {
            std::cerr << "✗ Failed to load: " << sampleName << std::endl;
        }
        
        std::cout << std::endl;
    }
    
    std::cout << "=== Test Summary ===" << std::endl;
    std::cout << "Successful: " << successCount << "/" << totalCount << std::endl;
    std::cout << "Failed: " << (totalCount - successCount) << "/" << totalCount << std::endl;
    
    if (successCount == totalCount) {
        std::cout << "🎉 All tests passed! The JSON conversion is working correctly." << std::endl;
        return 0;
    } else {
        std::cout << "❌ Some tests failed. Please check the conversion process." << std::endl;
        return 1;
    }
}
