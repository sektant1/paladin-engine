#include "graphics/GraphicsAPI.h"
#include "graphics/ShaderProgram.h"
#include "render/Material.h"
#include "render/Mesh.h"
#include <iostream>

namespace eng
{
    bool GraphicsAPI::Init()
    {
        glEnable(GL_DEPTH_TEST);
        return true;
    }

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

    const std::shared_ptr<ShaderProgram>& GraphicsAPI::GetDefaultShaderProgram()
    {
        if (!m_defaultShaderProgram)
        {
            std::string vertexShaderSource = R"(
            #version 330 core
            layout (location = 0) in vec3 position;
            layout (location = 1) in vec3 color;
            layout (location = 2) in vec2 uv;
            layout (location = 3) in vec3 normal;
        
            out vec2 vUV;
            out vec3 vNormal;
            out vec3 vFragPos;
        
            uniform mat4 uModel;
            uniform mat4 uView;
            uniform mat4 uProjection;
        
            void main()
            {
                vUV = uv;
                vNormal = normalize(transpose(inverse(mat3(uModel))) * normal);
                vFragPos = vec3(uModel * vec4(position, 1.0));
                gl_Position = uProjection * uView * uModel * vec4(position, 1.0);
            }
            )";

            std::string fragmentShaderSource = R"(
            #version 330 core

            struct Light
            {
                vec3 color;
                vec3 direction;
            };

            uniform Light uLight;
            uniform vec3 uCameraPos;

            out vec4 FragColor;

            in vec2 vUV;
            in vec3 vNormal;
            in vec3 vFragPos;

            uniform sampler2D baseColorTexture;

            void main()
            {
                vec3 norm = normalize(vNormal);
                
                // diffuse
                vec3 lightDir = normalize(-uLight.direction);
                float diff = max(dot(norm, lightDir), 0.0);
                vec3 diffuse = diff * uLight.color;

                // specular
                vec3 viewDir = normalize(uCameraPos - vFragPos);
                vec3 redlectDir = reflect(-lightDir, norm);
                float spec = pow(max(dot(viewDir, redlectDir), 0.0), 32.0);
                float specularStrength = 0.5;
                vec3 specular = specularStrength * spec * uLight.color;
                
                // ambient
                const float ambientStrength = 0.4;
                vec3 ambient = ambientStrength * uLight.color;
    
                vec3 result = diffuse + specular + ambient;

                vec4 texColor = texture(baseColorTexture, vUV);

                FragColor = texColor * vec4(result, 1.0);
            }
            )";

            m_defaultShaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
        }

        return m_defaultShaderProgram;
    }

    const std::shared_ptr<ShaderProgram>& GraphicsAPI::GetDefault2DShaderProgram()
    {
        if (!m_default2DShaderProgram)
        {
            std::string vertexShaderSource = R"(
            #version 330 core
            layout (location = 0) in vec2 position;
        
            out vec2 vUV;
        
            uniform mat4 uModel;
            uniform mat4 uView;
            uniform mat4 uProjection;

            uniform vec2 uPivot;
            uniform vec2 uSize;    

            uniform vec2 uUVMin;
            uniform vec2 uUVMax;  
        
            void main()
            {
                vec2 local = (position - uPivot) * uSize;
                vUV = mix(uUVMin, uUVMax, position);
                
                gl_Position = uProjection * uView * uModel * vec4(local, 0.0, 1.0);
            }
            )";

            std::string fragmentShaderSource = R"(
            #version 330 core

            in vec2 vUV;

            uniform vec4 uColor;

            uniform sampler2D uTex;

            out vec4 FragColor;

            void main()
            {
                vec4 src = texture(uTex, vUV) * uColor;
                FragColor = src;
            }
            )";

            m_default2DShaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
        }
        return m_default2DShaderProgram;
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
        glClear(GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT);
    }

    void GraphicsAPI::SetDepthTestEnabled(bool enabled)
    {
        if (enabled)
        {
            glEnable(GL_DEPTH_TEST);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }
    }

    void GraphicsAPI::SetBlendMode(BlendMode mode)
    {
        switch (mode)
        {
        case BlendMode::Disabled:
        {
            glDisable(GL_BLEND);
        }
        break;
        case BlendMode::Alpha:
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        break;
        case BlendMode::Additive:
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
        }
        break;
        case BlendMode::Multiply:
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR, GL_ZERO);
        }
        break;
        default:
        {
            glDisable(GL_BLEND);
        }
            break;
        }
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

    void GraphicsAPI::UnbindMesh(Mesh* mesh)
    {
        if (mesh)
        {
            mesh->Unbind();
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