#pragma once
#include <string>
#include <unordered_map>
#include <GL/glew.h>
#include <glm/mat4x4.hpp>

namespace eng
{
    class ShaderProgram
    {
    public:
        ShaderProgram() = delete;
        ShaderProgram(const ShaderProgram&) = delete;
        ShaderProgram& operator=(const ShaderProgram&) = delete;
        explicit ShaderProgram(GLuint shaderProgramID);
        ~ShaderProgram();

        void Bind();
        GLint GetUniformLocation(const std::string& name);
        void SetUniform(const std::string& name, float value);
        void SetUniform(const std::string& name, float v0, float v1);
        void SetUniform(const std::string& name, const glm::mat4& mat);

    private:
        std::unordered_map<std::string, GLint> m_uniformLocationCache;
        GLuint m_shaderProgramID = 0;
    };
}