#pragma once

#include "Types.h"

// -- Window ------------------------------------------------------------------
inline constexpr COA::i32 kWindowWidth  = 1280;
inline constexpr COA::i32 kWindowHeight = 720;

// -- Scene -------------------------------------------------------------------
inline constexpr const char *kInitialScenePath = "scenes/scene.json";

// -- Player ------------------------------------------------------------------
inline constexpr COA::f32 kPlayerWalkSpeed = 5.0F;
inline constexpr COA::f32 kPlayerRunSpeed  = 10.0F;

// Child object names looked up at Player::Init
inline constexpr const char *kChildBulletName = "bullet_33";
inline constexpr const char *kChildBoomName   = "BOOM_35";
inline constexpr const char *kChildGunName    = "Gun";

// Audio / animation cue names
inline constexpr const char *kSfxShoot  = "shoot";
inline constexpr const char *kSfxJump   = "jump";
inline constexpr const char *kSfxStep   = "step";
inline constexpr const char *kAnimShoot = "shoot";

// -- Bullet ------------------------------------------------------------------
inline constexpr COA::f32 kBulletLifetime = 2.0F;

// -- LabObject ---------------------------------------------------------------
inline constexpr COA::f32 kLabDefaultTimeScale = 0.3F;
inline constexpr COA::f32 kLabTimeScale1       = 0.25F;
inline constexpr COA::f32 kLabTimeScale2       = 0.5F;
inline constexpr COA::f32 kLabTimeScale3       = 1.0F;
inline constexpr COA::f32 kLabTimeScale4       = 1.5F;
inline constexpr COA::f32 kLabTimeScale5       = -1.5F;

// Shader uniforms (Shadertoy-style)
inline constexpr const char *kUniformResolution = "iResolution";
inline constexpr const char *kUniformTime       = "iTime";
inline constexpr const char *kUniformMouse      = "iMouse";

// -- Shader asset paths ------------------------------------------------------
inline constexpr const char *kLabVertShaderPath   = "assets/shaders/lab.vert";
inline constexpr const char *kLabFragShaderPath   = "assets/shaders/lab.frag";
inline constexpr const char *kCoolVertShaderPath  = "assets/shaders/cool.vert";
inline constexpr const char *kGreenFragShaderPath = "assets/shaders/green.frag";
