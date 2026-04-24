#pragma once

#include <memory>

#include "Common.h"
#include "Types.h"

struct ma_engine;

namespace COA
{
class AudioManager
{
public:
    AudioManager();
    ~AudioManager();

    bool       Init();
    ma_engine *GetEngine();

    void SetListenerPosition(const vec3 &pos);

private:
    std::unique_ptr<ma_engine> m_engine;
};
}  // namespace COA
