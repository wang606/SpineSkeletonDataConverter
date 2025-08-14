#define SPINE_VERSION_STRING "4.1"

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
        
        // 更严格的验证
        std::cout << "\n  Performing detailed validation..." << std::endl;
        
        // 验证版本兼容性
        String spineVersion = skeletonData->getVersion();
        if (spineVersion.length() > 0 && !spineVersion.startsWith("4.1")) {
            std::cerr << "  Warning: Spine version mismatch. Expected 4.1.x, got: " << spineVersion.buffer() << std::endl;
        }
        
        // 验证骨骼层次结构
        std::cout << "  Validating bone hierarchy..." << std::endl;
        for (size_t i = 0; i < skeletonData->getBones().size(); ++i) {
            BoneData* bone = skeletonData->getBones()[i];
            if (bone->getParent()) {
                // 确保父骨骼在当前骨骼之前定义
                bool parentFound = false;
                for (size_t j = 0; j < i; ++j) {
                    if (skeletonData->getBones()[j] == bone->getParent()) {
                        parentFound = true;
                        break;
                    }
                }
                if (!parentFound) {
                    std::cerr << "  Error: Bone '" << bone->getName().buffer() 
                              << "' references parent '" << bone->getParent()->getName().buffer() 
                              << "' that is not defined before it!" << std::endl;
                    delete skeleton;
                    delete atlas;
                    return false;
                }
            }
        }
        
        // 验证插槽-骨骼关系
        std::cout << "  Validating slot-bone relationships..." << std::endl;
        for (size_t i = 0; i < skeletonData->getSlots().size(); ++i) {
            SlotData* slot = skeletonData->getSlots()[i];
            BoneData& slotBone = slot->getBoneData();
            
            // 确保插槽引用的骨骼存在
            bool boneFound = false;
            for (size_t j = 0; j < skeletonData->getBones().size(); ++j) {
                if (skeletonData->getBones()[j] == &slotBone) {
                    boneFound = true;
                    break;
                }
            }
            if (!boneFound) {
                std::cerr << "  Error: Slot '" << slot->getName().buffer() 
                          << "' references non-existent bone '" << slotBone.getName().buffer() << "'!" << std::endl;
                delete skeleton;
                delete atlas;
                return false;
            }
        }
        
        // 验证皮肤和附件
        std::cout << "  Validating skins and attachments..." << std::endl;
        for (size_t i = 0; i < skeletonData->getSkins().size(); ++i) {
            Skin* skin = skeletonData->getSkins()[i];
            
            // 测试设置皮肤
            skeleton->setSkin(skin);
            skeleton->setSlotsToSetupPose();
            std::cout << "    Validated skin: " << skin->getName().buffer() << std::endl;
        }
        
        // 测试动画时间轴的完整性
        std::cout << "  Validating animation timelines..." << std::endl;
        for (size_t i = 0; i < skeletonData->getAnimations().size(); ++i) {
            Animation* animation = skeletonData->getAnimations()[i];
            
            // 基本验证：确保动画有有效的持续时间
            if (animation->getDuration() <= 0) {
                std::cerr << "  Warning: Animation '" << animation->getName().buffer() 
                          << "' has zero or negative duration!" << std::endl;
            }
            
            std::cout << "    Validated animation: " << animation->getName().buffer() 
                      << " (duration: " << animation->getDuration() << "s)" << std::endl;
        }
        
        // 更严格的骨架测试
        std::cout << "  Performing comprehensive skeleton test..." << std::endl;
        
        // 测试所有皮肤
        for (size_t i = 0; i < skeletonData->getSkins().size(); ++i) {
            Skin* skin = skeletonData->getSkins()[i];
            skeleton->setSkin(skin);
            skeleton->setSlotsToSetupPose();
            std::cout << "    Tested skin: " << skin->getName().buffer() << std::endl;
        }
        
        // 测试所有动画的完整播放
        if (skeletonData->getAnimations().size() > 0) {
            std::cout << "  Testing complete animation playback..." << std::endl;
            AnimationStateData* stateData = new AnimationStateData(skeletonData);
            AnimationState* animationState = new AnimationState(stateData);
            
            for (size_t i = 0; i < skeletonData->getAnimations().size(); ++i) {
                Animation* animation = skeletonData->getAnimations()[i];
                std::cout << "    Testing animation: " << animation->getName().buffer() << std::endl;
                
                // 清除所有轨道
                animationState->clearTracks();
                
                // 设置动画
                animationState->setAnimation(0, animation, false);
                
                // 测试整个动画的播放
                float duration = animation->getDuration();
                int testFrames = static_cast<int>(duration * 30); // 30 FPS测试
                if (testFrames > 300) testFrames = 300; // 最多测试10秒
                
                for (int frame = 0; frame < testFrames; ++frame) {
                    float deltaTime = 1.0f / 30.0f;
                    animationState->update(deltaTime);
                    animationState->apply(*skeleton);
                    skeleton->updateWorldTransform();
                }
                
                std::cout << "      Animation played successfully for " << testFrames << " frames" << std::endl;
            }
            
            delete animationState;
            delete stateData;
        }
        
        std::cout << "  All validations passed!" << std::endl;
        
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

extern "C" __declspec(dllexport) bool testLoadSkeletonJson41(const char* jsonPath, const char* atlasPath) {
    return testLoadSkeletonJson(std::string(jsonPath), std::string(atlasPath));
}

int main(int argc, char* argv[]) {
    std::cout << "=== Spine 4.1 JSON Loader Test ===" << std::endl;
    
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
    
    std::string basePath = "./data/41/";
    
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
