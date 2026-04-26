#include <algorithm>
#include <cstdio>
#include <memory>
#include <utility>
#include <vector>

#include "scene/Scene.h"

#include "Common.h"
#include "Engine.h"
#include "Log.h"
#include "io/ModelImporter.h"
#include "scene/GameObject.h"
#include "scene/components/AnimationComponent.h"
#include "scene/components/AudioComponent.h"
#include "scene/components/AudioListenerComponent.h"
#include "scene/components/CameraComponent.h"
#include "scene/components/LightComponent.h"
#include "scene/components/MeshComponent.h"
#include "scene/components/SkinnedMeshComponent.h"
#include "scene/components/PhysicsComponent.h"
#include "scene/components/PlayerControllerComponent.h"

namespace mnd
{

void Scene::RegisterTypes()
{
    AnimationComponent::Register();
    CameraComponent::Register();
    LightComponent::Register();
    MeshComponent::Register();
    SkinnedMeshComponent::Register();
    PhysicsComponent::Register();
    PlayerControllerComponent::Register();
    AudioComponent::Register();
    AudioListenerComponent::Register();
}

void Scene::Update(f32 deltaTime)
{
    m_objects.erase(std::remove_if(m_objects.begin(), m_objects.end(), [](auto &obj) { return !obj->IsAlive(); }),
                    m_objects.end());

    for (auto &obj : m_objectsToAdd)
    {
        SetParent(obj.first, obj.second);
    }
    m_objectsToAdd.clear();

    m_isUpdating = true;

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

    m_isUpdating = false;
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

GameObject *Scene::CreateObject(const std::string &type, const std::string &name, GameObject *parent)
{
    GameObject *obj = GameObjectFactory::GetInstance().CreateGameObject(type);
    if (obj)
    {
        obj->SetName(name);
        obj->m_scene = this;
        if (m_isUpdating)
        {
            m_objectsToAdd.push_back({obj, parent});
        } else
        {
            SetParent(obj, parent);
        }
    }
    return obj;
}

GameObject *Scene::CreateObject(const std::string &name, GameObject *parent)
{
    auto obj = new GameObject();
    obj->SetName(name);
    obj->m_scene = this;
    if (m_isUpdating)
    {
        m_objectsToAdd.push_back({obj, parent});
    } else
    {
        SetParent(obj, parent);
    }
    LOG_INFO(
        "Created GameObject '%s' (parent=%s)", name.c_str(), parent ? parent->GetName().c_str() : kRootObjectLabel);
    return obj;
}

void Scene::LoadObject(const nlohmann::json &jsonObject, GameObject *parent)
{
    const str name = jsonObject.value(kJsonKeyName, kDefaultObjectName);

    GameObject *gameObject = nullptr;

    if (jsonObject.contains(kJsonKeyType))
    {
        const std::string type = jsonObject.value(kJsonKeyType, "");
        if (type == kSceneTypeGltf)
        {
            std::string path = jsonObject.value(kJsonKeyPath, "");
            gameObject       = GameObject::LoadGLTF(path, this);
            if (gameObject)
            {
                gameObject->SetParent(parent);
                gameObject->SetName(name);
            } else
            {
                LOG_ERROR("Scene::LoadObject failed to load GLTF '%s' for object '%s'", path.c_str(), name.c_str());
            }
        } else if (type == kSceneTypeFbx || type == kSceneTypeModel)
        {
            std::string path = jsonObject.value(kJsonKeyPath, "");
            gameObject       = ModelImporter::Import(path, this);
            if (gameObject)
            {
                gameObject->SetParent(parent);
                gameObject->SetName(name);
            } else
            {
                LOG_ERROR("Scene::LoadObject failed to import model '%s' for object '%s'", path.c_str(), name.c_str());
            }
        } else
        {
            gameObject = CreateObject(type, name, parent);
            if (!gameObject)
            {
                LOG_ERROR("Scene::LoadObject unknown GameObject type '%s' (name='%s')", type.c_str(), name.c_str());
            }
        }
    } else
    {
        gameObject = CreateObject(name, parent);
    }

    if (!gameObject)
    {
        return;
    }

    // Read transform
    if (jsonObject.contains(kJsonKeyPosition))
    {
        auto      posObj = jsonObject[kJsonKeyPosition];
        glm::vec3 pos;
        pos.x = posObj.value(kJsonKeyX, 0.0f);
        pos.y = posObj.value(kJsonKeyY, 0.0f);
        pos.z = posObj.value(kJsonKeyZ, 0.0f);
        gameObject->SetPosition(pos);
    }

    if (jsonObject.contains(kJsonKeyRotation))
    {
        auto      rotObj = jsonObject[kJsonKeyRotation];
        glm::quat rot;
        rot.x = rotObj.value(kJsonKeyX, 0.0f);
        rot.y = rotObj.value(kJsonKeyY, 0.0f);
        rot.z = rotObj.value(kJsonKeyZ, 0.0f);
        rot.w = rotObj.value(kJsonKeyW, 1.0f);
        gameObject->SetRotation(rot);
    }

    if (jsonObject.contains(kJsonKeyScale))
    {
        auto      scaleObj = jsonObject[kJsonKeyScale];
        glm::vec3 scale;
        scale.x = scaleObj.value(kJsonKeyX, 1.0f);
        scale.y = scaleObj.value(kJsonKeyY, 1.0f);
        scale.z = scaleObj.value(kJsonKeyZ, 1.0f);
        gameObject->SetScale(scale);
    }

    gameObject->LoadProperties(jsonObject);

    if (jsonObject.contains(kJsonKeyComponents) && jsonObject[kJsonKeyComponents].is_array())
    {
        const auto &components = jsonObject[kJsonKeyComponents];
        for (const auto &comp : components)
        {
            const std::string type      = comp.value(kJsonKeyType, "");
            Component        *component = ComponentFactory::GetInstance().CreateComponent(type);
            if (component)
            {
                component->LoadProperties(comp);
                gameObject->AddComponent(component);
            } else
            {
                LOG_ERROR("Unknown component type '%s' on object '%s' (not registered in ComponentFactory)",
                          type.c_str(),
                          name.c_str());
            }
        }
    }

    if (jsonObject.contains(kJsonKeyChildren) && jsonObject[kJsonKeyChildren].is_array())
    {
        const auto &children = jsonObject[kJsonKeyChildren];
        for (const auto &child : children)
        {
            LoadObject(child, gameObject);
        }
    }

    gameObject->Init();
}

std::shared_ptr<Scene> Scene::Load(const str &path)
{
    LOG_INFO("Scene::Load '%s'", path.c_str());
    const str contents = Engine::GetInstance().GetFileSystem().LoadAssetFileText(path);
    if (contents.empty())
    {
        LOG_ERROR("Scene::Load empty or missing file '%s'", path.c_str());
        return nullptr;
    }

    nlohmann::json json;
    try
    {
        json = nlohmann::json::parse(contents);
    } catch (const nlohmann::json::parse_error &e)
    {
        LOG_ERROR("Scene::Load JSON parse error in '%s': %s", path.c_str(), e.what());
        return nullptr;
    }
    if (json.empty())
    {
        LOG_ERROR("Scene::Load JSON empty in '%s'", path.c_str());
        return nullptr;
    }

    auto result = std::make_shared<Scene>();

    const str sceneName = json.value(kJsonKeyName, kDefaultSceneName);
    if (json.contains(kJsonKeyObjects) && json[kJsonKeyObjects].is_array())
    {
        const auto &objects = json[kJsonKeyObjects];
        const auto  total   = objects.size();
        std::size_t i       = 0;
        for (const auto &obj : objects)
        {
            const str objName = obj.value(kJsonKeyName, str("object"));
            char      msg[128];
            std::snprintf(msg, sizeof(msg), "Loading %s (%zu/%zu)", objName.c_str(), i + 1, total);
            Engine::GetInstance().UpdateLoadingProgress(
                total > 0 ? static_cast<float>(i) / static_cast<float>(total) : 0.0F, msg);
            result->LoadObject(obj, nullptr);
            ++i;
        }
    } else
    {
        LOG_WARN("Scene '%s' has no 'objects' array", sceneName.c_str());
    }

    if (json.contains(kJsonKeyCamera))
    {
        str cameraObjName = json.value(kJsonKeyCamera, "");
        for (const auto &child : result->m_objects)
        {
            if (auto object = child->FindChildByName(cameraObjName))
            {
                result->SetMainCamera(object);
                break;
            }
        }
        if (!result->GetMainCamera())
        {
            LOG_ERROR(
                "Scene '%s' camera target '%s' not found in loaded objects", sceneName.c_str(), cameraObjName.c_str());
        }
    } else
    {
        LOG_WARN("Scene '%s' has no 'camera' key — render will use identity matrices", sceneName.c_str());
    }

    LOG_INFO("Scene '%s' loaded (%zu root objects)", sceneName.c_str(), result->m_objects.size());
    return result;
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
}  // namespace mnd
