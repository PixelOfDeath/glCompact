///@file
#pragma once
#include <cstdint> //C++11

namespace glCompact {
    /** @enum glCompact::BlendFactorRgb
     * @brief Factor for RGB value
     */
    enum class BlendFactorRgb : int32_t {
        zero                 =      0, //GL_ZERO,                     ///< factor is 0, so end result is also 00
        one                  =      1, //GL_ONE,                      ///< factor is 1, so end result is exactly the same
                srcRgb       = 0x0300, //GL_SRC_COLOR,                ///< stuff
        oneMinusSrcRgb       = 0x0301, //GL_ONE_MINUS_SRC_COLOR,      ///< stuff
                dstRgb       = 0x0306, //GL_DST_COLOR,                ///< stuff
        oneMinusdstRgb       = 0x0307, //GL_ONE_MINUS_DST_COLOR,      ///< stuff
                srcA         = 0x0302, //GL_SRC_ALPHA,                ///< stuff
        oneMinusSrcA         = 0x0303, //GL_ONE_MINUS_SRC_ALPHA,      ///< stuff
                dstA         = 0x0304, //GL_DST_ALPHA,                ///< stuff
        oneMinusDstA         = 0x0305, //GL_ONE_MINUS_DST_ALPHA,      ///< stuff
                constantRgb  = 0x8001, //GL_CONSTANT_COLOR,           ///< stuff
        oneMinusConstantRgb  = 0x8002, //GL_ONE_MINUS_CONSTANT_COLOR, ///< stuff
                constandA    = 0x8003, //GL_CONSTANT_ALPHA,           ///< stuff
        oneMinusConstantA    = 0x8004, //GL_ONE_MINUS_CONSTANT_ALPHA, ///< stuff
                srcASaturate = 0x0308, //GL_SRC_ALPHA_SATURATE,       ///< stuff min(srcA, 1 - dstA) ?
                src1Rgb      = 0x88F9, //GL_SRC1_COLOR,               ///< stuff
        oneMinusSrc1Rgb      = 0x88FA, //GL_ONE_MINUS_SRC1_COLOR,     ///< stuff
                src1A        = 0x8589, //GL_SRC1_ALPHA,               ///< stuff
        oneMinusSrc1A        = 0x88FB  //GL_ONE_MINUS_SRC1_ALPHA      ///< stuff
    };
}
