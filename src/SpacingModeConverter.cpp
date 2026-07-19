#include "SkeletonData.h"

void convertSpacingMode4xTo3x(SkeletonData& skeleton) {
    for (auto& path : skeleton.pathConstraints) {
        if (path.spacingMode == SpacingMode_Proportional) {
            path.spacingMode = SpacingMode_Length;
        }
    }
}
