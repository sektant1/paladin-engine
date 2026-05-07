#version 330 core

struct Light
{
    vec3 color;
    vec3 position;
};

uniform Light uLight;
uniform vec3 uCameraPos;
uniform vec3 color;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 FragNormal;

in vec2 vUV;
in vec3 vNormal;
in vec3 vViewNormal;
in vec3 vFragPos;

uniform sampler2D baseColorTexture;

void main()
{
    vec3 norm = normalize(vNormal);

    vec3 lightDir = normalize(uLight.position - vFragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLight.color;

    vec3 viewDir = normalize(uCameraPos - vFragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    float specularStrength = 0.5;
    vec3 specular = specularStrength * spec * uLight.color;

    const float ambientStrength = 0.4;
    vec3 ambient = ambientStrength * uLight.color;

    vec4 texColor = texture(baseColorTexture, vUV);
    vec3 result = (diffuse + specular + ambient) * texColor.xyz * color;

    FragColor = vec4(result, 1.0);
    FragNormal = vec4(normalize(vViewNormal) * 0.5 + 0.5, 1.0);
}
