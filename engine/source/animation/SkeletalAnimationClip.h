/**
 * @file SkeletalAnimationClip.h
 * @ingroup mnd_animation
 * @brief Per-bone TRS keyframe tracks driving a Skeleton.
 */

#pragma once

#include <string>
#include <vector>

#include "Types.h"
#include "animation/KeyFrame.h"

namespace mnd
{

struct BoneTrack
{
    i32                       boneIndex = -1;  ///< Index into Skeleton::bones.
    std::vector<KeyFrameVec3> positions;
    std::vector<KeyFrameQuat> rotations;
    std::vector<KeyFrameVec3> scales;
};

struct SkeletalAnimationClip
{
    std::string            name;
    f32                    duration = 0.0f;
    bool                   looping  = true;
    std::vector<BoneTrack> tracks;
};

}  // namespace mnd
