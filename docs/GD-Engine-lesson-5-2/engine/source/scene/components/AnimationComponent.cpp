#include "scene/components/AnimationComponent.h"
#include "scene/GameObject.h"

namespace eng
{
    void AnimationComponent::Update(float deltaTime)
    {
        if (!m_clip)
        {
            return;
        }

        if (!m_isPlaying)
        {
            return;
        }

        m_time += deltaTime;

        if (m_time > m_clip->duration)
        {
            if (m_looping)
            {
                m_time = std::fmod(m_time, m_clip->duration);
            }
            else
            {
                m_time = 0.0f;
                m_isPlaying = false;
                return;
            }
        }

        for (auto& binding : m_bindings)
        {
            auto& obj = binding.first;
            auto& trackIndices = binding.second->trackIndices;
            for (auto i : trackIndices)
            {
                auto& track = m_clip->tracks[i];
                if (!track.positions.empty())
                {
                    auto pos = Interpolate(track.positions, m_time);
                    obj->SetPosition(pos);
                }
                if (!track.rotations.empty())
                {
                    auto rot = Interpolate(track.rotations, m_time);
                    obj->SetRotation(rot);
                }
                if (!track.scales.empty())
                {
                    auto scale = Interpolate(track.scales, m_time);
                    obj->SetScale(scale);
                }
            }
        }
    }

    void AnimationComponent::SetClip(AnimationClip* clip)
    {
        m_clip = clip;
        BuildBindings();
    }

    void AnimationComponent::RegisterClip(const std::string& name, const std::shared_ptr<AnimationClip>& clip)
    {
        m_clips[name] = clip;
    }

    void AnimationComponent::Play(const std::string& name, bool loop)
    {
        if (m_clip && m_clip->name == name)
        {
            m_time = 0.0f;
            m_isPlaying = true;
            m_looping = loop;
        }
        else
        {
            auto it = m_clips.find(name);
            if (it != m_clips.end())
            {
                SetClip(it->second.get());
                m_time = 0.0f;
                m_isPlaying = true;
                m_looping = loop;
            }
        }
    }

    bool AnimationComponent::IsPlaying() const
    {
        return m_isPlaying;
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
            auto& track = m_clip->tracks[i];
            auto targetObject = m_owner->FindChildByName(track.targetName);
            if (targetObject)
            {
                auto it = m_bindings.find(targetObject);
                if (it != m_bindings.end())
                {
                    it->second->trackIndices.push_back(i);
                }
                else
                {
                    auto binding = std::make_unique<ObjectBinding>();
                    binding->object = targetObject;
                    binding->trackIndices.push_back(i);
                    m_bindings.emplace(targetObject, std::move(binding));
                }
            }
        }
    }

    glm::vec3 AnimationComponent::Interpolate(const std::vector<KeyFrameVec3>& keys, float time)
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
            float k = (time - keys[i0].time) / deltaTime;

            return glm::mix(keys[i0].value, keys[i1].value, k);
        }

        return keys.back().value;
    }

    glm::quat AnimationComponent::Interpolate(const std::vector<KeyFrameQuat>& keys, float time)
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
            float k = (time - keys[i0].time) / deltaTime;

            return glm::slerp(keys[i0].value, keys[i1].value, k);
        }

        return keys.back().value;
    }
}