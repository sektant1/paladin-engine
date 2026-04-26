/**
 * @file Constants.h
 * @ingroup mnd_types
 * @brief Engine-wide compile-time tunables and ANSI colour codes.
 *
 * Defaults that several subsystems read at startup live here so you can
 * adjust window size, gravity, internal-resolution presets, log buffer
 * limits and a few other knobs in one place. All values are
 * `inline constexpr` to give every translation unit zero-cost access
 * without ODR worries.
 *
 * @code
 *   engine.Init(kDefaultWindowWidth, kDefaultWindowHeight);
 *   physics.SetGravity({0.0F, kGravity, 0.0F});
 * @endcode
 */

#pragma once

#include <cstddef>

#include "Types.h"

// -- Window / display --------------------------------------------------------
inline constexpr mnd::i32    kDefaultWindowWidth  = 1280;
inline constexpr mnd::i32    kDefaultWindowHeight = 720;
inline constexpr const char *kDefaultWindowTitle  = "GL Game Engine";

// -- Logging -----------------------------------------------------------------
inline constexpr std::size_t kMaxLogEntries     = 1024;
inline constexpr std::size_t kLogMessageBufSize = 1024;
inline constexpr std::size_t kLogFullBufSize    = 1280;
inline constexpr mnd::i32    kLogPrefixBufSize  = 256;

// ANSI color escape codes
inline constexpr const char *kAnsiReset = "\033[0m";
inline constexpr const char *kAnsiInfo  = "\033[32m";
inline constexpr const char *kAnsiWarn  = "\033[33m";
inline constexpr const char *kAnsiError = "\033[31m";
inline constexpr const char *kAnsiFatal = "\033[1;31m";

// -- Renderer ----------------------------------------------------------------
inline constexpr mnd::i32 kDefaultInternalWidth  = 640;
inline constexpr mnd::i32 kDefaultInternalHeight = 480;
inline constexpr mnd::f32 kDefaultAmbient        = 0.35F;

inline constexpr mnd::i32 kInternalPresetTinyW = 160;
inline constexpr mnd::i32 kInternalPresetTinyH = 120;
inline constexpr mnd::i32 kInternalPresetLowW  = 320;
inline constexpr mnd::i32 kInternalPresetLowH  = 240;
inline constexpr mnd::i32 kInternalPresetMedW  = 640;
inline constexpr mnd::i32 kInternalPresetMedH  = 480;
inline constexpr mnd::i32 kInternalResMin      = 16;
inline constexpr mnd::i32 kInternalResMax      = 4096;

inline constexpr const char *kGlslVersionDirective = "#version 330 core";
inline constexpr mnd::i32    kShaderInfoLogSize    = 512;

// Mesh layout: pos(3) + normal(3) + color(3) + uv(2) = 11 floats
inline constexpr mnd::i32 kStandardVertexFloats = 11;
inline constexpr mnd::f32 kPI                   = 3.14159265358979323846f;
// -- Input -------------------------------------------------------------------
inline constexpr mnd::i32 kMaxKeys         = 512;
inline constexpr mnd::i32 kMaxMouseButtons = 16;

// -- Physics -----------------------------------------------------------------
inline constexpr mnd::f32 kGravity              = -9.81F;
inline constexpr mnd::f32 kDefaultCapsuleRadius = 0.4F;
inline constexpr mnd::f32 kDefaultCapsuleHeight = 1.2F;
inline constexpr mnd::f32 kDefaultStepHeight    = 0.35F;

// -- Camera / controller -----------------------------------------------------
inline constexpr mnd::f32 kPitchLimitDegrees       = 89.0F;
inline constexpr mnd::f32 kCameraFov               = 60.0F;
inline constexpr mnd::f32 kCameraNearPlane         = 0.01F;
inline constexpr mnd::f32 kCameraFarPlane          = 1000.0F;
// Mouse-look: degrees per pixel of mouse delta. Raw (no deltaTime scaling)
// matches GoldSrc/Source — each mouse count maps to a fixed angle regardless
// of frame rate.
inline constexpr mnd::f32 kDefaultMouseSensitivity = 0.15F;
// Top run speed: HL1 sv_maxspeed=320 ups ≈ 8.13 m/s. 7.5 m/s feels close
// while reading better at 1u≈1m scenes.
inline constexpr mnd::f32 kDefaultMoveSpeed = 7.5F;
// Jump impulse multiplier on moveSpeed. HL1 jump = 268 ups ≈ 6.8 m/s,
// so 7.5 * 0.9 ≈ 6.75 m/s. Reaches ~2.3 m at gravity 9.81.
inline constexpr mnd::f32 kDefaultJumpSpeed = 0.9F;

// -- HL/GoldSrc-style movement -----------------------------------------------
// Tuned to mirror sv_accelerate / sv_airaccelerate / sv_friction / sv_stopspeed.
inline constexpr mnd::f32 kQuakeGroundAccel = 10.0F;  ///< sv_accelerate.
inline constexpr mnd::f32 kQuakeAirAccel    = 10.0F;  ///< sv_airaccelerate (HL1 = 10; Q3 = 1).
inline constexpr mnd::f32 kQuakeFriction    = 4.0F;   ///< sv_friction.
inline constexpr mnd::f32 kQuakeStopSpeed   = 2.5F;   ///< sv_stopspeed (100 ups).
// Per-axis wishspeed cap mid-air. HL has no cap → set above any reasonable
// moveSpeed so the binding cap becomes moveSpeed itself. Lower it (≈ 0.76)
// for CPMA/Q3-style strafe-jumping.
inline constexpr mnd::f32 kQuakeAirCap = 30.0F;

// -- Simulation --------------------------------------------------------------
inline constexpr mnd::f32 kFixedTimeStep = 1.0F / 60.0F;
inline constexpr mnd::i32 kMaxSubSteps   = 4;

// -- Asset paths -------------------------------------------------------------
inline constexpr const char *kAssetsDirName = "assets";

// -- Scene JSON keys ---------------------------------------------------------
inline constexpr const char *kJsonKeyName       = "name";
inline constexpr const char *kJsonKeyType       = "type";
inline constexpr const char *kJsonKeyPath       = "path";
inline constexpr const char *kJsonKeyPosition   = "position";
inline constexpr const char *kJsonKeyRotation   = "rotation";
inline constexpr const char *kJsonKeyScale      = "scale";
inline constexpr const char *kJsonKeyComponents = "components";
inline constexpr const char *kJsonKeyChildren   = "children";
inline constexpr const char *kJsonKeyObjects    = "objects";
inline constexpr const char *kJsonKeyCamera     = "camera";
inline constexpr const char *kJsonKeyX          = "x";
inline constexpr const char *kJsonKeyY          = "y";
inline constexpr const char *kJsonKeyZ          = "z";
inline constexpr const char *kJsonKeyW          = "w";

inline constexpr const char *kSceneTypeGltf     = "gltf";
inline constexpr const char *kDefaultObjectName = "Object";
inline constexpr const char *kDefaultSceneName  = "noname";
inline constexpr const char *kRootObjectLabel   = "";
