/**
 * @file Audio.h
 * @ingroup mnd_audio
 * @brief Reference-counted audio clip with 3D positioning.
 *
 * A single mnd::Audio instance wraps one decoded miniaudio stream. Clips
 * are loaded through the static factory and shared between callers via
 * `std::shared_ptr<Audio>`. Hand them to a mnd::AudioComponent to
 * trigger playback through the engine's mnd::AudioManager.
 *
 * @code
 *   auto shot = mnd::Audio::Load("sfx/laser.wav");
 *   shot->SetVolume(0.6F);
 *   shot->Play();
 * @endcode
 */

#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Common.h"

struct ma_sound;
struct ma_decoder;

namespace mnd
{
/**
 * @ingroup mnd_audio
 * @brief Single playable audio clip backed by a decoded miniaudio stream.
 */
class Audio
{
 public:
    ~Audio();

    /// Place the emitter at a world-space position (3D spatialisation).
    void SetPosition(const glm::vec3 &position);
    /// Start playback. Pass @c true to loop forever.
    void Play(bool loop = false);
    /// Halt playback if currently playing; no-op otherwise.
    void Stop();
    /// True between Play() and natural end / Stop().
    [[nodiscard]] bool IsPlaying() const;
    /// Linear gain in [0, 1]. Values above 1 amplify but may clip.
    void                SetVolume(float volume);
    [[nodiscard]] float GetVolume() const;

    /**
     * @brief Decode an audio file from disk and return a shared instance.
     * @param path Path under the assets directory; resolved by FileSystem.
     * @return Non-null shared pointer on success, null on decode failure.
     */
    static std::shared_ptr<Audio> Load(const std::string &path);

 private:
    std::unique_ptr<ma_sound>   m_sound;
    std::unique_ptr<ma_decoder> m_decoder;
    std::vector<char>           m_buffer;
};
}  // namespace mnd
