#include "render/Material.h"

#include "graphics/ShaderProgram.h"

namespace ENG
{

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

void Material::SetParam(const std::string &name, float v0, float v1, float v2)
{
    m_float3Params[name] = {v0, v1, v2};
}

void Material::SetParam(const std::string &name, float v0, float v1, float v2, float v3)
{
    m_float4Params[name] = {v0, v1, v2, v3};
}

void Material::Bind()
{
    if (!m_shaderProgram) {
        return;
    }
    m_shaderProgram->Bind();

    for (const auto &param : m_floatParams) {
        m_shaderProgram->SetUniform(param.first, param.second);
    }
    for (const auto &param : m_float2Params) {
        auto &v = param.second;
        m_shaderProgram->SetUniform(param.first, std::get<0>(v), std::get<1>(v));
    }
    for (const auto &param : m_float3Params) {
        auto &v = param.second;
        m_shaderProgram->SetUniform(param.first, std::get<0>(v), std::get<1>(v), std::get<2>(v));
    }
    for (const auto &param : m_float4Params) {
        auto &v = param.second;
        m_shaderProgram->SetUniform(param.first, std::get<0>(v), std::get<1>(v), std::get<2>(v), std::get<3>(v));
    }
}
}  // namespace ENG
