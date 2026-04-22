/**
 * @file Component.h
 * @brief Abstract base for all GameObject components and the COMPONENT macro.
 *
 * ## Entity-Component pattern
 * A GameObject is a named scene node with a transform. Behaviour and
 * rendering are added by attaching Component subclasses. Each component
 * type gets a unique numeric ID at runtime so that typed lookup via
 * GameObject::GetComponent<T>() works in O(n) without RTTI.
 *
 * ## Adding a new component type
 * 1. Subclass Component.
 * 2. Place the COMPONENT(MyComponent) macro at the top of the public section.
 * 3. Implement Update(f32 deltaTime).
 * 4. Attach via GameObject::AddComponent(new MyComponent(...)).
 *
 * @code
 *   class SpinComponent : public ENG::Component {
 *       COMPONENT(SpinComponent)
 *   public:
 *       void Update(ENG::f32 dt) override { m_owner->SetRotation(...); }
 *   };
 * @endcode
 *
 * @see GameObject::AddComponent, GameObject::GetComponent, COMPONENT
 */

#pragma once

#include "Types.h"

namespace ENG
{

class GameObject;

/**
 * @brief Abstract base class for all attachable behaviours.
 *
 * Each concrete subclass gets a unique static type ID via StaticTypeId<T>(),
 * which allows efficient component lookup without dynamic_cast.
 *
 * The engine calls Update() once per frame on every component of every
 * active GameObject in the current Scene.
 */
class Component
{
public:
    virtual ~Component() = default;

    /**
     * @brief Called once per frame by the owning GameObject.
     * @param deltaTime Seconds since the previous frame.
     */
    virtual void Update(f32 deltaTime) = 0;

    /**
     * @brief Returns the runtime type ID for this component instance.
     *        Implemented automatically by the COMPONENT macro.
     */
    [[nodiscard]] virtual usize GetTypeId() const = 0;

    /// Returns the GameObject this component is attached to.
    GameObject *GetOwner();

    /**
     * @brief Returns the unique type ID for component type T.
     *
     * IDs are assigned sequentially on first call for each type and remain
     * stable for the lifetime of the process. Used by GameObject::GetComponent<T>().
     */
    template<typename T>
    static usize StaticTypeId()
    {
        static usize typeId = nextId++;
        return typeId;
    }

protected:
    /// Back-pointer to the owning GameObject, set by GameObject::AddComponent().
    GameObject *m_owner = nullptr;

    friend class GameObject;

private:
    static usize nextId;  ///< Global counter for assigning unique type IDs.
};

// clang-format off
/**
 * @brief Boilerplate macro that wires up the runtime type ID for a component.
 *
 * Place at the very top of the public section of any Component subclass.
 * Provides:
 * - `static usize TypeId()` — class-level type ID.
 * - `usize GetTypeId() const override` — virtual override.
 *
 * @param ComponentClass  The concrete component class name.
 */
#define COMPONENT(ComponentClass) \
public: \
static usize TypeId() { return Component::StaticTypeId<ComponentClass>(); } \
usize GetTypeId() const override {return TypeId();} \


}  // namespace ENG
