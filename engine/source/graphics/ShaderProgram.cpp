#include "graphics/ShaderProgram.h"

#include "Log.h"

namespace ENG
{
ShaderProgram::ShaderProgram(GLuint shaderProgramID)
    : m_shaderProgramID(shaderProgramID)
{
    LOG_INFO("ShaderProgram constructed (id=%u)", m_shaderProgramID);
}

ShaderProgram::~ShaderProgram()
{
    LOG_INFO("ShaderProgram destroyed (id=%u)", m_shaderProgramID);
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
    GLint location = glGetUniformLocation(m_shaderProgramID, name.c_str());
    if (location == -1) {
        LOG_WARN("Uniform '%s' not found in shader program %u", name.c_str(), m_shaderProgramID);
    }
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

void ShaderProgram::SetUniform(const std::string &name, float v0, float v1, float v2)
{
    auto location = GetUniformLocation(name);
    glUniform3f(location, v0, v1, v2);
}

void ShaderProgram::SetUniform(const std::string &name, float v0, float v1, float v2, float v3)
{
    auto location = GetUniformLocation(name);
    glUniform4f(location, v0, v1, v2, v3);
}

void ShaderProgram::SetUniform(const std::string &name, const mat4 &mat)
{
    auto location = GetUniformLocation(name);
    glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(mat));
}

}  // namespace ENG
