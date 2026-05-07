#pragma once
#include "scene/Component.h"
#include <string>
#include <vector>
#include <memory>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>

namespace eng
{
    class Scene;

    class GameObject
    {
    public:
        virtual ~GameObject() = default;
        virtual void Init();
        virtual void LoadProperties(const nlohmann::json& json);
        virtual void Update(float deltaTime);
        const std::string& GetName() const;
        void SetName(const std::string& name);
        GameObject* GetParent();
        bool SetParent(GameObject* parent);
        Scene* GetScene();
        bool IsAlive() const;
        void MarkForDestroy();

        void SetActive(bool active);
        bool IsActive() const;

        void AddComponent(Component* component);
        template<typename T, typename = typename std::enable_if_t<std::is_base_of_v<Component, T>>>
        T* GetComponent()
        {
            size_t typeId = Component::StaticTypeId<T>();

            for (auto& component : m_components)
            {
                if (component->GetTypeId() == typeId)
                {
                    return static_cast<T*>(component.get());
                }
            }

            return nullptr;
        }

        GameObject* FindChildByName(const std::string& name);

        const glm::vec3& GetPosition() const;
        glm::vec3 GetWorldPosition() const;
        glm::vec2 GetPosition2D() const;
        glm::vec2 GetWorldPosition2D() const;
        void SetPosition(const glm::vec3& pos);
        void SetWorldPosition(const glm::vec3& pos);
        void SetPosition2D(const glm::vec2& pos);

        const glm::quat& GetRotation() const;
        glm::quat GetWorldRotation();
        float GetRotation2D() const;
        void SetRotation(const glm::quat& rot);
        void SetWorldRotation(const glm::quat& rot);
        void SetRotation2D(float rotation);

        const glm::vec3& GetScale() const;
        glm::vec2 GetScale2D() const;
        void SetScale(const glm::vec3& scale);
        void SetScale2D(const glm::vec2& scale);

        glm::mat4 GetLocalTransform() const;
        glm::mat4 GetLocalTransform2D() const;
        glm::mat4 GetWorldTransform() const;
        glm::mat4 GetWorldTransform2D() const;

        static GameObject* LoadGLTF(const std::string& path, Scene* scene);

    protected:
        GameObject() = default;

    protected:
        std::string m_name;
        GameObject* m_parent = nullptr;
        Scene* m_scene = nullptr;
        std::vector<std::unique_ptr<GameObject>> m_children;
        std::vector<std::unique_ptr<Component>> m_components;
        bool m_isAlive = true;
        glm::vec3 m_position = glm::vec3(0.0f);
        glm::quat m_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 m_scale = glm::vec3(1.0f);
        bool m_active = true;

        friend class Scene;
    };

    class ObjectCreatorBase
    {
    public:
        virtual ~ObjectCreatorBase() = default;
        virtual GameObject* CreateGameObject() = 0;
    };

    template<typename T>
    class ObjectCreator : public ObjectCreatorBase
    {
    public:
        virtual GameObject* CreateGameObject() override
        {
            return new T();
        }
    };

    class GameObjectFactory
    {
    public:
        static GameObjectFactory& GetInstance();
        template<typename T>
        void RegisterObject(const std::string& name)
        {
            m_creators.emplace(name, std::make_unique<ObjectCreator<T>>());
        }

        GameObject* CreateGameObject(const std::string& typeName);

    private:
        std::unordered_map<std::string, std::unique_ptr<ObjectCreatorBase>> m_creators;
    };

#define GAMEOBJECT(ObjectClass) \
public: \
    static void Register() { eng::GameObjectFactory::GetInstance().RegisterObject<ObjectClass>(std::string(#ObjectClass)); }
}