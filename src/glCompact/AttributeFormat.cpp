#include "glCompact/AttributeFormat.hpp"
#include "glCompact/gl/Constants.hpp"

using namespace glCompact::gl;

namespace glCompact {
    /**
        \ingroup API
        \class glCompact::AttributeFormat
        \brief enum class for all formats that can by used as attribute input
        \details bla...
    */

    const AttributeFormatDetail AttributeFormat::attributeFormatDetail[] = {
    //TODO: Component count could also be GL_BGRA if shaderType is GL_FLOAT? Does that make a difference to 4??
    //   baseType                         componentCount/BGRA
    //   |                                |                   normalized
    //   |                                |                   |   byteSize
    //   |                                |                   |   |
        {GL_NONE,                         0,                  0,  0},

        {GL_UNSIGNED_BYTE,                1,                  1,  1}, //R8_UNORM
        {GL_BYTE,                         1,                  1,  1}, //R8_SNORM
        {GL_UNSIGNED_BYTE,                1,                  0,  1}, //R8_UINT
        {GL_BYTE,                         1,                  0,  1}, //R8_SINT
        {GL_UNSIGNED_SHORT,               1,                  1,  2}, //R16_UNORM
        {GL_SHORT,                        1,                  1,  2}, //R16_SNORM
        {GL_UNSIGNED_SHORT,               1,                  0,  2}, //R16_UINT
        {GL_SHORT,                        1,                  0,  2}, //R16_SINT
        {GL_UNSIGNED_INT,                 1,                  1,  4}, //R32_UNORM
        {GL_INT,                          1,                  1,  4}, //R32_SNORM
        {GL_UNSIGNED_INT,                 1,                  0,  4}, //R32_UINT
        {GL_INT,                          1,                  0,  4}, //R32_SINT
        {GL_HALF_FLOAT,                   1,                  0,  2}, //R16_SFLOAT
        {GL_FLOAT,                        1,                  0,  4}, //R32_SFLOAT
        {GL_DOUBLE,                       1,                  0,  8}, //R64_SFLOAT

        {GL_UNSIGNED_BYTE,                2,                  1,  2}, //R8G8_UNORM
        {GL_BYTE,                         2,                  1,  2}, //R8G8_SNORM
        {GL_UNSIGNED_BYTE,                2,                  0,  2}, //R8G8_UINT
        {GL_BYTE,                         2,                  0,  2}, //R8G8_SINT
        {GL_UNSIGNED_SHORT,               2,                  1,  4}, //R16G16_UNORM
        {GL_SHORT,                        2,                  1,  4}, //R16G16_SNORM
        {GL_UNSIGNED_SHORT,               2,                  0,  4}, //R16G16_UINT
        {GL_SHORT,                        2,                  0,  4}, //R16G16_SINT
        {GL_UNSIGNED_INT,                 2,                  1,  8}, //R32G32_UNORM
        {GL_INT,                          2,                  1,  8}, //R32G32_SNORM
        {GL_UNSIGNED_INT,                 2,                  0,  8}, //R32G32_UINT
        {GL_INT,                          2,                  0,  8}, //R32G32_SINT
        {GL_HALF_FLOAT,                   2,                  0,  4}, //R16G16_SFLOAT
        {GL_FLOAT,                        2,                  0,  8}, //R32G32_SFLOAT
        {GL_DOUBLE,                       2,                  0, 16}, //R64G64_SFLOAT

        {GL_UNSIGNED_BYTE,                3,                  1,  3}, //R8G8B8_UNORM
        {GL_BYTE,                         3,                  1,  3}, //R8G8B8_SNORM
        {GL_UNSIGNED_BYTE,                3,                  0,  3}, //R8G8B8_UINT
        {GL_BYTE,                         3,                  0,  3}, //R8G8B8_SINT
        {GL_UNSIGNED_SHORT,               3,                  1,  6}, //R16G16B16_UNORM
        {GL_SHORT,                        3,                  1,  6}, //R16G16B16_SNORM
        {GL_UNSIGNED_SHORT,               3,                  0,  6}, //R16G16B16_UINT
        {GL_SHORT,                        3,                  0,  6}, //R16G16B16_SINT
        {GL_UNSIGNED_INT,                 3,                  1, 12}, //R32G32B32_UNORM
        {GL_INT,                          3,                  1, 12}, //R32G32B32_SNORM
        {GL_UNSIGNED_INT,                 3,                  0, 12}, //R32G32B32_UINT
        {GL_INT,                          3,                  0, 12}, //R32G32B32_SINT
        {GL_HALF_FLOAT,                   3,                  0,  6}, //R16G16B16_SFLOAT
        {GL_FLOAT,                        3,                  0, 12}, //R32G32B32_SFLOAT
        {GL_DOUBLE,                       3,                  0, 24}, //R64G64B64_SFLOAT

        {GL_UNSIGNED_BYTE,                4,                  1,  4}, //R8G8B8A8_UNORM
        {GL_BYTE,                         4,                  1,  4}, //R8G8B8A8_SNORM
        {GL_UNSIGNED_BYTE,                4,                  0,  4}, //R8G8B8A8_UINT
        {GL_BYTE,                         4,                  0,  4}, //R8G8B8A8_SINT
        {GL_UNSIGNED_SHORT,               4,                  1,  8}, //R16G16B16A16_UNORM
        {GL_SHORT,                        4,                  1,  8}, //R16G16B16A16_SNORM
        {GL_UNSIGNED_SHORT,               4,                  0,  8}, //R16G16B16A16_UINT
        {GL_SHORT,                        4,                  0,  8}, //R16G16B16A16_SINT
        {GL_UNSIGNED_INT,                 4,                  1, 16}, //R32G32B32A32_UNORM
        {GL_INT,                          4,                  1, 16}, //R32G32B32A32_SNORM
        {GL_UNSIGNED_INT,                 4,                  0, 16}, //R32G32B32A32_UINT
        {GL_INT,                          4,                  0, 16}, //R32G32B32A32_SINT
        {GL_HALF_FLOAT,                   4,                  0,  8}, //R16G16B16A16_SFLOAT
        {GL_FLOAT,                        4,                  0, 16}, //R32G32B32A32_SFLOAT
        {GL_DOUBLE,                       4,                  0, 32}, //R64G64B64A64_SFLOAT

        {GL_UNSIGNED_BYTE,                GL_BGRA,            1,  4}, //B8G8R8A8_UNORM

        {GL_INT_2_10_10_10_REV,           4,                  1,  4}, //A2B10G10R10_UNORM
        {GL_UNSIGNED_INT_2_10_10_10_REV,  4,                  1,  4}, //A2B10G10R10_SNORM
        {GL_INT_2_10_10_10_REV,           GL_BGRA,            1,  4}, //A2R10G10B10_UNORM
        {GL_UNSIGNED_INT_2_10_10_10_REV,  GL_BGRA,            1,  4}, //A2R10G10B10_SNORM

        {GL_UNSIGNED_INT_10F_11F_11F_REV, 3,                  1,  4}, //B10G11R11_UFLOAT
    };
}
