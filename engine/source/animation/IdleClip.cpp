#include "animation/IdleClip.h"

#include <cmath>
#include <vector>

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

#include "animation/SkeletalAnimationClip.h"
#include "animation/Skeleton.h"

namespace mnd
{

namespace
{

constexpr u32 kSamplesPerSecond = 30;  ///< Sampling density. 30 Hz is plenty for a slow idle.

struct IdleSpec
{
    const char *boneName;
    glm::vec3   axis;        ///< Rotation axis in bone-local space.
    f32         amplitudeRad; ///< Peak rotation in radians.
    f32         phaseOffset;  ///< Phase shift in cycles ([0,1)) — desyncs left/right for organic feel.
};

/// Canonical FPS-arms bones we know how to drive. Each entry is opt-in: if the
/// bone is missing in the skeleton we silently skip it. Amplitudes are tuned
/// small (1–2.5°) to read as "alive" without looking jittery.
const IdleSpec kIdleSpecs[] = {
    // Shoulders breathe in/out around X (lift/drop).
    {"shoulder.r", glm::vec3(1, 0, 0),  0.025f, 0.00f},
    {"shoulder.l", glm::vec3(1, 0, 0),  0.025f, 0.05f},
    // Biceps follow with a tiny counter-rotation.
    {"bicep.r",    glm::vec3(0, 0, 1), -0.018f, 0.07f},
    {"bicep.l",    glm::vec3(0, 0, 1),  0.018f, 0.12f},
    // Forearms get a subtle yaw (weapon hand sway).
    {"forearm.r",  glm::vec3(0, 1, 0),  0.022f, 0.10f},
    {"forearm.l",  glm::vec3(0, 1, 0), -0.022f, 0.15f},
    // Wrists add the highest-frequency wobble — half the period for life.
    {"wrist.r",    glm::vec3(1, 0, 0),  0.030f, 0.20f},
    {"wrist.l",    glm::vec3(1, 0, 0),  0.030f, 0.25f},
    // Whole rig root: tiny pitch giving the camera-relative bob.
    {"root",       glm::vec3(1, 0, 0),  0.012f, 0.00f},
};

}  // namespace

std::shared_ptr<SkeletalAnimationClip> MakeBreathingIdleClip(const Skeleton &skeleton, f32 duration, f32 ampScale)
{
    auto clip      = std::make_shared<SkeletalAnimationClip>();
    clip->name     = "idle";
    clip->duration = duration;
    clip->looping  = true;

    if (duration <= 0.0f || skeleton.Size() == 0)
    {
        return clip;
    }

    const u32 sampleCount = static_cast<u32>(duration * kSamplesPerSecond) + 1;

    for (const auto &spec : kIdleSpecs)
    {
        i32 boneIx = skeleton.FindBoneIndex(spec.boneName);
        if (boneIx < 0)
        {
            continue;
        }

        const auto &bone = skeleton.GetBones()[boneIx];

        BoneTrack track;
        track.boneIndex = boneIx;
        track.rotations.reserve(sampleCount);

        for (u32 s = 0; s < sampleCount; ++s)
        {
            f32 t = static_cast<f32>(s) / static_cast<f32>(kSamplesPerSecond);
            // Two phases: shoulders/biceps breathe at 1× duration, wrists at 2× — looks alive.
            f32 cyclesPerLoop = 1.0f;
            f32 phase         = (t / duration + spec.phaseOffset) * cyclesPerLoop;
            f32 angle         = spec.amplitudeRad * ampScale * std::sin(phase * 6.283185307f);

            glm::quat anim = glm::angleAxis(angle, glm::normalize(spec.axis));
            // Compose with bind rotation so shoulders etc. keep their rest orientation.
            glm::quat finalRot = bone.bindRot * anim;

            track.rotations.push_back({t, finalRot});
        }

        clip->tracks.push_back(std::move(track));
    }

    return clip;
}

}  // namespace mnd
