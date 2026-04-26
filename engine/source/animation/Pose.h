/**
 * @file Pose.h
 * @ingroup mnd_animation
 * @brief Per-frame skeletal pose: local bone transforms + final GPU palette.
 */

#pragma once

#include <vector>

#include <glm/mat4x4.hpp>

#include "Types.h"

namespace mnd
{

class Skeleton;
struct SkeletalAnimationClip;

/**
 * @brief Per-frame pose state for a Skeleton.
 *
 * `localTransforms[i]` is bone i's TRS relative to its parent for the current
 * playback time. `finalPalette[i]` is the matrix actually uploaded as
 * `uBones[i]` to the skinned vertex shader: it transforms a vertex from
 * mesh-bind space into mesh-posed space.
 */
struct Pose
{
    std::vector<glm::mat4> localTransforms;  ///< Animated TRS per bone (parent-relative).
    std::vector<glm::mat4> finalPalette;     ///< Skinning matrices: globalPose * offsetMatrix.
};

/// Resize `pose` buffers to match `skeleton` and reset to bind pose.
void ResetPoseToBind(const Skeleton &skeleton, Pose &pose);

/**
 * @brief Sample `clip` at `time` and write each animated bone's TRS into `pose.localTransforms`.
 *
 * Bones not driven by the clip retain their bind-pose local transform.
 */
void EvaluatePose(const SkeletalAnimationClip &clip, const Skeleton &skeleton, f32 time, Pose &pose);

/**
 * @brief Concatenate parent transforms and apply offsetMatrix to produce the GPU palette.
 *
 * Skeleton bones must be stored such that a parent comes before its children
 * (topological order). ModelImporter guarantees this.
 */
void ComputePalette(const Skeleton &skeleton, Pose &pose);

}  // namespace mnd
