#include "scene/components/AnimationComponent.h"

#include "animation/SkeletalAnimationClip.h"
#include "animation/Skeleton.h"
#include "scene/GameObject.h"

namespace mnd
{
void AnimationComponent::Update(float deltaTime)
{
    if (!m_isPlaying)
    {
        return;
    }
    if (!m_clip && !m_activeSkelClip)
    {
        return;
    }

    // Effective duration: longest of the active lanes.
    float duration = 0.0f;
    if (m_clip)            duration = std::max(duration, m_clip->duration);
    if (m_activeSkelClip)  duration = std::max(duration, m_activeSkelClip->duration);

    m_time += deltaTime;
    if (duration > 0.0f && m_time > duration)
    {
        if (m_looping)
        {
            m_time = std::fmod(m_time, duration);
        } else
        {
            m_time      = duration;
            m_isPlaying = false;
        }
    }

    // Node-track lane: drive child GameObjects by name.
    if (m_clip)
    {
        for (auto &binding : m_bindings)
        {
            auto &obj          = binding.first;
            auto &trackIndices = binding.second->trackIndices;
            for (auto i : trackIndices)
            {
                auto &track = m_clip->tracks[i];
                if (!track.positions.empty())
                {
                    obj->SetPosition(Interpolate(track.positions, m_time));
                }
                if (!track.rotations.empty())
                {
                    obj->SetRotation(Interpolate(track.rotations, m_time));
                }
                if (!track.scales.empty())
                {
                    obj->SetScale(Interpolate(track.scales, m_time));
                }
            }
        }
    }

    // Skeletal lane: evaluate pose → fill GPU palette read by SkinnedMeshComponent.
    if (m_activeSkelClip && m_skeleton)
    {
        EvaluatePose(*m_activeSkelClip, *m_skeleton, m_time, m_pose);
        ComputePalette(*m_skeleton, m_pose);
        m_palette = m_pose.finalPalette;
    }
}

void AnimationComponent::SetClip(AnimationClip *clip)
{
    m_clip = clip;
    BuildBindings();
}

void AnimationComponent::RegisterClip(const std::string &name, const std::shared_ptr<AnimationClip> &clip)
{
    m_clips[name] = clip;
}

void AnimationComponent::RegisterSkeletalClip(const std::string &name, const std::shared_ptr<SkeletalAnimationClip> &clip)
{
    // Step 7 wires this into Update(). For now we just stash so the importer compiles.
    m_skelClips[name] = clip;
}

void AnimationComponent::SetSkeleton(const std::shared_ptr<Skeleton> &skeleton)
{
    m_skeleton = skeleton;
}

void AnimationComponent::Play(const std::string &name, bool loop)
{
    bool found = false;

    auto nodeIt = m_clips.find(name);
    if (nodeIt != m_clips.end())
    {
        SetClip(nodeIt->second.get());
        found = true;
    } else
    {
        m_clip = nullptr;
        m_bindings.clear();
    }

    auto skelIt = m_skelClips.find(name);
    if (skelIt != m_skelClips.end())
    {
        m_activeSkelClip = skelIt->second.get();
        if (m_skeleton)
        {
            ResetPoseToBind(*m_skeleton, m_pose);
        }
        found = true;
    } else
    {
        m_activeSkelClip = nullptr;
    }

    if (!found)
    {
        return;
    }

    m_time      = 0.0f;
    m_isPlaying = true;
    m_looping   = loop;
}

void AnimationComponent::BuildBindings()
{
    m_bindings.clear();
    if (!m_clip)
    {
        return;
    }

    for (size_t i = 0; i < m_clip->tracks.size(); ++i)
    {
        auto &track        = m_clip->tracks[i];
        auto  targetObject = m_owner->FindChildByName(track.targetName);
        if (targetObject)
        {
            auto it = m_bindings.find(targetObject);
            if (it != m_bindings.end())
            {
                it->second->trackIndices.push_back(i);
            } else
            {
                auto binding    = std::make_unique<ObjectBinding>();
                binding->object = targetObject;
                binding->trackIndices.push_back(i);
                m_bindings.emplace(targetObject, std::move(binding));
            }
        }
    }
}

glm::vec3 AnimationComponent::Interpolate(const std::vector<KeyFrameVec3> &keys, float time)
{
    if (keys.empty())
    {
        return glm::vec3(0.0f);
    }

    if (keys.size() == 1)
    {
        return keys[0].value;
    }

    if (time <= keys.front().time)
    {
        return keys.front().value;
    }

    if (time >= keys.back().time)
    {
        return keys.back().value;
    }

    size_t i0 = 0;
    size_t i1 = 0;

    for (size_t i = 1; i < keys.size(); ++i)
    {
        if (time <= keys[i].time)
        {
            i1 = i;
            break;
        }
    }

    i0 = i1 > 0 ? i1 - 1 : 0;

    if (time >= keys[i0].time && time <= keys[i1].time)
    {
        float deltaTime = keys[i1].time - keys[i0].time;
        float k         = (time - keys[i0].time) / deltaTime;

        return glm::mix(keys[i0].value, keys[i1].value, k);
    }

    return keys.back().value;
}

glm::quat AnimationComponent::Interpolate(const std::vector<KeyFrameQuat> &keys, float time)
{
    if (keys.empty())
    {
        return glm::vec3(0.0f);
    }

    if (keys.size() == 1)
    {
        return keys[0].value;
    }

    if (time <= keys.front().time)
    {
        return keys.front().value;
    }

    if (time >= keys.back().time)
    {
        return keys.back().value;
    }

    size_t i0 = 0;
    size_t i1 = 0;

    for (size_t i = 1; i < keys.size(); ++i)
    {
        if (time <= keys[i].time)
        {
            i1 = i;
            break;
        }
    }

    i0 = i1 > 0 ? i1 - 1 : 0;

    if (time >= keys[i0].time && time <= keys[i1].time)
    {
        float deltaTime = keys[i1].time - keys[i0].time;
        float k         = (time - keys[i0].time) / deltaTime;

        return glm::slerp(keys[i0].value, keys[i1].value, k);
    }

    return keys.back().value;
}

bool AnimationComponent::IsPlaying()
{
    return m_isPlaying;
}

}  // namespace mnd
