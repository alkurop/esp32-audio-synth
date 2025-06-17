#pragma once
#include <cstdint>
 

namespace sound_module
{
    struct Stereo
    {
        float left = 0.0f;
        float right = 0.0f;

        Stereo() = default;
        Stereo(float l, float r) : left(l), right(r) {}
    };
}
