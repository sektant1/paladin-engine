#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Common.h"

struct ma_sound;
struct ma_decoder;

namespace COA
{
class Audio
{
public:
    ~Audio();
    void  SetPosition(const glm::vec3 &position);
    void  Play(bool loop = false);
    void  Stop();
    bool  IsPlaying() const;
    void  SetVolume(float volume);
    float GetVolume() const;

    static std::shared_ptr<Audio> Load(const std::string &path);

private:
    std::unique_ptr<ma_sound>   m_sound;
    std::unique_ptr<ma_decoder> m_decoder;
    std::vector<char>           m_buffer;
};
}  // namespace COA
