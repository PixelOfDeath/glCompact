///@file
#pragma once
#include <cstdint> //C++11

namespace glCompact {
    enum class StencilOperator : int32_t {
        keep      = 0x1E00, //GL_KEEP,
        zero      =      0, //GL_ZERO,
        replace   = 0x1E01, //GL_REPLACE,
        incClamp  = 0x1E02, //GL_INCR,
        incWrap   = 0x8507, //GL_INCR_WRAP,
        decClamp  = 0x1E03, //GL_DECR,
        decWrap   = 0x8508, //GL_DECR_WRAP,
        bitInvert = 0x150A  //GL_INVERT //bit invert!
    };
}
