#include "animation/Skeleton.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

namespace mnd
{

void Skeleton::AddBone(const Bone &bone)
{
    Bone copy = bone;

    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(copy.localBind, copy.bindScale, copy.bindRot, copy.bindPos, skew, perspective);

    m_nameToIndex[copy.name] = m_bones.size();
    m_bones.push_back(copy);
}

i32 Skeleton::FindBoneIndex(const std::string &name) const
{
    auto it = m_nameToIndex.find(name);
    if (it == m_nameToIndex.end())
    {
        return -1;
    }
    return static_cast<i32>(it->second);
}

}  // namespace mnd
