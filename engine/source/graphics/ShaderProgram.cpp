#include "graphics/ShaderProgram.h"

namespace ENG
{

ShaderProgram::ShaderProgram(GLuint shaderProgramID)
    : m_shaderProgramID(shaderProgramID)
{
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(m_shaderProgramID);
}

void ShaderProgram::Bind()
{
    glUseProgram(m_shaderProgramID);
}

GLint ShaderProgram::GetUniformLocation(const std::string &name)
{
    auto iter = m_uniformLocationCache.find(name);
    if (iter != m_uniformLocationCache.end()) {
        return iter->second;
    }
    GLint location               = glGetUniformLocation(m_shaderProgramID, name.c_str());
    m_uniformLocationCache[name] = location;

    return location;
}

void ShaderProgram::SetUniform(const std::string &name, float value)
{
    auto location = GetUniformLocation(name);
    glUniform1f(location, value);
}

void ShaderProgram::SetUniform(const std::string &name, float v0, float v1)
{
    auto location = GetUniformLocation(name);
    glUniform2f(location, v0, v1);
}
}  // namespace ENG
