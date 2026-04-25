#include "graphics/GraphicsAPI.h"

#include "Constants.h"
#include "Log.h"
#include "graphics/ShaderProgram.h"
#include "render/Material.h"
#include "render/Mesh.h"

namespace COA
{

bool GraphicsAPI::Init()
{
    glEnable(GL_DEPTH_TEST);
    return true;
}

std::shared_ptr<ShaderProgram> GraphicsAPI::CreateShaderProgram(const std::string &vertexSource,
                                                                const std::string &fragmentSource)
{
    GLuint      vertexShader     = glCreateShader(GL_VERTEX_SHADER);
    const char *vertexShaderCStr = vertexSource.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderCStr, nullptr);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[kShaderInfoLogSize];
        glGetShaderInfoLog(vertexShader, kShaderInfoLogSize, nullptr, infoLog);
        LOG_ERROR("Vertex shader compilation failed: %s", infoLog);
        return nullptr;
    }

    GLuint      fragmentShader           = glCreateShader(GL_FRAGMENT_SHADER);
    const char *fragmentShaderSourceCStr = fragmentSource.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderSourceCStr, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[kShaderInfoLogSize];
        glGetShaderInfoLog(fragmentShader, kShaderInfoLogSize, nullptr, infoLog);
        LOG_ERROR("Fragment shader compilation failed: %s", infoLog);
        return nullptr;
    }

    GLuint shaderProgramID = glCreateProgram();
    glAttachShader(shaderProgramID, vertexShader);
    glAttachShader(shaderProgramID, fragmentShader);
    glLinkProgram(shaderProgramID);

    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[kShaderInfoLogSize];
        glGetProgramInfoLog(shaderProgramID, kShaderInfoLogSize, nullptr, infoLog);
        LOG_ERROR("Shader program linking failed: %s", infoLog);
        return nullptr;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    LOG_INFO("Shader program created (id=%u)", shaderProgramID);
    return std::make_shared<ShaderProgram>(shaderProgramID);
}

const std::shared_ptr<ShaderProgram> &GraphicsAPI::GetDefaultShaderProgram()
{
    if (!m_defaultShaderProgram)
    {
        std::string vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 color;
        layout(location = 2) in vec2 uv;
        layout(location = 3) in vec3 normal;

        out vec2 vUV;
        out vec3 vNormal;
        out vec3 vFragPos;

        uniform mat4 uModel;
        uniform mat4 uProjection;
        uniform mat4 uView;

        void main()
        {
            vUV = uv;

            vFragPos = vec3(uModel * vec4(position, 1.0));

            vNormal = mat3(transpose(inverse(uModel))) * normal;

            gl_Position = uProjection * uView * uModel * vec4(position, 1.0);
        }
    )";

        std::string fragmentShaderSource = R"(
        #version 330 core

        struct Light {
            vec3 color;
            vec3 position;
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
            vec3 normal = normalize(vNormal);
            
            // diffuse
            vec3 lightDir = normalize(uLight.position - vFragPos);
            float diff = max(dot(normal, lightDir), 0.0);
            vec3 ambient = 0.1 * uLight.color;
            vec3 diffuse = diff * uLight.color;
            
            // specular
            vec3 viewDir = normalize(uCameraPos - vFragPos);
            vec3 reflectDir = reflect(-lightDir, normal);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
            float specularStrenght = 0.5;
            vec3 specular = specularStrenght * spec * uLight.color;

            vec3 result = (ambient + diffuse) + specular;

            vec4 texColor = texture(baseColorTexture, vUV);

            FragColor = texColor * vec4(result, 1.0);
        }
    )";

        m_defaultShaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
    }
    return m_defaultShaderProgram;
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GraphicsAPI::BindShaderProgram(ShaderProgram *shaderProgram)
{
    if (shaderProgram)
    {
        shaderProgram->Bind();
    } else
    {
        LOG_WARN("BindShaderProgram called with nullptr");
    }
}

void GraphicsAPI::BindMaterial(Material *material)
{
    if (material)
    {
        material->Bind();
    } else
    {
        LOG_WARN("BindMaterial called with nullptr");
    }
}

void GraphicsAPI::BindMesh(Mesh *mesh)
{
    if (mesh)
    {
        mesh->Bind();
    } else
    {
        LOG_WARN("BindMesh called with nullptr");
    }
}

void GraphicsAPI::UnbindMesh(Mesh *mesh)
{
    if (mesh)
    {
        mesh->Unbind();
    } else
    {
        LOG_WARN("BindMesh called with nullptr");
    }
}

void GraphicsAPI::DrawMesh(Mesh *mesh)
{
    if (mesh)
    {
        mesh->Draw();
    } else
    {
        LOG_WARN("DrawMesh called with nullptr");
    }
}

}  // namespace COA
