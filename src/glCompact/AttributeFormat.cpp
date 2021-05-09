#include "glCompact/AttributeFormat.hpp"
#include "glCompact/gl/Constants.hpp"

using namespace glCompact::gl;

namespace glCompact {
    /**
        \ingroup API
        \class glCompact::AttributeFormat
        \brief enum class for all formats that can by used as attribute input
        \details Each location in an attribute layout uses one of this formats listed here.

        UNORM = unsigned integer normalized in GLSL to 0..1
        SNORM = signed integer normalized in GLSL to -1..1
        UFLOAT = unsigned float
        SFLOAT = signed float

        There are up to 4 components per formate. Each component is followed by its bit count.

        For example:

        R8G8_UNORM = Each component is an unsigned 8 bit integer. Presented in GLSL as normalized float in the range of 0..1. Making this a 16 bit format.
    */

    const AttributeFormatDetail AttributeFormat::detail() const {
        switch (formatEnum) {
            case R8_UNORM           : return {GL_UNSIGNED_BYTE,                1,                  1,  1};
            case R8_SNORM           : return {GL_BYTE,                         1,                  1,  1};
            case R8_UINT            : return {GL_UNSIGNED_BYTE,                1,                  0,  1};
            case R8_SINT            : return {GL_BYTE,                         1,                  0,  1};
            case R16_UNORM          : return {GL_UNSIGNED_SHORT,               1,                  1,  2};
            case R16_SNORM          : return {GL_SHORT,                        1,                  1,  2};
            case R16_UINT           : return {GL_UNSIGNED_SHORT,               1,                  0,  2};
            case R16_SINT           : return {GL_SHORT,                        1,                  0,  2};
            case R32_UNORM          : return {GL_UNSIGNED_INT,                 1,                  1,  4};
            case R32_SNORM          : return {GL_INT,                          1,                  1,  4};
            case R32_UINT           : return {GL_UNSIGNED_INT,                 1,                  0,  4};
            case R32_SINT           : return {GL_INT,                          1,                  0,  4};
            case R16_SFLOAT         : return {GL_HALF_FLOAT,                   1,                  0,  2};
            case R32_SFLOAT         : return {GL_FLOAT,                        1,                  0,  4};
            case R64_SFLOAT         : return {GL_DOUBLE,                       1,                  0,  8};

            case R8G8_UNORM         : return {GL_UNSIGNED_BYTE,                2,                  1,  2};
            case R8G8_SNORM         : return {GL_BYTE,                         2,                  1,  2};
            case R8G8_UINT          : return {GL_UNSIGNED_BYTE,                2,                  0,  2};
            case R8G8_SINT          : return {GL_BYTE,                         2,                  0,  2};
            case R16G16_UNORM       : return {GL_UNSIGNED_SHORT,               2,                  1,  4};
            case R16G16_SNORM       : return {GL_SHORT,                        2,                  1,  4};
            case R16G16_UINT        : return {GL_UNSIGNED_SHORT,               2,                  0,  4};
            case R16G16_SINT        : return {GL_SHORT,                        2,                  0,  4};
            case R32G32_UNORM       : return {GL_UNSIGNED_INT,                 2,                  1,  8};
            case R32G32_SNORM       : return {GL_INT,                          2,                  1,  8};
            case R32G32_UINT        : return {GL_UNSIGNED_INT,                 2,                  0,  8};
            case R32G32_SINT        : return {GL_INT,                          2,                  0,  8};
            case R16G16_SFLOAT      : return {GL_HALF_FLOAT,                   2,                  0,  4};
            case R32G32_SFLOAT      : return {GL_FLOAT,                        2,                  0,  8};
            case R64G64_SFLOAT      : return {GL_DOUBLE,                       2,                  0, 16};

            case R8G8B8_UNORM       : return {GL_UNSIGNED_BYTE,                3,                  1,  3};
            case R8G8B8_SNORM       : return {GL_BYTE,                         3,                  1,  3};
            case R8G8B8_UINT        : return {GL_UNSIGNED_BYTE,                3,                  0,  3};
            case R8G8B8_SINT        : return {GL_BYTE,                         3,                  0,  3};
            case R16G16B16_UNORM    : return {GL_UNSIGNED_SHORT,               3,                  1,  6};
            case R16G16B16_SNORM    : return {GL_SHORT,                        3,                  1,  6};
            case R16G16B16_UINT     : return {GL_UNSIGNED_SHORT,               3,                  0,  6};
            case R16G16B16_SINT     : return {GL_SHORT,                        3,                  0,  6};
            case R32G32B32_UNORM    : return {GL_UNSIGNED_INT,                 3,                  1, 12};
            case R32G32B32_SNORM    : return {GL_INT,                          3,                  1, 12};
            case R32G32B32_UINT     : return {GL_UNSIGNED_INT,                 3,                  0, 12};
            case R32G32B32_SINT     : return {GL_INT,                          3,                  0, 12};
            case R16G16B16_SFLOAT   : return {GL_HALF_FLOAT,                   3,                  0,  6};
            case R32G32B32_SFLOAT   : return {GL_FLOAT,                        3,                  0, 12};
            case R64G64B64_SFLOAT   : return {GL_DOUBLE,                       3,                  0, 24};

            case R8G8B8A8_UNORM     : return {GL_UNSIGNED_BYTE,                4,                  1,  4};
            case R8G8B8A8_SNORM     : return {GL_BYTE,                         4,                  1,  4};
            case R8G8B8A8_UINT      : return {GL_UNSIGNED_BYTE,                4,                  0,  4};
            case R8G8B8A8_SINT      : return {GL_BYTE,                         4,                  0,  4};
            case R16G16B16A16_UNORM : return {GL_UNSIGNED_SHORT,               4,                  1,  8};
            case R16G16B16A16_SNORM : return {GL_SHORT,                        4,                  1,  8};
            case R16G16B16A16_UINT  : return {GL_UNSIGNED_SHORT,               4,                  0,  8};
            case R16G16B16A16_SINT  : return {GL_SHORT,                        4,                  0,  8};
            case R32G32B32A32_UNORM : return {GL_UNSIGNED_INT,                 4,                  1, 16};
            case R32G32B32A32_SNORM : return {GL_INT,                          4,                  1, 16};
            case R32G32B32A32_UINT  : return {GL_UNSIGNED_INT,                 4,                  0, 16};
            case R32G32B32A32_SINT  : return {GL_INT,                          4,                  0, 16};
            case R16G16B16A16_SFLOAT: return {GL_HALF_FLOAT,                   4,                  0,  8};
            case R32G32B32A32_SFLOAT: return {GL_FLOAT,                        4,                  0, 16};
            case R64G64B64A64_SFLOAT: return {GL_DOUBLE,                       4,                  0, 32};

            case B8G8R8A8_UNORM     : return {GL_UNSIGNED_BYTE,                GL_BGRA,            1,  4};

            case A2B10G10R10_UNORM  : return {GL_INT_2_10_10_10_REV,           4,                  1,  4};
            case A2B10G10R10_SNORM  : return {GL_UNSIGNED_INT_2_10_10_10_REV,  4,                  1,  4};
            case A2R10G10B10_UNORM  : return {GL_INT_2_10_10_10_REV,           GL_BGRA,            1,  4};
            case A2R10G10B10_SNORM  : return {GL_UNSIGNED_INT_2_10_10_10_REV,  GL_BGRA,            1,  4};

            case B10G11R11_UFLOAT   : return {GL_UNSIGNED_INT_10F_11F_11F_REV, 3,                  1,  4};
        }
                                      return {GL_NONE,                         0,                  0,  0};
    }
}
