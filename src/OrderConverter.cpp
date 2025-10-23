#include "SkeletonData.h"

void convertOrder42ToBelow(SkeletonData& skeleton) {
    std::vector<size_t> orders; 
    for (auto& ik : skeleton.ikConstraints)
        orders.push_back(ik.order); 
    for (auto& transform : skeleton.transformConstraints)
        orders.push_back(transform.order);
    for (auto& path : skeleton.pathConstraints)
        orders.push_back(path.order);
    std::sort(orders.begin(), orders.end());
    orders.erase(std::unique(orders.begin(), orders.end()), orders.end());
    for (auto& ik : skeleton.ikConstraints) {
        auto it = std::find(orders.begin(), orders.end(), ik.order);
        if (it != orders.end())
            ik.order = std::distance(orders.begin(), it);
    }
    for (auto& transform : skeleton.transformConstraints) {
        auto it = std::find(orders.begin(), orders.end(), transform.order);
        if (it != orders.end())
            transform.order = std::distance(orders.begin(), it);
    }
    for (auto& path : skeleton.pathConstraints) {
        auto it = std::find(orders.begin(), orders.end(), path.order);
        if (it != orders.end())
            path.order = std::distance(orders.begin(), it);
    }
}