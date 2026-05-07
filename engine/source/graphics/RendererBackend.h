#pragma once

namespace mnd
{

class RendererBackend
{
public:
    virtual ~RendererBackend() = default;

    virtual bool Init() = 0;
};

}  // namespace mnd
