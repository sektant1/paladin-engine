#pragma once

#include "Constants.h"
#include "Types.h"

namespace COA
{

struct RenderSettings
{
    bool useInternalRes = true;
    int  internalW      = kDefaultInternalWidth;
    int  internalH      = kDefaultInternalHeight;

    vec4 clearColor = vec4(0.0F, 0.0F, 0.0F, 1.0F);

    f32  snapX          = 320.0F;
    f32  snapY          = 240.0F;
    f32  fogStart       = 0.0F;
    f32  fogEnd         = 0.0F;
    f32  ambient        = kDefaultAmbient;
    vec3 lightDir       = vec3(0.0F, -1.0F, 0.0F);
    f32  colorDepth     = 0.0F;  ///< levels per channel; <= 1 disables quantize
    f32  ditherStrength = 0.0F;
};

}  // namespace COA
