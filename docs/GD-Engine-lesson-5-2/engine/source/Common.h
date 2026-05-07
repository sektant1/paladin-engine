#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace eng
{
    struct CameraData
    {
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
        glm::mat4 orthoMatrix;
        glm::vec3 position;
    };

    struct LightData
    {
        glm::vec3 color;
        glm::vec3 position;
    };
}