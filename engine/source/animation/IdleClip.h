/**
 * @file IdleClip.h
 * @ingroup mnd_animation
 * @brief Procedural skeletal idle clip — used when an asset ships rigged but
 *        without baked animations. Produces a gentle breathing + sway loop on
 *        common humanoid bone names (shoulders, biceps, forearms, wrists).
 */

#pragma once

#include <memory>

#include "Types.h"

namespace mnd
{

class Skeleton;
struct SkeletalAnimationClip;

/**
 * @brief Build a looping breathing/sway clip targeting whichever of the
 *        canonical arm bones exist in `skeleton`. Bones it doesn't find are
 *        silently skipped, so this works for full bodies, FPS arms-only rigs,
 *        and partial rigs alike.
 *
 * @param skeleton Source skeleton; bind-pose decomposition is read for fallback.
 * @param duration Clip length in seconds (default 4 — a slow breath cycle).
 * @return Shared SkeletalAnimationClip tagged with name "idle".
 */
std::shared_ptr<SkeletalAnimationClip> MakeBreathingIdleClip(const Skeleton &skeleton,
                                                             f32             duration = 4.0f,
                                                             f32             ampScale = 1.0f);

}  // namespace mnd
