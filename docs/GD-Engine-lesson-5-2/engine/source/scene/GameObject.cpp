#include "scene/GameObject.h"
#include "Engine.h"
#include "graphics/VertexLayout.h"
#include "graphics/Texture.h"
#include "render/Material.h"
#include "render/Mesh.h"
#include "scene/components/MeshComponent.h"
#include "scene/components/AnimationComponent.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

namespace eng
{
    void GameObject::Init()
    {
    }

    void GameObject::LoadProperties(const nlohmann::json& json)
    {
    }

    void GameObject::Update(float deltaTime)
    {
        if (!m_active)
        {
            return;
        }

        for (auto& component : m_components)
        {
            component->Update(deltaTime);
        }

        for (auto it = m_children.begin(); it != m_children.end();)
        {
            if ((*it)->IsAlive())
            {
                (*it)->Update(deltaTime);
                ++it;
            }
            else
            {
                it = m_children.erase(it);
            }
        }
    }

    const std::string& GameObject::GetName() const
    {
        return m_name;
    }

    void GameObject::SetName(const std::string& name)
    {
        m_name = name;
    }

    GameObject* GameObject::GetParent()
    {
        return m_parent;
    }

    bool GameObject::SetParent(GameObject* parent)
    {
        if (!m_scene)
        {
            return false;
        }

        return m_scene->SetParent(this, parent);
    }

    Scene* GameObject::GetScene()
    {
        return m_scene;
    }

    bool GameObject::IsAlive() const
    {
        return m_isAlive;
    }

    void GameObject::MarkForDestroy()
    {
        m_isAlive = false;
    }

    void GameObject::SetActive(bool active)
    {
        m_active = active;
    }

    bool GameObject::IsActive() const
    {
        return m_active;
    }

    void GameObject::AddComponent(Component* component)
    {
        if (!component)
        {
            return;
        }
        m_components.emplace_back(component);
        component->m_owner = this;
        component->Init();
    }

    GameObject* GameObject::FindChildByName(const std::string& name)
    {
        if (m_name == name)
        {
            return this;
        }

        for (auto& child : m_children)
        {
            if (auto res = child->FindChildByName(name))
            {
                return res;
            }
        }

        return nullptr;
    }

    const glm::vec3& GameObject::GetPosition() const
    {
        return m_position;
    }

    glm::vec3 GameObject::GetWorldPosition() const
    {
        glm::vec4 hom = GetWorldTransform() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        return glm::vec3(hom) / hom.w;
    }

    glm::vec2 GameObject::GetPosition2D() const
    {
        return glm::vec2(m_position);
    }

    glm::vec2 GameObject::GetWorldPosition2D() const
    {
        glm::vec4 hom = GetWorldTransform2D() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        return glm::vec3(hom) / hom.w;
    }

    void GameObject::SetPosition(const glm::vec3& pos)
    {
        m_position = pos;
    }

    void GameObject::SetWorldPosition(const glm::vec3& pos)
    {
        if (m_parent)
        {
            glm::mat4 parentWorld = m_parent->GetWorldTransform();
            glm::mat4 invParentWorld = glm::inverse(parentWorld);
            glm::vec4 localPos = invParentWorld * glm::vec4(pos, 1.0f);
            SetPosition(glm::vec3(localPos) / localPos.w);
        }
        else
        {
            SetPosition(pos);
        }
    }

    void GameObject::SetPosition2D(const glm::vec2& pos)
    {
        m_position = glm::vec3(pos, 0.0f);
    }

    const glm::quat& GameObject::GetRotation() const
    {
        return m_rotation;
    }

    glm::quat GameObject::GetWorldRotation()
    {
        if (m_parent)
        {
            return m_parent->GetWorldRotation() * m_rotation;
        }
        else
        {
            return m_rotation;
        }
    }

    float GameObject::GetRotation2D() const
    {
        return glm::angle(m_rotation);
    }

    void GameObject::SetRotation(const glm::quat& rot)
    {
        m_rotation = rot;
    }

    void GameObject::SetWorldRotation(const glm::quat& rot)
    {
        if (m_parent)
        {
            glm::quat parentWorldRot = m_parent->GetWorldRotation();
            glm::quat invParentWorldRot = glm::inverse(parentWorldRot);
            glm::quat newLocalRot = invParentWorldRot * rot;
            SetRotation(newLocalRot);
        }
        else
        {
            SetRotation(rot);
        }
    }

    void GameObject::SetRotation2D(float rotation)
    {
        m_rotation = glm::angleAxis(rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    }

    const glm::vec3& GameObject::GetScale() const
    {
        return m_scale;
    }

    glm::vec2 GameObject::GetScale2D() const
    {
        return glm::vec2(m_scale);
    }

    void GameObject::SetScale(const glm::vec3& scale)
    {
        m_scale = scale;
    }

    void GameObject::SetScale2D(const glm::vec2& scale)
    {
        m_scale = glm::vec3(scale, 1.0f);
    }

    glm::mat4 GameObject::GetLocalTransform() const
    {
        glm::mat4 mat = glm::mat4(1.0f);

        // Translation
        mat = glm::translate(mat, m_position);

        // Rotation
        mat = mat * glm::mat4_cast(m_rotation);

        // Scale
        mat = glm::scale(mat, m_scale);

        return mat;
    }

    glm::mat4 GameObject::GetLocalTransform2D() const
    {
        glm::mat4 mat = glm::mat4(1.0f);

        const auto rotationZ = GetRotation2D();
        float c = cos(rotationZ);
        float s = sin(rotationZ);

        mat[0][0] = m_scale.x * c;
        mat[0][1] = m_scale.x * s;
        mat[1][0] = -m_scale.y * s;
        mat[1][1] = m_scale.y * c;
        mat[3][0] = m_position.x;
        mat[3][1] = m_position.y;

        return mat;
    }

    glm::mat4 GameObject::GetWorldTransform() const
    {
        if (m_parent)
        {
            return m_parent->GetWorldTransform() * GetLocalTransform();
        }
        else
        {
            return GetLocalTransform();
        }
    }

    glm::mat4 GameObject::GetWorldTransform2D() const
    {
        if (m_parent)
        {
            return m_parent->GetWorldTransform2D() * GetLocalTransform2D();
        }
        else
        {
            return GetLocalTransform2D();
        }
    }

    void ParseGLTFNode(cgltf_node* node, GameObject* parent, const std::filesystem::path& folder)
    {
        auto object = parent->GetScene()->CreateObject(node->name, parent);

        if (node->has_matrix)
        {
            auto mat = glm::make_mat4(node->matrix);
            glm::vec3 translation, scale, skew;
            glm::vec4 perspective;
            glm::quat orentation;
            glm::decompose(mat, scale, orentation, translation, skew, perspective);

            object->SetPosition(translation);
            object->SetRotation(orentation);
            object->SetScale(scale);
        }
        else
        {
            if (node->has_translation)
            {
                object->SetPosition(glm::vec3(node->translation[0],
                    node->translation[1],
                    node->translation[2]));
            }

            if (node->has_rotation)
            {
                object->SetRotation(glm::quat(node->rotation[3],
                    node->rotation[0],
                    node->rotation[1],
                    node->rotation[2]));
            }

            if (node->has_scale)
            {
                object->SetScale(glm::vec3(node->scale[0],
                    node->scale[1],
                    node->scale[2]));
            }
        }

        if (node->mesh)
        {
            for (cgltf_size pi = 0; pi < node->mesh->primitives_count; ++pi)
            {
                auto& primitive = node->mesh->primitives[pi];
                if (primitive.type != cgltf_primitive_type_triangles)
                {
                    continue;
                }

                auto readFloats = [](const cgltf_accessor* acc, cgltf_size i, float* out, int n)
                    {
                        std::fill(out, out + n, 0.0f);
                        return cgltf_accessor_read_float(acc, i, out, n) == 1;
                    };

                auto readIndex = [](const cgltf_accessor* acc, cgltf_size i)
                    {
                        cgltf_uint out = 0;
                        cgltf_bool ok = cgltf_accessor_read_uint(acc, i, &out, 1);
                        return ok ? static_cast<uint32_t>(out) : 0;
                    };

                VertexLayout vertexLayout;
                cgltf_accessor* accessors[4] = { nullptr, nullptr, nullptr };

                for (cgltf_size ai = 0; ai < primitive.attributes_count; ++ai)
                {
                    auto& attr = primitive.attributes[ai];
                    auto acc = attr.data;
                    if (!acc)
                    {
                        continue;
                    }

                    VertexElement element;
                    element.type = GL_FLOAT;

                    switch (attr.type)
                    {
                    case cgltf_attribute_type_position:
                    {
                        accessors[VertexElement::PositionIndex] = acc;
                        element.index = VertexElement::PositionIndex;
                        element.size = 3;
                    }
                    break;
                    case cgltf_attribute_type_color:
                    {
                        if (attr.index != 0)
                        {
                            continue;
                        }
                        accessors[VertexElement::ColorIndex] = acc;
                        element.index = VertexElement::ColorIndex;
                        element.size = 3;
                    }
                    break;
                    case cgltf_attribute_type_texcoord:
                    {
                        if (attr.index != 0)
                        {
                            continue;
                        }
                        accessors[VertexElement::UVIndex] = acc;
                        element.index = VertexElement::UVIndex;
                        element.size = 2;
                    }
                    break;
                    case cgltf_attribute_type_normal:
                    {
                        accessors[VertexElement::NormalIndex] = acc;
                        element.index = VertexElement::NormalIndex;
                        element.size = 3;
                    }
                    break;
                    default:
                        continue;
                    }

                    if (element.size > 0)
                    {
                        element.offset = vertexLayout.stride;
                        vertexLayout.stride += element.size * sizeof(float);
                        vertexLayout.elements.push_back(element);
                    }
                }

                if (!accessors[VertexElement::PositionIndex])
                {
                    continue;
                }
                auto vertexCount = accessors[VertexElement::PositionIndex]->count;

                std::vector<float> vertices;
                vertices.resize((vertexLayout.stride / sizeof(float))* vertexCount);

                for (cgltf_size vi = 0; vi < vertexCount; ++vi)
                {
                    for (auto& el : vertexLayout.elements)
                    {
                        if (!accessors[el.index])
                        {
                            continue;
                        }

                        auto index = (vi * vertexLayout.stride + el.offset) / sizeof(float);
                        float* outData = &vertices[index];
                        readFloats(accessors[el.index], vi, outData, el.size);
                    }
                }

                std::shared_ptr<Mesh> mesh;
                if (primitive.indices)
                {
                    auto indexCount = primitive.indices->count;
                    std::vector<uint32_t> indices(indexCount);
                    for (cgltf_size i = 0; i < indexCount; ++i)
                    {
                        indices[i] = readIndex(primitive.indices, i);
                    }
                    mesh = std::make_shared<Mesh>(vertexLayout, vertices, indices);
                }
                else
                {
                    mesh = std::make_shared<Mesh>(vertexLayout, vertices);
                }

                auto mat = std::make_shared<Material>();
                mat->SetShaderProgram(Engine::GetInstance().GetGraphicsAPI().GetDefaultShaderProgram());

                if (primitive.material)
                {
                    auto gltfMat = primitive.material;
                    if (gltfMat->has_pbr_metallic_roughness)
                    {
                        auto pbr = gltfMat->pbr_metallic_roughness;
                        auto texture = pbr.base_color_texture.texture;
                        if (texture && texture->image)
                        {
                            if (texture->image->uri)
                            {
                                auto path = folder / std::string(texture->image->uri);
                                auto tex = Engine::GetInstance().GetTextureManager().GetOrLoadTexture(path.string());
                                mat->SetParam("baseColorTexture", tex);
                            }
                        }
                    }
                    else if (gltfMat->has_pbr_specular_glossiness)
                    {
                        auto pbr = gltfMat->pbr_specular_glossiness;
                        auto texture = pbr.diffuse_texture.texture;
                        if (texture && texture->image)
                        {
                            if (texture->image->uri)
                            {
                                auto path = folder / std::string(texture->image->uri);
                                auto tex = Engine::GetInstance().GetTextureManager().GetOrLoadTexture(path.string());
                                mat->SetParam("baseColorTexture", tex);
                            }
                        }
                    }

                    object->AddComponent(new MeshComponent(mat, mesh));
                }
            }
        }

        for (cgltf_size ci = 0; ci < node->children_count; ++ci)
        {
            ParseGLTFNode(node->children[ci], object, folder);
        }
    }

    auto ReadScalar = [](cgltf_accessor* acc, cgltf_size index)
        {
            float res = 0.0f;
            cgltf_accessor_read_float(acc, index, &res, 1);
            return res;
        };

    auto ReadVec3 = [](cgltf_accessor* acc, cgltf_size index)
        {
            glm::vec3 res;
            cgltf_accessor_read_float(acc, index, glm::value_ptr(res), 3);
            return res;
        };

    auto ReadQuat = [](cgltf_accessor* acc, cgltf_size index)
        {
            float res[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            cgltf_accessor_read_float(acc, index, res, 4);
            return glm::quat(res[3], res[0], res[1], res[2]);
        };

    auto ReadTimes = [](cgltf_accessor* acc, std::vector<float>& outTimes)
        {
            outTimes.resize(acc->count);
            for (cgltf_size i = 0; i < acc->count; ++i)
            {
                outTimes[i] = ReadScalar(acc, i);
            }
        };

    auto ReadOutputVec3 = [](cgltf_accessor* acc, std::vector<glm::vec3>& outValues)
        {
            outValues.resize(acc->count);
            for (cgltf_size i = 0; i < acc->count; ++i)
            {
                outValues[i] = ReadVec3(acc, i);
            }
        };

    auto ReadOutputQuat = [](cgltf_accessor* acc, std::vector<glm::quat>& outValues)
        {
            outValues.resize(acc->count);
            for (cgltf_size i = 0; i < acc->count; ++i)
            {
                outValues[i] = ReadQuat(acc, i);
            }
        };

    GameObject* GameObject::LoadGLTF(const std::string& path, Scene* gameScene)
    {
        auto contents = Engine::GetInstance().GetFileSystem().LoadAssetFileText(path);
        if (contents.empty())
        {
            return nullptr;
        }

        if (!gameScene)
        {
            return nullptr;
        }

        cgltf_options options = {};
        cgltf_data* data = nullptr;

        cgltf_result res = cgltf_parse(&options, contents.data(), contents.size(), &data);
        if (res != cgltf_result_success)
        {
            return nullptr;
        }

        auto fullPath = Engine::GetInstance().GetFileSystem().GetAssetsFolder() / path;
        auto fullFolderPath = fullPath.remove_filename();
        auto relativeFolderPath = std::filesystem::path(path).remove_filename();

        res = cgltf_load_buffers(&options, data, fullFolderPath.string().c_str());
        if (res != cgltf_result_success)
        {
            cgltf_free(data);
            return nullptr;
        }

        auto resultObject = gameScene->CreateObject("Result");
        auto scene = &data->scenes[0];

        for (cgltf_size i = 0; i < scene->nodes_count; ++i)
        {
            auto node = scene->nodes[i];
            ParseGLTFNode(node, resultObject, relativeFolderPath);
        }

        std::vector<std::shared_ptr<AnimationClip>> clips;
        for (cgltf_size ai = 0; ai < data->animations_count; ++ai)
        {
            auto& anim = data->animations[ai];

            auto clip = std::make_shared<AnimationClip>();
            clip->name = anim.name ? anim.name : "noname";
            clip->duration = 0.0f;

            std::unordered_map<cgltf_node*, size_t> trackIndexOf;

            auto GetOrCreateTrack = [&](cgltf_node* node) -> TransformTrack&
                {
                    auto it = trackIndexOf.find(node);
                    if (it != trackIndexOf.end())
                    {
                        return clip->tracks[it->second];
                    }

                    TransformTrack track;
                    track.targetName = node->name;
                    clip->tracks.push_back(track);
                    size_t idx = clip->tracks.size() - 1;
                    trackIndexOf[node] = idx;
                    return clip->tracks[idx];
                };

            for (cgltf_size ci = 0; ci < anim.channels_count; ++ci)
            {
                auto& channel = anim.channels[ci];
                auto sampler = channel.sampler;

                if (!channel.target_node || !sampler || !sampler->input || !sampler->output)
                {
                    continue;
                }

                std::vector<float> times;
                ReadTimes(sampler->input, times);

                auto& track = GetOrCreateTrack(channel.target_node);

                switch (channel.target_path)
                {
                case cgltf_animation_path_type_translation:
                {
                    std::vector<glm::vec3> values;
                    ReadOutputVec3(sampler->output, values);
                    track.positions.resize(times.size());
                    for (size_t i = 0; i < times.size(); ++i)
                    {
                        track.positions[i].time = times[i];
                        track.positions[i].value = values[i];
                    }
                }
                break;
                case cgltf_animation_path_type_rotation:
                {
                    std::vector<glm::quat> values;
                    ReadOutputQuat(sampler->output, values);
                    track.rotations.resize(times.size());
                    for (size_t i = 0; i < times.size(); ++i)
                    {
                        track.rotations[i].time = times[i];
                        track.rotations[i].value = values[i];
                    }
                }
                break;
                case cgltf_animation_path_type_scale:
                {
                    std::vector<glm::vec3> values;
                    ReadOutputVec3(sampler->output, values);
                    track.scales.resize(times.size());
                    for (size_t i = 0; i < times.size(); ++i)
                    {
                        track.scales[i].time = times[i];
                        track.scales[i].value = values[i];
                    }
                }
                break;
                default:
                    break;
                }

                clip->duration = std::max(clip->duration, times.back());
            }

            clips.push_back(std::move(clip));
        }

        if (!clips.empty())
        {
            auto animComp = new AnimationComponent();
            resultObject->AddComponent(animComp);
            for (auto& clip : clips)
            {
                animComp->RegisterClip(clip->name, clip);
            }
        }

        cgltf_free(data);

        return resultObject;
    }

    GameObjectFactory& GameObjectFactory::GetInstance()
    {
        static GameObjectFactory instance;
        return instance;
    }

    GameObject* GameObjectFactory::CreateGameObject(const std::string& typeName)
    {
        auto it = m_creators.find(typeName);
        if (it == m_creators.end())
        {
            return nullptr;
        }

        return it->second->CreateGameObject();
    }
}