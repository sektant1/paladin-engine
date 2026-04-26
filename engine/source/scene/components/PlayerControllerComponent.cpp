#include "scene/components/PlayerControllerComponent.h"

#include <algorithm>
#include <cmath>

#include "Constants.h"
#include "Engine.h"
#include "GLFW/glfw3.h"
#include "Types.h"
#include "editor/Editor.h"
#include "input/InputManager.h"

namespace mnd
{

namespace
{
constexpr f32 kMinHorizontalSpeed = 1e-4F;

vec3 FlattenY(const vec3 &v)
{
    return vec3(v.x, 0.0F, v.z);
}
}  // namespace

void PlayerControllerComponent::Init()
{
    m_kinematicController = std::make_unique<KinematicCharacterController>(
        kDefaultCapsuleRadius, kDefaultCapsuleHeight, m_owner->GetWorldPosition());
}

void PlayerControllerComponent::ApplyFriction(f32 deltaTime)
{
    f32 speed = glm::length(FlattenY(m_velocity));
    if (speed < kMinHorizontalSpeed)
    {
        m_velocity.x = 0.0F;
        m_velocity.z = 0.0F;
        return;
    }
    f32 control  = (speed < kQuakeStopSpeed) ? kQuakeStopSpeed : speed;
    f32 drop     = control * m_friction * deltaTime;
    f32 newSpeed = std::max(0.0F, speed - drop);
    f32 scale    = newSpeed / speed;
    m_velocity.x *= scale;
    m_velocity.z *= scale;
}

void PlayerControllerComponent::Accelerate(const vec3 &wishDir, f32 wishSpeed, f32 accel, f32 deltaTime)
{
    f32 currentSpeed = glm::dot(m_velocity, wishDir);
    f32 addSpeed     = wishSpeed - currentSpeed;
    if (addSpeed <= 0.0F)
    {
        return;
    }
    f32 accelSpeed = accel * deltaTime * wishSpeed;
    if (accelSpeed > addSpeed)
    {
        accelSpeed = addSpeed;
    }
    m_velocity += accelSpeed * wishDir;
}

void PlayerControllerComponent::Update(f32 deltaTime)
{
    auto      &inputManager = Engine::GetInstance().GetInputManager();
    const bool editorOpen   = Engine::GetInstance().GetEditor().IsVisible();

    auto rotation = m_owner->GetRotation();

    if (!editorOpen && inputManager.IsMousePositionChanged())
    {
        const auto &oldPos     = inputManager.GetMousePositionOld();
        const auto &currentPos = inputManager.GetMousePositionCurrent();

        f32 deltaX = currentPos.x - oldPos.x;
        f32 deltaY = currentPos.y - oldPos.y;

        // Mouse-look is frame-rate independent (HL/Source convention): each
        // mouse count = fixed angle, no deltaTime scaling.
        f32 yDeltaAngle = -deltaX * m_sensitivity;
        m_yRot += yDeltaAngle;
        glm::quat yRot = glm::angleAxis(glm::radians(m_yRot), glm::vec3(0, 1, 0));

        f32 xDeltaAngle = -deltaY * m_sensitivity;
        m_xRot += xDeltaAngle;
        m_xRot         = std::clamp(m_xRot, -kPitchLimitDegrees, kPitchLimitDegrees);
        glm::quat xRot = glm::angleAxis(glm::radians(m_xRot), glm::vec3(1, 0, 0));

        rotation = glm::normalize(yRot * xRot);
        m_owner->SetRotation(rotation);
    }

    if (editorOpen)
    {
        m_velocity = vec3(0.0F);
        m_kinematicController->Walk(vec3(0.0F));
        m_owner->SetPosition(m_kinematicController->GetPosition());
        return;
    }

    // Build horizontal-plane basis from yaw only — pitch shouldn't tilt motion.
    vec3 front = glm::normalize(FlattenY(rotation * vec3(0.0F, 0.0F, -1.0F)));
    vec3 right = glm::normalize(FlattenY(rotation * vec3(1.0F, 0.0F, 0.0F)));

    vec3 wish(0.0F);
    if (inputManager.IsKeyPressed(Key::A))
    {
        wish -= right;
    } else if (inputManager.IsKeyPressed(Key::D))
    {
        wish += right;
    }
    if (inputManager.IsKeyPressed(Key::W))
    {
        wish += front;
    } else if (inputManager.IsKeyPressed(Key::S))
    {
        wish -= front;
    }

    vec3 wishDir(0.0F);
    f32  wishMag = glm::length(wish);
    if (wishMag > kMinHorizontalSpeed)
    {
        wishDir = wish / wishMag;
    }

    const bool onGround    = m_kinematicController->OnGround();
    const bool jumpPressed = inputManager.IsKeyPressed(Key::Space);

    if (onGround)
    {
        ApplyFriction(deltaTime);
        Accelerate(wishDir, m_moveSpeed, m_groundAccel, deltaTime);
    } else
    {
        // Air control: cap wishspeed so high-speed strafe-jumping stays possible
        // while still letting the player nudge direction in the air.
        f32 airWishSpeed = std::min(m_moveSpeed, m_airCap);
        Accelerate(wishDir, airWishSpeed, m_airAccel, deltaTime);
    }

    // Horizontal motion: pass per-frame displacement to the kinematic controller.
    m_kinematicController->Walk(vec3(m_velocity.x * deltaTime, 0.0F, m_velocity.z * deltaTime));

    // Bunny-hop: with auto-hop, jump fires every frame Space is held while on
    // ground (so you bhop just by holding it). Without it, you have to retap.
    if (onGround && jumpPressed && (m_autoHop || !m_jumpHeldLast))
    {
        m_kinematicController->Jump(vec3(0.0F, m_moveSpeed * m_jumpSpeed, 0.0F));
    }
    m_jumpHeldLast = jumpPressed;

    m_owner->SetPosition(m_kinematicController->GetPosition());
}

bool PlayerControllerComponent::OnGround() const
{
    if (m_kinematicController)
    {
        return m_kinematicController->OnGround();
    }
    return false;
}

void PlayerControllerComponent::SetMoveSpeed(f32 speed)
{
    m_moveSpeed = speed;
}

f32 PlayerControllerComponent::GetMoveSpeed() const
{
    return m_moveSpeed;
}

}  // namespace mnd
