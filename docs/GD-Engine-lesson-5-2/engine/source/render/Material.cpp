#include "render/Material.h"
#include "graphics/ShaderProgram.h"
#include "graphics/Texture.h"
#include "Engine.h"

#include <nlohmann/json.hpp>

namespace eng
{
    void Material::SetShaderProgram(const std::shared_ptr<ShaderProgram>& shaderProgram)
    {
        m_shaderProgram = shaderProgram;
    }

    ShaderProgram* Material::GetShaderProgram()
    {
        return m_shaderProgram.get();
    }

    void Material::SetParam(const std::string& name, float value)
    {
        m_floatParams[name] = value;
    }

    void Material::SetParam(const std::string& name, float v0, float v1)
    {
        m_float2Params[name] = { v0, v1 };
    }

    void Material::SetParam(const std::string& name, const glm::vec3& value)
    {
        m_float3Params[name] = value;
    }

    void Material::SetParam(const std::string& name, const std::shared_ptr<Texture>& texture)
    {
        m_textures[name] = texture;
    }

    void Material::Bind()
    {
        if (!m_shaderProgram)
        {
            return;
        }

        m_shaderProgram->Bind();

        for (auto& param : m_floatParams)
        {
            m_shaderProgram->SetUniform(param.first, param.second);
        }

        for (auto& param : m_float2Params)
        {
            m_shaderProgram->SetUniform(param.first, param.second.first, param.second.second);
        }

        for (auto& param : m_float3Params)
        {
            m_shaderProgram->SetUniform(param.first, param.second);
        }

        for (auto& param : m_textures)
        {
            m_shaderProgram->SetTexture(param.first, param.second.get());
        }
    }

    std::shared_ptr<Material> Material::Load(const std::string& path)
    {
        auto contents = Engine::GetInstance().GetFileSystem().LoadAssetFileText(path);

        if (contents.empty())
        {
            return nullptr;
        }

        nlohmann::json json = nlohmann::json::parse(contents);
        std::shared_ptr<Material> result;

        if (json.contains("shader"))
        {
            auto shaderObj = json["shader"];
            std::string vertexPath = shaderObj.value("vertex", "");
            std::string fragmentPath = shaderObj.value("fragment", "");

            auto& fs = Engine::GetInstance().GetFileSystem();
            auto vertexSrc = fs.LoadAssetFileText(vertexPath);
            auto fragmentSrc = fs.LoadAssetFileText(fragmentPath);

            auto& graphicsAPI = Engine::GetInstance().GetGraphicsAPI();
            auto shaderProgram = graphicsAPI.CreateShaderProgram(vertexSrc, fragmentSrc);
            if (!shaderProgram)
            {
                return nullptr;
            }

            result = std::make_shared<Material>();
            result->SetShaderProgram(shaderProgram);
        }

        if (json.contains("params"))
        {
            auto paramsObj = json["params"];

            // Floats
            if (paramsObj.contains("float"))
            {
                for (auto& p : paramsObj["float"])
                {
                    std::string name = p.value("name", "");
                    float value = p.value("value", 0.0f);
                    result->SetParam(name, value);
                }
            }

            // Float2
            if (paramsObj.contains("float2"))
            {
                for (auto& p : paramsObj["float2"])
                {
                    std::string name = p.value("name", "");
                    float v0 = p.value("value0", 0.0f);
                    float v1 = p.value("value1", 0.0f);
                    result->SetParam(name, v0, v1);
                }
            }

            // Float3
            if (paramsObj.contains("float3"))
            {
                for (auto& p : paramsObj["float3"])
                {
                    std::string name = p.value("name", "");
                    float v0 = p.value("value0", 0.0f);
                    float v1 = p.value("value1", 0.0f);
                    float v2 = p.value("value2", 0.0f);
                    result->SetParam(name, glm::vec3(v0, v1, v2));
                }
            }

            // Textures
            if (paramsObj.contains("textures"))
            {
                for (auto& p : paramsObj["textures"])
                {
                    std::string name = p.value("name", "");
                    std::string texPath = p.value("path", "");
                    auto texture = Texture::Load(texPath);

                    result->SetParam(name, texture);
                }
            }
        }

        return result;
    }
}