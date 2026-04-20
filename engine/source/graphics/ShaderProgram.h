#pragma once
#include <string>
#include <unordered_map>

#include <GL/glew.h>

#include "Types.h"

namespace ENG
{
class Texture;

class ShaderProgram
{
public:
    ShaderProgram()                                 = delete;
    ShaderProgram(const ShaderProgram &)            = delete;
    ShaderProgram &operator=(const ShaderProgram &) = delete;

    explicit ShaderProgram(GLuint shaderProgramID);
    ~ShaderProgram();

    void  Bind();
    GLint GetUniformLocation(const std::string &name);

    void SetUniform(const std::string &name, float value);
    void SetUniform(const std::string &name, float v0, float v1);
    void SetUniform(const std::string &name, float v0, float v1, float v2);
    void SetUniform(const std::string &name, float v0, float v1, float v2, float v3);
    void SetUniform(const std::string &name, const mat4 &mat);
    void SetTexture(const std::string &name, Texture *texture);

private:
    std::unordered_map<std::string, GLint> m_uniformLocationCache;
    GLuint                                 m_shaderProgramID    = 0;
    int                                    m_currentTextureUnit = 0;
};
}  // namespace ENG
