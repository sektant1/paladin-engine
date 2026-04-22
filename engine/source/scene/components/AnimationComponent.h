/**
 * @file AnimationComponent.h
 * @brief Skeletal / transform animation system driven by GLTF keyframe data.
 *
 * ## Data model
 * ```
 *  AnimationClip
 *   └── TransformTrack[]   (one per animated GLTF node)
 *        ├── KeyFrameVec3[] positions
 *        ├── KeyFrameQuat[] rotations
 *        └── KeyFrameVec3[] scales
 * ```
 * Each track targets a child GameObject by name (targetName). On Play(),
 * AnimationComponent walks the owner's child hierarchy and caches a binding
 * (ObjectBinding) that maps track indices to live GameObjects.
 *
 * ## Per-frame playback
 * Update() advances m_time by deltaTime, linearly interpolates between the
 * surrounding keyframes for each track, and writes the result directly to
 * the target GameObject's local position / rotation / scale.
 *
 * ## Loading
 * AnimationClips are populated by GameObject::LoadGLTF() from the GLTF
 * animation channels, then registered on this component via RegisterClip().
 *
 * @code
 *   auto *anim = gunObject->GetComponent<AnimationComponent>();
 *   anim->Play("fire", false);   // play once, no loop
 * @endcode
 *
 * @see GameObject::LoadGLTF, TransformTrack, AnimationClip
 */

#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

#include "scene/Component.h"

namespace ENG
{

/**
 * @brief A single keyframe for a vec3 channel (position or scale).
 */
struct KeyFrameVec3
{
    float     time  = 0.0f;               ///< Keyframe timestamp in seconds.
    glm::vec3 value = glm::vec3(0.0f);   ///< Vec3 value at this keyframe.
};

/**
 * @brief A single keyframe for a quaternion rotation channel.
 */
struct KeyFrameQuat
{
    float     time  = 0.0f;                               ///< Keyframe timestamp in seconds.
    glm::quat value = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); ///< Unit quaternion at this keyframe.
};

/**
 * @brief Animation data for one GLTF node: position, rotation, and scale tracks.
 *
 * targetName matches the child GameObject name so AnimationComponent can
 * resolve the live object reference at Play() time.
 */
struct TransformTrack
{
    std::string               targetName;  ///< Name of the child GameObject to animate.
    std::vector<KeyFrameVec3> positions;   ///< Sorted position keyframes.
    std::vector<KeyFrameQuat> rotations;   ///< Sorted rotation keyframes (slerp-interpolated).
    std::vector<KeyFrameVec3> scales;      ///< Sorted scale keyframes.
};

/**
 * @brief A named, playable animation containing one or more transform tracks.
 */
struct AnimationClip
{
    std::string                 name;             ///< Clip identifier (matches GLTF animation name).
    float                       duration = 0.0f;  ///< Total clip length in seconds.
    bool                        looping  = true;  ///< Whether playback wraps at the end.
    std::vector<TransformTrack> tracks;            ///< Per-node animation channels.
};

/**
 * @brief Runtime binding from a TransformTrack to a live child GameObject.
 *
 * Built once when Play() is called (or when the clip changes) by walking
 * the owner's child hierarchy and matching names to track targets.
 */
struct ObjectBinding
{
    GameObject         *object = nullptr;  ///< Non-owning pointer to the target child.
    std::vector<size_t> trackIndices;      ///< Indices into AnimationClip::tracks that drive this object.
};

/**
 * @brief Drives transform animation on a hierarchy of GameObjects.
 *
 * Attach to the root of a GLTF-loaded hierarchy. Register the clips loaded
 * from the file (this is done automatically by GameObject::LoadGLTF), then
 * call Play() to start playback.
 */
class AnimationComponent : public Component
{
    COMPONENT(AnimationComponent)
public:
    /**
     * @brief Advance playback time and write interpolated transforms to bound objects.
     * @param deltaTime Seconds since the previous frame.
     */
    void Update(float deltaTime) override;

    /**
     * @brief Set the active clip directly (bypasses the name registry).
     * @param clip Raw pointer to the clip to play; not owned.
     */
    void SetClip(AnimationClip *clip);

    /**
     * @brief Register a named clip so it can be started via Play().
     * @param name Lookup key (typically the GLTF animation name).
     * @param clip Shared AnimationClip loaded from GLTF.
     */
    void RegisterClip(const std::string &name, const std::shared_ptr<AnimationClip> &clip);

    /**
     * @brief Start playing a registered clip by name.
     *
     * Resolves target GameObjects by name (BuildBindings) and resets m_time
     * to 0. Calling Play() while already playing restarts from the beginning.
     *
     * @param name Loop key registered via RegisterClip.
     * @param loop true to loop indefinitely; false to play once and stop.
     */
    void Play(const std::string &name, bool loop = true);

private:
    /// Linear interpolation between the two surrounding vec3 keyframes.
    glm::vec3 Interpolate(const std::vector<KeyFrameVec3> &keys, float time);

    /// Spherical linear interpolation (slerp) between surrounding quat keyframes.
    glm::quat Interpolate(const std::vector<KeyFrameQuat> &keys, float time);

    /**
     * @brief Walk the owner's child hierarchy and cache live object pointers for each track.
     *
     * Called automatically by Play(). Matches TransformTrack::targetName against
     * child GameObject names using FindChildByName().
     */
    void BuildBindings();

private:
    AnimationClip *m_clip      = nullptr; ///< Active clip (non-owning); may be nullptr.
    float          m_time      = 0.0f;    ///< Current playback position in seconds.
    bool           m_looping   = true;    ///< Whether to wrap at the end of the clip.
    bool           m_isPlaying = false;   ///< False until Play() is called.

    std::unordered_map<std::string, std::shared_ptr<AnimationClip>>  m_clips;    ///< Registered clips by name.
    std::unordered_map<GameObject *, std::unique_ptr<ObjectBinding>> m_bindings; ///< Track-to-object bindings built at play time.
};

}  // namespace ENG
