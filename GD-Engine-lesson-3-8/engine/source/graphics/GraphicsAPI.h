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

    class GraphicsAPI
    {
    public:
        std::shared_ptr<ShaderProgram> CreateShaderProgram(const std::string& vertexSource, 
            const std::string& fragmentSource);
        GLuint CreateVertexBuffer(const std::vector<float>& vertices);
        GLuint CreateIndexBuffer(const std::vector<uint32_t>& indices);

        void SetClearColor(float r, float g, float b, float a);
        void ClearBuffers();

        void BindShaderProgram(ShaderProgram* shaderProgram);
        void BindMaterial(Material* material);
        void BindMesh(Mesh* mesh);
        void DrawMesh(Mesh* mesh);
    };
}