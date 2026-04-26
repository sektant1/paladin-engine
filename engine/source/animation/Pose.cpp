#include "animation/Pose.h"

#include <algorithm>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "animation/Skeleton.h"
#include "animation/SkeletalAnimationClip.h"

namespace mnd
{

namespace
{
template <typename Key>
usize FindKeyIndex(const std::vector<Key> &keys, f32 time)
{
    for (usize i = 0; i + 1 < keys.size(); ++i)
    {
        if (time < keys[i + 1].time)
        {
            return i;
        }
    }
    return keys.empty() ? 0 : keys.size() - 1;
}

glm::vec3 InterpolateVec3(const std::vector<KeyFrameVec3> &keys, f32 time)
{
    if (keys.empty())
    {
        return glm::vec3(0.0f);
    }
    if (keys.size() == 1)
    {
        return keys.front().value;
    }
    auto i0 = FindKeyIndex(keys, time);
    auto i1 = std::min(i0 + 1, keys.size() - 1);
    if (i0 == i1)
    {
        return keys[i0].value;
    }
    f32 dt = keys[i1].time - keys[i0].time;
    f32 t  = dt > 0.0f ? (time - keys[i0].time) / dt : 0.0f;
    t      = std::clamp(t, 0.0f, 1.0f);
    return glm::mix(keys[i0].value, keys[i1].value, t);
}

glm::quat InterpolateQuat(const std::vector<KeyFrameQuat> &keys, f32 time)
{
    if (keys.empty())
    {
        return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    }
    if (keys.size() == 1)
    {
        return keys.front().value;
    }
    auto i0 = FindKeyIndex(keys, time);
    auto i1 = std::min(i0 + 1, keys.size() - 1);
    if (i0 == i1)
    {
        return keys[i0].value;
    }
    f32 dt = keys[i1].time - keys[i0].time;
    f32 t  = dt > 0.0f ? (time - keys[i0].time) / dt : 0.0f;
    t      = std::clamp(t, 0.0f, 1.0f);
    return glm::slerp(keys[i0].value, keys[i1].value, t);
}

glm::mat4 ComposeTRS(const glm::vec3 &translation, const glm::quat &rotation, const glm::vec3 &scale)
{
    glm::mat4 mat(1.0f);
    mat = glm::translate(mat, translation);
    mat = mat * glm::mat4_cast(rotation);
    mat = glm::scale(mat, scale);
    return mat;
}
}  // namespace

void ResetPoseToBind(const Skeleton &skeleton, Pose &pose)
{
    const auto &bones = skeleton.GetBones();
    pose.localTransforms.resize(bones.size());
    pose.finalPalette.resize(bones.size());
    for (usize i = 0; i < bones.size(); ++i)
    {
        pose.localTransforms[i] = bones[i].localBind;
        pose.finalPalette[i]    = glm::mat4(1.0f);
    }
}

void EvaluatePose(const SkeletalAnimationClip &clip, const Skeleton &skeleton, f32 time, Pose &pose)
{
    const auto &bones = skeleton.GetBones();
    if (pose.localTransforms.size() != bones.size())
    {
        ResetPoseToBind(skeleton, pose);
    } else
    {
        for (usize i = 0; i < bones.size(); ++i)
        {
            pose.localTransforms[i] = bones[i].localBind;
        }
    }

    for (const auto &track : clip.tracks)
    {
        if (track.boneIndex < 0 || static_cast<usize>(track.boneIndex) >= bones.size())
        {
            continue;
        }
        // Missing channels fall back to the bone's bind component, not zero —
        // otherwise a rotation-only track would collapse the bone's bind translation/scale.
        const Bone &bone = bones[track.boneIndex];
        glm::vec3 t = track.positions.empty() ? bone.bindPos   : InterpolateVec3(track.positions, time);
        glm::quat r = track.rotations.empty() ? bone.bindRot   : InterpolateQuat(track.rotations, time);
        glm::vec3 s = track.scales.empty()    ? bone.bindScale : InterpolateVec3(track.scales,    time);

        pose.localTransforms[track.boneIndex] = ComposeTRS(t, r, s);
    }
}

void ComputePalette(const Skeleton &skeleton, Pose &pose)
{
    const auto &bones = skeleton.GetBones();
    if (pose.localTransforms.size() != bones.size())
    {
        ResetPoseToBind(skeleton, pose);
    }
    pose.finalPalette.resize(bones.size());

    std::vector<glm::mat4> globalTransforms(bones.size(), glm::mat4(1.0f));

    for (usize i = 0; i < bones.size(); ++i)
    {
        const auto &bone = bones[i];
        if (bone.parentIndex < 0)
        {
            globalTransforms[i] = pose.localTransforms[i];
        } else
        {
            globalTransforms[i] = globalTransforms[bone.parentIndex] * pose.localTransforms[i];
        }
        pose.finalPalette[i] = globalTransforms[i] * bone.offsetMatrix;
    }
}

}  // namespace mnd
