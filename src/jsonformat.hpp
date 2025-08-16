#ifndef JSONFORMAT_HPP
#define JSONFORMAT_HPP

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include "json.hpp"
using json = nlohmann::ordered_json; 

inline std::string formatNumber(double v) {
    if (std::floor(v) == v) {
        return std::to_string((long long)v);
    }
    std::ostringstream oss;
    if (std::fabs(v) > 1e6 || (std::fabs(v) > 0 && std::fabs(v) < 0.001)) {
        double absV = std::fabs(v);
        if (absV < 0.01) {
            oss << std::uppercase << std::scientific << std::setprecision(1) << v;
        } else {
            oss << std::uppercase << std::scientific << std::setprecision(2) << v;
        }
        return oss.str();
    }
    if (std::fabs(v) >= 1.0) {
        oss << std::fixed << std::setprecision(2) << v;
    } else {
        oss << std::fixed << std::setprecision(5) << v;
    }
    return oss.str();
}

inline void customDump(const json& j, std::ostringstream& oss) {
    if (j.is_array()) {
        oss << "["; 
        for (size_t i = 0; i < j.size(); i++) {
            customDump(j[i], oss);
            if (i + 1 < j.size()) oss << ",";
        }
        oss << "]";
    } else if (j.is_object()) {
        oss << "{";
        bool first = true;
        for (auto it = j.begin(); it != j.end(); ++it) {
            if (!first) oss << ",";
            oss << "\"" << it.key() << "\":";
            customDump(it.value(), oss);
            first = false;
        }
        oss << "}";
    } else if (j.is_number_float()) {
        oss << formatNumber(j.get<double>());
    } else if (j.is_number_integer() || j.is_number_unsigned()) {
        oss << j.get<long long>();
    } else if (j.is_string()) {
        oss << "\"" << j.get<std::string>() << "\"";
    } else if (j.is_boolean()) {
        oss << (j.get<bool>() ? "true" : "false");
    } else if (j.is_null()) {
        oss << "null";
    }
}

inline std::string customDump(const json& j) {
    std::ostringstream oss;
    customDump(j, oss);
    return oss.str();
}

#endif // JSONFORMAT_HPP
