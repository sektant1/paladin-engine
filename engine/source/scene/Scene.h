#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "Types.h"
#include "scene/GameObject.h"

namespace ENG
{

class Scene
{
public:
    void Update(f32 deltaTime);
    void Clear();

    GameObject *CreateObject(const std::string &name, GameObject *parent = nullptr);

    template<typename T, typename = typename std::enable_if_t<std::is_base_of_v<GameObject, T>>>
    T *CreateObject(const str &name, GameObject *parent = nullptr)
    {
        auto obj = new T();
        obj->SetName(name);
        SetParent(obj, parent);

        return obj;
    }

    bool        SetParent(GameObject *obj, GameObject *parent);
    void        SetMainCamera(GameObject *camera);
    GameObject *GetMainCamera();

private:
    std::vector<std::unique_ptr<GameObject>> m_objects;

    GameObject *m_mainCamera = nullptr;
};
}  // namespace ENG
