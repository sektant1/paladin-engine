#include <memory>

#include "scene/GameObject.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "graphics/Texture.h"
#include "graphics/VertexLayout.h"
#include "render/Mesh.h"
#include "scene/components/MeshComponent.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

#include "Engine.h"
#include "Log.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

namespace ENG
{

void GameObject::Update(f32 deltaTime)
{
    for (auto &component : m_components)
    {
        component->Update(deltaTime);
    }

    for (auto it = m_children.begin(); it != m_children.end();)
    {
        if ((*it)->IsAlive())
        {
            (*it)->Update(deltaTime);
            ++it;
        } else
        {
            it = m_children.erase(it);
        }
    }
}

void GameObject::AddComponent(Component *component)
{
    if (!component)
    {
        LOG_ERROR("AddComponent called with nullptr on '%s'", m_name.c_str());
        return;
    }
    m_components.emplace_back(component);
    component->m_owner = this;
    LOG_INFO("Component added to '%s' (total=%zu)", m_name.c_str(), m_components.size());
}

const vec3 &GameObject::GetPosition() const
{
    return m_position;
}

void GameObject::SetPosition(const vec3 &pos)
{
    m_position = pos;
}

const quat &GameObject::GetRotation() const
{
    return m_rotation;
}

void GameObject::SetRotation(const quat &rot)
{
    m_rotation = rot;
}

const vec3 &GameObject::GetScale() const
{
    return m_scale;
}

void GameObject::SetScale(const vec3 &pos)
{
    m_scale = pos;
}

vec3 GameObject::GetWorldPosition() const
{
    vec4 hom = GetWorldTransform() * vec4(0.0F, 0.0F, 0.0F, 1.0F);
    return vec3(hom) / hom.w;
}

mat4 GameObject::GetLocalTransform() const
{
    mat4 mat = mat4(1.0F);

    // Translation
    mat = translate(mat, m_position);

    // Rotation
    // mat = rotate(mat, m_rotation.x, vec3(1.0f, 0.0f, 0.0f));  // X Axis
    // mat = rotate(mat, m_rotation.y, vec3(0.0f, 1.0f, 0.0f));  // Y Axis
    // mat = rotate(mat, m_rotation.z, vec3(0.0f, 0.0f, 1.0f));  // Z Axis
    mat = mat * mat4_cast(m_rotation);

    // Scale
    mat = scale(mat, m_scale);

    return mat;
}

mat4 GameObject::GetWorldTransform() const
{
    if (m_parent)
    {
        return m_parent->GetWorldTransform() * GetLocalTransform();
    } else
    {
        return GetLocalTransform();
    }
}

auto readFloats = [](const cgltf_accessor *acc, cgltf_size i, float *out, int n)
{
    std::fill(out, out + n, 0.0f);
    return cgltf_accessor_read_float(acc, i, out, n) == 1;
};

auto readIndex = [](const cgltf_accessor *acc, cgltf_size i)
{
    cgltf_uint out = 0;
    cgltf_bool ok  = cgltf_accessor_read_uint(acc, i, &out, 1);
    return ok ? static_cast<u32>(out) : 0;
};

void ParseGLTFNode(cgltf_node *node, GameObject *parent, const std::filesystem::path &folder)
{
    auto object = parent->GetScene()->CreateObject(node->name, parent);

    if (node->has_matrix)
    {
        auto mat = glm::make_mat4(node->matrix);
        vec3 translation, scale, skew;
        vec4 perspective;
        quat orientation;
        decompose(mat, scale, orientation, translation, skew, perspective);

        object->SetPosition(translation);
        object->SetRotation(orientation);
        object->SetScale(scale);
    } else
    {
        if (node->has_translation)
        {
            object->SetPosition(vec3(node->translation[0], node->translation[1], node->translation[2]));
        }
        if (node->has_rotation)
        {
            object->SetRotation(quat(node->rotation[3], node->rotation[0], node->rotation[1], node->rotation[2]));
        }
        if (node->has_scale)
        {
            object->SetScale(vec3(node->scale[0], node->scale[1], node->scale[2]));
        }
    }

    if (node->mesh)
    {
        for (cgltf_size pi = 0; pi < node->mesh->primitives_count; ++pi)
        {
            auto &primitive = node->mesh->primitives[pi];
            if (primitive.type != cgltf_primitive_type_triangles)
            {
                continue;
            }

            VertexLayout    vertexLayout;
            cgltf_accessor *accessors[4] = {nullptr, nullptr, nullptr};

            for (cgltf_size ai = 0; ai < primitive.attributes_count; ++ai)
            {
                auto &attr = primitive.attributes[ai];
                auto  acc  = attr.data;
                if (!acc)
                {
                    continue;
                }

                VertexElement element;
                element.type = GL_FLOAT;

                switch (attr.type)
                {
                    case cgltf_attribute_type_position: {
                        accessors[VertexElement::PositionIndex] = acc;

                        element.index = VertexElement::PositionIndex;
                        element.size  = 3;
                    }
                    break;
                    case cgltf_attribute_type_color: {
                        if (attr.index != 0)
                        {
                            continue;
                        }
                        accessors[VertexElement::ColorIndex] = acc;

                        element.index = VertexElement::ColorIndex;
                        element.size  = 3;
                    }
                    break;

                    case cgltf_attribute_type_texcoord: {
                        if (attr.index != 0)
                        {
                            continue;
                        }
                        accessors[VertexElement::UVIndex] = acc;

                        element.index = VertexElement::UVIndex;
                        element.size  = 2;
                    }
                    break;
                    case cgltf_attribute_type_normal: {
                        accessors[VertexElement::NormalIndex] = acc;

                        element.index = VertexElement::NormalIndex;
                        element.size  = 3;
                    }
                    break;
                    default:
                        continue;
                }

                if (element.size > 0)
                {
                    element.offset = vertexLayout.stride;
                    vertexLayout.stride += element.size * sizeof(f32);
                    vertexLayout.elements.push_back(element);
                }
            }

            if (!accessors[VertexElement::PositionIndex])
            {
                continue;
            }

            auto &vertexCount = accessors[VertexElement::PositionIndex]->count;

            std::vector<f32> vertices;
            vertices.resize(vertexLayout.stride / sizeof(f32) * vertexCount);

            for (cgltf_size vi = 0; vi < vertexCount; ++vi)
            {
                for (auto &elem : vertexLayout.elements)
                {
                    if (!accessors[elem.index])
                    {
                        continue;
                    }
                    auto index   = (vi * vertexLayout.stride + elem.offset) / sizeof(f32);
                    f32 *outData = &vertices[index];
                    readFloats(accessors[elem.index], vi, outData, elem.size);
                }
            }

            std::shared_ptr<Mesh> mesh;
            if (primitive.indices)
            {
                auto indexCount = primitive.indices->count;

                std::vector<u32> indices(indexCount);
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
                    auto pbr     = gltfMat->pbr_metallic_roughness;
                    auto texture = pbr.base_color_texture.texture;
                    if (texture && texture->image)
                    {
                        if (texture->image->uri)
                        {
                            auto path = folder / std::string(texture->image->uri);
                            auto tex  = Engine::GetInstance().GetTextureManager().GetOrLoadTexture(path);
                            mat->SetParam("baseColorTexture", tex);
                        }
                    }
                } else if (gltfMat->has_pbr_specular_glossiness)
                {
                    auto pbr     = gltfMat->pbr_specular_glossiness;
                    auto texture = pbr.diffuse_texture.texture;
                    if (texture && texture->image)
                    {
                        if (texture->image->uri)
                        {
                            auto path = folder / std::string(texture->image->uri);
                            auto tex  = Engine::GetInstance().GetTextureManager().GetOrLoadTexture(path);
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

GameObject *GameObject::LoadGLTF(const std::string &path)
{
    auto contents = Engine::GetInstance().GetFileSystem().LoadAssetFileText(path);
    if (contents.empty())
    {
        return nullptr;
    }

    cgltf_options options = {};
    cgltf_data   *data    = nullptr;
    cgltf_result  res     = cgltf_parse(&options, contents.data(), contents.size(), &data);

    if (res != cgltf_result_success)
    {
        return nullptr;
    }

    auto fullPath           = Engine::GetInstance().GetFileSystem().GetAssetsFolder() / path;
    auto fullFolderPath     = fullPath.remove_filename();
    auto relativeFolderPath = std::filesystem::path(path).remove_filename();

    res = cgltf_load_buffers(&options, data, fullFolderPath.string().c_str());

    if (res != cgltf_result_success)
    {
        cgltf_free(data);
        return nullptr;
    }

    auto resultObject = Engine::GetInstance().GetScene()->CreateObject("Result");
    auto scene        = &data->scenes[0];

    for (cgltf_size i = 0; i < scene->nodes_count; ++i)
    {
        auto node = scene->nodes[0];
        ParseGLTFNode(node, resultObject, relativeFolderPath);
    }

    cgltf_free(data);

    return resultObject;
}

const str &GameObject::GetName() const
{
    return m_name;
}

void GameObject::SetName(const str &name)
{
    m_name = name;
}

bool GameObject::SetParent(GameObject *parent)
{
    if (m_scene == nullptr)
    {
        return false;
    }
    return m_scene->SetParent(this, parent);
}

Scene *GameObject::GetScene()
{
    return m_scene;
}

GameObject *GameObject::GetParent()
{
    return m_parent;
}

bool GameObject::IsAlive() const
{
    return m_isAlive;
}

void GameObject::MarkForDestroy()
{
    LOG_INFO("GameObject '%s' marked for destroy", m_name.c_str());
    m_isAlive = false;
}

}  // namespace ENG
