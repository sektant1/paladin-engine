#pragma once
#include <memory>
#include <string>
#include <vector>
#include <GL/glew.h>

namespace eng
{
    class ShaderProgram;
    class Material;
    class Mesh;

    enum class BlendMode
    {
        Disabled,
        Alpha,
        Additive,
        Multiply
    };

    class GraphicsAPI
    {
    public:
        bool Init();
        std::shared_ptr<ShaderProgram> CreateShaderProgram(const std::string& vertexSource, 
            const std::string& fragmentSource);
        const std::shared_ptr<ShaderProgram>& GetDefaultShaderProgram();
        const std::shared_ptr<ShaderProgram>& GetDefault2DShaderProgram();
        GLuint CreateVertexBuffer(const std::vector<float>& vertices);
        GLuint CreateIndexBuffer(const std::vector<uint32_t>& indices);

        void SetClearColor(float r, float g, float b, float a);
        void ClearBuffers();
        void SetDepthTestEnabled(bool enabled);
        void SetBlendMode(BlendMode mode);

        void BindShaderProgram(ShaderProgram* shaderProgram);
        void BindMaterial(Material* material);
        void BindMesh(Mesh* mesh);
        void UnbindMesh(Mesh* mesh);
        void DrawMesh(Mesh* mesh);

    private:
        std::shared_ptr<ShaderProgram> m_defaultShaderProgram;
        std::shared_ptr<ShaderProgram> m_default2DShaderProgram;
    };
}