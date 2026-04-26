/**
 * @file KeyFrame.h
 * @ingroup mnd_animation
 * @brief Shared keyframe value types for transform-track animation.
 */

#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

namespace mnd
{

struct KeyFrameVec3
{
    float     time  = 0.0f;
    glm::vec3 value = glm::vec3(0.0f);
};

struct KeyFrameQuat
{
    float     time  = 0.0f;
    glm::quat value = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
};

}  // namespace mnd
