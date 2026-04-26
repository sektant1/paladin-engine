/**
 * @file Skeleton.h
 * @ingroup mnd_animation
 * @brief Bone hierarchy + bind/offset matrices for GPU-skinned meshes.
 */

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "Types.h"

namespace mnd
{

struct Bone
{
    std::string name;
    i32         parentIndex   = -1;            ///< Index into Skeleton::bones, or -1 for the root.
    glm::mat4   localBind     = glm::mat4(1);  ///< Local bind transform (relative to parent).
    glm::mat4   offsetMatrix  = glm::mat4(1);  ///< Inverse bind matrix in mesh space (Assimp `aiBone::mOffsetMatrix`).

    /// Bind decomposition (filled by `Skeleton::AddBone`). Used by `EvaluatePose`
    /// as a fallback when an animation track omits a TRS channel — without this
    /// a track that only animates rotation would zero out the bone's bind offset.
    glm::vec3 bindPos   = glm::vec3(0.0f);
    glm::quat bindRot   = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 bindScale = glm::vec3(1.0f);
};

class Skeleton
{
public:
    void  AddBone(const Bone &bone);
    i32   FindBoneIndex(const std::string &name) const;
    usize Size() const { return m_bones.size(); }

    const std::vector<Bone> &GetBones() const { return m_bones; }
    std::vector<Bone>       &GetBones() { return m_bones; }

private:
    std::vector<Bone>                       m_bones;
    std::unordered_map<std::string, size_t> m_nameToIndex;
};

}  // namespace mnd
