#pragma once

#include "json.hpp"
using Json = nlohmann::ordered_json; 

std::string dumpJson(const Json&); 
std::string uint64ToBase64(uint64_t);
uint64_t base64ToUint64(const std::string&);
