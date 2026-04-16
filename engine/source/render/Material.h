#pragma once
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>

namespace ENG
{
class ShaderProgram;

class Material
{
public:
    void SetShaderProgram(const std::shared_ptr<ShaderProgram> &shaderProgram);

    void SetParam(const std::string &name, float value);
    void SetParam(const std::string &name, float v0, float v1);
    void SetParam(const std::string &name, float v0, float v1, float v2);
    void SetParam(const std::string &name, float v0, float v1, float v2, float v3);

    void Bind();

private:
    std::shared_ptr<ShaderProgram>                                          m_shaderProgram;
    std::unordered_map<std::string, float>                                  m_floatParams;
    std::unordered_map<std::string, std::tuple<float, float>>               m_float2Params;
    std::unordered_map<std::string, std::tuple<float, float, float>>        m_float3Params;
    std::unordered_map<std::string, std::tuple<float, float, float, float>> m_float4Params;
};
}  // namespace ENG
