#include "scene/components/AudioComponent.h"
#include "scene/GameObject.h"

namespace eng
{
    void AudioComponent::LoadProperties(const nlohmann::json& json)
    {
        if (json.contains("audio"))
        {
            auto& clipsObject = json["audio"];
            for (auto& clip : clipsObject)
            {
                std::string name = clip.value("name", "noname");
                std::string path = clip.value("path", "");
                auto audio = Audio::Load(path);
                if (audio)
                {
                    float volume = clip.value("volume", 1.0f);
                    audio->SetVolume(volume);
                    RegisterAudio(name, audio);
                }
            }
        }
    }

    void AudioComponent::Update(float deltaTime)
    {
        auto pos = m_owner->GetWorldPosition();
        for (auto& clip : m_clips)
        {
            if (clip.second->IsPlaying())
            {
                clip.second->SetPosition(pos);
            }
        }
    }

    void AudioComponent::RegisterAudio(const std::string& name, std::shared_ptr<Audio>& clip)
    {
        m_clips[name] = clip;
    }

    void AudioComponent::Play(const std::string& name, bool loop)
    {
        auto it = m_clips.find(name);
        if (it != m_clips.end())
        {
            if (it->second)
            {
                it->second->Play(loop);
            }
        }
    }

    void AudioComponent::Stop(const std::string& name)
    {
        auto it = m_clips.find(name);
        if (it != m_clips.end())
        {
            if (it->second)
            {
                it->second->Stop();
            }
        }
    }

    bool AudioComponent::IsPlaying(const std::string& name)
    {
        auto it = m_clips.find(name);
        if (it != m_clips.end())
        {
            if (it->second)
            {
                return it->second->IsPlaying();
            }
        }

        return false;
    }
}