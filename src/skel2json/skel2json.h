#pragma once

#include <string>

// Spine version conversion functions in their respective namespaces
namespace spine37 {
    bool skel2json37(const std::string& skelPath, const std::string& jsonPath);
}

namespace spine38 {
    bool skel2json38(const std::string& skelPath, const std::string& jsonPath);
}

namespace spine40 {
    bool skel2json40(const std::string& skelPath, const std::string& jsonPath);
}

namespace spine41 {
    bool skel2json41(const std::string& skelPath, const std::string& jsonPath);
}

namespace spine42 {
    bool skel2json42(const std::string& skelPath, const std::string& jsonPath);
}
