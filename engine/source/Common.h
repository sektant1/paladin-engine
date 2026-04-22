#pragma once

#include "Types.h"
#include "render/Material.h"

namespace ENG
{

struct CameraData
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec3 position;
};

struct LightData
{
    vec3 color;
    vec3 position;
};
}  // namespace ENG
