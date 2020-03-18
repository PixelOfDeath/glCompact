///@file
#pragma once
#include <cstdint> //C++11

namespace glCompact {
    /** @enum glCompact::BlendFactorA
     * @brief Factor for A value
     */
    enum class BlendFactorA : int32_t {
        zero                 =      0, //GL_ZERO
        one                  =      1, //GL_ONE
                srcA         = 0x0302, //GL_SRC_ALPHA
        oneMinusSrcA         = 0x0303, //GL_ONE_MINUS_SRC_ALPHA
                dstA         = 0x0304, //GL_DST_ALPHA
        oneMinusDstA         = 0x0305, //GL_ONE_MINUS_DST_ALPHA
                constandA    = 0x8003, //GL_CONSTANT_ALPHA
        oneMinusConstantA    = 0x8004, //GL_ONE_MINUS_CONSTANT_ALPHA
                srcASaturate = 0x0308, //GL_SRC_ALPHA_SATURATE //min(srcA, 1 - dstA) ?
                src1A        = 0x8589, //GL_SRC1_ALPHA
        oneMinusSrc1A        = 0x88FB  //GL_ONE_MINUS_SRC1_ALPHA
    };
}
