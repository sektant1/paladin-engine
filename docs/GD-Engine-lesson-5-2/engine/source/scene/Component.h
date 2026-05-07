#pragma once

#include <nlohmann/json.hpp>

#include <cstddef>
#include <string>
#include <unordered_map>
#include <memory>

namespace eng
{
    class GameObject;

    class Component
    {
    public:
        virtual ~Component() = default;
        virtual void LoadProperties(const nlohmann::json& json);
        virtual void Update(float deltaTime) = 0;
        virtual void Init();
        virtual size_t GetTypeId() const = 0;

        GameObject* GetOwner();

        template<typename T>
        static size_t StaticTypeId()
        {
            static size_t typeId = nextId++;
            return typeId;
        }

    protected:
        GameObject* m_owner = nullptr;

        friend class GameObject;

    private:
        static size_t nextId;
    };

    class ComponentCreatorBase
    {
    public:
        virtual ~ComponentCreatorBase() = default;
        virtual Component* CreateComponent() = 0;
    };

    template<typename T>
    class ComponentCreator : public ComponentCreatorBase
    {
    public:
        Component* CreateComponent() override
        {
            return new T();
        }
    };

    class ComponentFactory
    {
    public:
        static ComponentFactory& GetInstance();

        template<typename T>
        void RegisterComponent(const std::string& name)
        {
            m_creators.emplace(name, std::make_unique<ComponentCreator<T>>());
        }

        Component* CreateComponent(const std::string& name);

    private:
        std::unordered_map<std::string, std::unique_ptr<ComponentCreatorBase>> m_creators;
    };

#define COMPONENT(ComponentClass) \
public: \
    static size_t TypeId() { return eng::Component::StaticTypeId<ComponentClass>(); } \
    size_t GetTypeId() const override { return TypeId(); } \
    static void Register() { eng::ComponentFactory::GetInstance().RegisterComponent<ComponentClass>(std::string(#ComponentClass)); }
}