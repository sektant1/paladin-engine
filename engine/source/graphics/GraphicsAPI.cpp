#include "graphics/GraphicsAPI.h"

#include "Log.h"
#include "graphics/ShaderProgram.h"
#include "render/Material.h"
#include "render/Mesh.h"

namespace ENG
{

std::shared_ptr<ShaderProgram> GraphicsAPI::CreateShaderProgram(const std::string &vertexSource,
                                                                const std::string &fragmentSource)
{
    GLuint      vertexShader     = glCreateShader(GL_VERTEX_SHADER);
    const char *vertexShaderCStr = vertexSource.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderCStr, nullptr);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        LOG_ERROR("Vertex shader compilation failed: %s", infoLog);
        return nullptr;
    }

    GLuint      fragmentShader           = glCreateShader(GL_FRAGMENT_SHADER);
    const char *fragmentShaderSourceCStr = fragmentSource.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderSourceCStr, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        LOG_ERROR("Fragment shader compilation failed: %s", infoLog);
        return nullptr;
    }

    GLuint shaderProgramID = glCreateProgram();
    glAttachShader(shaderProgramID, vertexShader);
    glAttachShader(shaderProgramID, fragmentShader);
    glLinkProgram(shaderProgramID);

    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgramID, 512, nullptr, infoLog);
        LOG_ERROR("Shader program linking failed: %s", infoLog);
        return nullptr;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    LOG_INFO("Shader program created (id=%u)", shaderProgramID);
    return std::make_shared<ShaderProgram>(shaderProgramID);
}

GLuint GraphicsAPI::CreateVertexBuffer(const std::vector<float> &vertices)
{
    GLuint VBO = 0;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return VBO;
}

GLuint GraphicsAPI::CreateIndexBuffer(const std::vector<uint32_t> &indices)
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

void GraphicsAPI::BindShaderProgram(ShaderProgram *shaderProgram)
{
    if (shaderProgram) {
        shaderProgram->Bind();
    } else {
        LOG_WARN("BindShaderProgram called with nullptr");
    }
}

void GraphicsAPI::BindMaterial(Material *material)
{
    if (material) {
        material->Bind();
    } else {
        LOG_WARN("BindMaterial called with nullptr");
    }
}

void GraphicsAPI::BindMesh(Mesh *mesh)
{
    if (mesh) {
        mesh->Bind();
    } else {
        LOG_WARN("BindMesh called with nullptr");
    }
}

void GraphicsAPI::DrawMesh(Mesh *mesh)
{
    if (mesh) {
        mesh->Draw();
    } else {
        LOG_WARN("DrawMesh called with nullptr");
    }
}

}  // namespace ENG
