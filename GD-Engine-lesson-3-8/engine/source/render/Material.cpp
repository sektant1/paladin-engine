#include "render/Material.h"
#include "graphics/ShaderProgram.h"

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
    }
}