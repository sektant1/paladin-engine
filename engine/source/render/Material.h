#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "graphics/ShaderProgram.h"

namespace ENG
{
class ShaderProgram;

class Material
{
public:
    void SetShaderProgram(const std::shared_ptr<ShaderProgram> &shaderProgram);
    void SetParam(const std::string &name, float value);
    void SetParam(const std::string &name, float v0, float v1);
    void Bind();

private:
    std::shared_ptr<ShaderProgram>                           m_shaderProgram;
    std::unordered_map<std::string, float>                   m_floatParams;
    std::unordered_map<std::string, std::pair<float, float>> m_float2Params;
};
}  // namespace ENG
