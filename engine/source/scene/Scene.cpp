#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "scene/Scene.h"

#include "Common.h"
#include "Log.h"
#include "scene/GameObject.h"
#include "scene/components/LightComponent.h"

namespace COA
{

void Scene::Update(f32 deltaTime)
{
    for (auto it = m_objects.begin(); it != m_objects.end();)
    {
        if ((*it)->IsAlive())
        {
            (*it)->Update(deltaTime);
            ++it;
        } else
        {
            it = m_objects.erase(it);
        }
    }
}

void Scene::SetMainCamera(GameObject *camera)
{
    m_mainCamera = camera;
}

GameObject *Scene::GetMainCamera()
{
    return m_mainCamera;
}

std::vector<LightData> Scene::CollectLight()
{
    std::vector<LightData> lights;
    for (auto &obj : m_objects)
    {
        CollectLightsRecursive(obj.get(), lights);
    }
    return lights;
}

void Scene::CollectLightsRecursive(GameObject *obj, std::vector<LightData> &out)
{
    if (auto light = obj->GetComponent<LightComponent>())
    {
        LightData data;
        data.color    = light->GetColor();
        data.position = obj->GetWorldPosition();
        out.push_back(data);
    }

    for (auto &child : obj->m_children)
    {
        CollectLightsRecursive(child.get(), out);
    }
}

void Scene::Clear()
{
    LOG_INFO("Scene::Clear (%zu root objects)", m_objects.size());
    m_objects.clear();
}

GameObject *Scene::CreateObject(const std::string &name, GameObject *parent)
{
    auto obj = new GameObject();
    obj->SetName(name);
    obj->m_scene = this;
    SetParent(obj, parent);
    LOG_INFO("Created GameObject '%s' (parent=%s)", name.c_str(), parent ? parent->GetName().c_str() : "<root>");
    return obj;
}

bool Scene::SetParent(GameObject *obj, GameObject *parent)
{
    if (!obj)
    {
        LOG_ERROR("Scene::SetParent called with null object");
        return false;
    }
    bool result        = false;
    auto currentParent = obj->GetParent();

    if (parent == nullptr)
    {
        if (currentParent != nullptr)
        {
            auto it = std::find_if(currentParent->m_children.begin(),
                                   currentParent->m_children.end(),
                                   [obj](const std::unique_ptr<GameObject> &el) { return el.get() == obj; });

            if (it != currentParent->m_children.end())
            {
                m_objects.push_back(std::move(*it));
                obj->m_parent = nullptr;
                currentParent->m_children.erase(it);
                result = true;
            }
        }

        // No parent currently. This can be in 2 cases.
        // 1. The object is in the scene root.
        // 2. The object has been just created.
        else
        {
            auto it = std::find_if(m_objects.begin(),
                                   m_objects.end(),
                                   [obj](const std::unique_ptr<GameObject> &el) { return el.get() == obj; });

            if (it == m_objects.end())
            {
                std::unique_ptr<GameObject> objHolder(obj);
                m_objects.push_back(std::move(objHolder));

                result = true;
            }
        }
    }
    // We are trying to add it as a child of another object

    else
    {
        if (currentParent != nullptr)
        {
            auto it = std::find_if(m_objects.begin(),
                                   m_objects.end(),
                                   [obj](const std::unique_ptr<GameObject> &el) { return el.get() == obj; });

            if (it != currentParent->m_children.end())
            {
                bool found          = false;
                auto currentElement = parent;
                while (currentParent)
                {
                    if (currentElement == obj)
                    {
                        found = true;
                        break;
                    }
                    currentElement = currentElement->GetParent();
                }

                if (!found)
                {
                    parent->m_children.push_back(std::move(*it));
                    obj->m_parent = parent;
                    currentParent->m_children.erase(it);
                    result = true;
                }
            }
        }

        // No parent currently. This can be in 2 cases.
        // 1. The object is in the scene root.
        // 2. The object has been just created.
        else
        {
            auto it = std::find_if(m_objects.begin(),
                                   m_objects.end(),
                                   [obj](const std::unique_ptr<GameObject> &el) { return el.get() == obj; });

            // The object has been just created
            if (it == m_objects.end())
            {
                std::unique_ptr<GameObject> objHolder(obj);
                parent->m_children.push_back(std::move(objHolder));
                obj->m_parent = parent;

                result = true;
            } else
            {
                bool found          = false;
                auto currentElement = parent;
                while (currentParent)
                {
                    if (currentElement == obj)
                    {
                        found = true;
                        break;
                    }
                    currentElement = currentElement->GetParent();
                }
                if (!found)
                {
                    parent->m_children.push_back(std::move(*it));
                    obj->m_parent = parent;
                    m_objects.erase(it);

                    result = true;
                }
            }
        }
    }

    if (!result)
    {
        LOG_WARN("Scene::SetParent failed for object '%s' (target parent=%s)",
                 obj->GetName().c_str(),
                 parent ? parent->GetName().c_str() : "<root>");
    }

    return result;
}
}  // namespace COA
