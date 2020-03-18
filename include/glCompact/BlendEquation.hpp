///@file
#pragma once
#include <cstdint> //C++11

namespace glCompact {
    enum class BlendEquation : int32_t {
        add             = 0x8006, //GL_FUNC_ADD
        subtract        = 0x800A, //GL_FUNC_SUBTRACT
        reverseSubtract = 0x800B, //GL_FUNC_REVERSE_SUBTRACT
        min             = 0x8007, //GL_MIN
        max             = 0x8008  //GL_MAX
    };
}
