#version 330 core

// Lit fragment for skinned meshes. Matches the default shader's surface
// model so a skinned character drops in next to static geometry without
// a visual mismatch.

struct Light {
    vec3 color;
    vec3 position;
};

uniform Light uLight;
uniform vec3  uCameraPos;
uniform vec3  color;

in vec2 vUV;
in vec3 vNormal;
in vec3 vFragPos;

uniform sampler2D baseColorTexture;

out vec4 FragColor;

void main()
{
    vec3 normal = normalize(vNormal);

    vec3  lightDir = normalize(uLight.position - vFragPos);
    float diff     = max(dot(normal, lightDir), 0.0);
    vec3  ambient  = 0.4 * uLight.color;
    vec3  diffuse  = diff * uLight.color;

    vec3  viewDir    = normalize(uCameraPos - vFragPos);
    vec3  reflectDir = reflect(-lightDir, normal);
    float spec       = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3  specular   = 0.5 * spec * uLight.color;

    vec4 texColor = texture(baseColorTexture, vUV);
    vec3 result   = (diffuse + specular + ambient) * texColor.xyz * color;

    FragColor = vec4(result, 1.0);
}
