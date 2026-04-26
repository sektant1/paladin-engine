#include <memory>

#include "render/Material.h"

#include <nlohmann/json.hpp>

#include "Engine.h"
#include "Log.h"
#include "graphics/ShaderProgram.h"

namespace mnd
{

ShaderProgram *Material::GetShaderProgram()
{
    return m_shaderProgram.get();
}

void Material::SetShaderProgram(const std::shared_ptr<ShaderProgram> &shaderProgram)
{
    m_shaderProgram = shaderProgram;
}

void Material::SetParam(const std::string &name, float value)
{
    m_floatParams[name] = value;
}

void Material::SetParam(const std::string &name, float v0, float v1)
{
    m_float2Params[name] = {v0, v1};
}

void Material::SetParam(const std::string &name, const vec3 &value)
{
    m_float3Params[name] = value;
}

void Material::SetParam(const std::string &name, const vec4 &value)
{
    m_float4Params[name] = value;
}

void Material::SetParam(const std::string &name, const std::shared_ptr<Texture> &texture)
{
    m_textures[name] = texture;
}

std::shared_ptr<Material> Material::Load(const str &path)
{
    auto contents = Engine::GetInstance().GetFileSystem().LoadAssetFileText(path);
    if (contents.empty())
    {
        LOG_ERROR("Material::Load empty or missing file '%s'", path.c_str());
        return nullptr;
    }

    nlohmann::json json;
    try
    {
        json = nlohmann::json::parse(contents);
    } catch (const nlohmann::json::parse_error &e)
    {
        LOG_ERROR("Material::Load JSON parse error in '%s': %s", path.c_str(), e.what());
        return nullptr;
    }
    std::shared_ptr<Material> result;

    if (json.contains("shader"))
    {
        auto shaderObj    = json["shader"];
        str  vertexPath   = shaderObj.value("vertex", "");
        str  fragmentPath = shaderObj.value("fragment", "");

        auto &fs             = Engine::GetInstance().GetFileSystem();
        auto  vertexSource   = fs.LoadAssetFileText(vertexPath);
        auto  fragmentSource = fs.LoadAssetFileText(fragmentPath);

        auto &graphicsAPI   = Engine::GetInstance().GetGraphicsAPI();
        auto  shaderProgram = graphicsAPI.CreateShaderProgram(vertexSource, fragmentSource);

        if (!shaderProgram)
        {
            LOG_ERROR(
                "Material::Load shader program creation failed (vert='%s' frag='%s')",
                vertexPath.c_str(),
                fragmentPath.c_str()
            );
            return nullptr;
        }

        result = std::make_shared<Material>();
        result->SetShaderProgram(shaderProgram);
    }

    if (json.contains("params"))
    {
        auto paramsObj = json["params"];

        // Float
        if (paramsObj.contains("float"))
        {
            for (auto &param : paramsObj["float"])
            {
                str name  = param.value("name", "");
                f32 value = param.value("value", 0.0f);
                result->SetParam(name, value);
            }
        }

        // Float 2
        if (paramsObj.contains("float2"))
        {
            for (auto &param : paramsObj["float2"])
            {
                str name = param.value("name", "");
                f32 v0   = param.value("value0", 0.0F);
                f32 v1   = param.value("value1", 0.0F);

                result->SetParam(name, v0, v1);
            }
        }

        // Float 3
        if (paramsObj.contains("float3"))
        {
            for (auto &param : paramsObj["float3"])
            {
                str name = param.value("name", "");
                f32 v0   = param.value("value0", 0.0F);
                f32 v1   = param.value("value1", 0.0F);
                f32 v2   = param.value("value2", 0.0F);

                result->SetParam(name, vec3(v0, v1, v2));
            }
        }

        // Float 4
        if (paramsObj.contains("float4"))
        {
            for (auto &param : paramsObj["float4"])
            {
                str name = param.value("name", "");
                f32 v0   = param.value("value0", 0.0F);
                f32 v1   = param.value("value1", 0.0F);
                f32 v2   = param.value("value2", 0.0F);
                f32 v3   = param.value("value3", 0.0F);

                result->SetParam(name, vec4(v0, v1, v2, v3));
            }
        }

        // Textures
        if (paramsObj.contains("textures"))
        {
            for (auto &param : paramsObj["textures"])
            {
                str  name    = param.value("name", "");
                str  texPath = param.value("path", "");
                auto texture = Texture::Load(texPath);

                result->SetParam(name, texture);
            }
        }
    }

    return result;
}

void Material::Bind()
{
    if (!m_shaderProgram)
    {
        LOG_WARN("Material::Bind with no shader program set");
        return;
    }
    m_shaderProgram->Bind();

    for (const auto &param : m_floatParams)
    {
        m_shaderProgram->SetUniform(param.first, param.second);
    }
    for (const auto &param : m_float2Params)
    {
        auto &v = param.second;
        m_shaderProgram->SetUniform(param.first, std::get<0>(v), std::get<1>(v));
    }
    for (const auto &param : m_float3Params)
    {
        auto &v = param.second;
        m_shaderProgram->SetUniform(param.first, param.second);
    }
    for (const auto &param : m_float4Params)
    {
        auto &v = param.second;
        m_shaderProgram->SetUniform(param.first, param.second);
    }
    for (const auto &tex : m_textures)
    {
        m_shaderProgram->SetTexture(tex.first, tex.second.get());
    }
}
}  // namespace mnd
