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
    vec3 ambient = 0.4 * uLight.color;
    vec3 diffuse = diff * uLight.color;

    // specular
    vec3 viewDir = normalize(uCameraPos - vFragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    float specularStrenght = 0.5;
    vec3 specular = specularStrenght * spec * uLight.color;

    vec3 result = diffuse + specular + ambient;

    vec4 texColor = texture(baseColorTexture, vUV);

    FragColor = texColor * vec4(result, 1.0);
}
