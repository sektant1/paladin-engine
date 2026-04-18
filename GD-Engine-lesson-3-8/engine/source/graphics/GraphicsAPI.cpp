#include "graphics/GraphicsAPI.h"
#include "graphics/ShaderProgram.h"
#include "render/Material.h"
#include "render/Mesh.h"
#include <iostream>

namespace eng
{
    std::shared_ptr<ShaderProgram> GraphicsAPI::CreateShaderProgram(const std::string& vertexSource,
        const std::string& fragmentSource)
    {
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const char* vertexShaderCStr = vertexSource.c_str();
        glShaderSource(vertexShader, 1, &vertexShaderCStr, nullptr);
        glCompileShader(vertexShader);

        GLint success;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
            std::cerr << "ERROR:VERTEX_SHADER_COMPILATION_FAILED: " << infoLog << std::endl;
            return nullptr;
        }

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        const char* fragmentShaderSourceCStr = fragmentSource.c_str();
        glShaderSource(fragmentShader, 1, &fragmentShaderSourceCStr, nullptr);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
            std::cerr << "ERROR:FRAGMENT_SHADER_COMPILATION_FAILED: " << infoLog << std::endl;
            return nullptr;
        }

        GLuint shaderProgramID = glCreateProgram();
        glAttachShader(shaderProgramID, vertexShader);
        glAttachShader(shaderProgramID, fragmentShader);
        glLinkProgram(shaderProgramID);

        glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetProgramInfoLog(shaderProgramID, 512, nullptr, infoLog);
            std::cerr << "ERROR:SHADER_PROGRAM_LINKING_FAILED: " << infoLog << std::endl;
            return nullptr;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return std::make_shared<ShaderProgram>(shaderProgramID);
    }

    GLuint GraphicsAPI::CreateVertexBuffer(const std::vector<float>& vertices)
    {
        GLuint VBO = 0;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        return VBO;
    }

    GLuint GraphicsAPI::CreateIndexBuffer(const std::vector<uint32_t>& indices)
    {
        GLuint EBO = 0;
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        return EBO;
    }

    void GraphicsAPI::SetClearColor(float r, float g, float b, float a)
    {
        glClearColor(r, g, b, a);
    }

    void GraphicsAPI::ClearBuffers()
    {
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void GraphicsAPI::BindShaderProgram(ShaderProgram* shaderProgram)
    {
        if (shaderProgram)
        {
            shaderProgram->Bind();
        }
    }

    void GraphicsAPI::BindMaterial(Material* material)
    {
        if (material)
        {
            material->Bind();
        }
    }

    void GraphicsAPI::BindMesh(Mesh* mesh)
    {
        if (mesh)
        {
            mesh->Bind();
        }
    }

    void GraphicsAPI::DrawMesh(Mesh* mesh)
    {
        if (mesh)
        {
            mesh->Draw();
        }
    }
}