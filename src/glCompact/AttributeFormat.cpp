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

        {GL_UNSIGNED_BYTE,                1,                  1,  1},
        {GL_BYTE,                         1,                  1,  1},
        {GL_UNSIGNED_BYTE,                1,                  0,  1},
        {GL_BYTE,                         1,                  0,  1},
        {GL_UNSIGNED_SHORT,               1,                  1,  2},
        {GL_SHORT,                        1,                  1,  2},
        {GL_UNSIGNED_SHORT,               1,                  0,  2},
        {GL_SHORT,                        1,                  0,  2},
        {GL_UNSIGNED_INT,                 1,                  1,  4},
        {GL_INT,                          1,                  1,  4},
        {GL_UNSIGNED_INT,                 1,                  0,  4},
        {GL_INT,                          1,                  0,  4},
        {GL_HALF_FLOAT,                   1,                  0,  2},
        {GL_FLOAT,                        1,                  0,  4},
        {GL_DOUBLE,                       1,                  0,  8},

        {GL_UNSIGNED_BYTE,                2,                  1,  2},
        {GL_BYTE,                         2,                  1,  2},
        {GL_UNSIGNED_BYTE,                2,                  0,  2},
        {GL_BYTE,                         2,                  0,  2},
        {GL_UNSIGNED_SHORT,               2,                  1,  4},
        {GL_SHORT,                        2,                  1,  4},
        {GL_UNSIGNED_SHORT,               2,                  0,  4},
        {GL_SHORT,                        2,                  0,  4},
        {GL_UNSIGNED_INT,                 2,                  1,  8},
        {GL_INT,                          2,                  1,  8},
        {GL_UNSIGNED_INT,                 2,                  0,  8},
        {GL_INT,                          2,                  0,  8},
        {GL_HALF_FLOAT,                   2,                  0,  4},
        {GL_FLOAT,                        2,                  0,  8},
        {GL_DOUBLE,                       2,                  0, 16},

        {GL_UNSIGNED_BYTE,                3,                  1,  3},
        {GL_BYTE,                         3,                  1,  3},
        {GL_UNSIGNED_BYTE,                3,                  0,  3},
        {GL_BYTE,                         3,                  0,  3},
        {GL_UNSIGNED_SHORT,               3,                  1,  6},
        {GL_SHORT,                        3,                  1,  6},
        {GL_UNSIGNED_SHORT,               3,                  0,  6},
        {GL_SHORT,                        3,                  0,  6},
        {GL_UNSIGNED_INT,                 3,                  1, 12},
        {GL_INT,                          3,                  1, 12},
        {GL_UNSIGNED_INT,                 3,                  0, 12},
        {GL_INT,                          3,                  0, 12},
        {GL_HALF_FLOAT,                   3,                  0,  6},
        {GL_FLOAT,                        3,                  0, 12},
        {GL_DOUBLE,                       3,                  0, 24},

        {GL_UNSIGNED_BYTE,                4,                  1,  4},
        {GL_BYTE,                         4,                  1,  4},
        {GL_UNSIGNED_BYTE,                4,                  0,  4},
        {GL_BYTE,                         4,                  0,  4},
        {GL_UNSIGNED_SHORT,               4,                  1,  8},
        {GL_SHORT,                        4,                  1,  8},
        {GL_UNSIGNED_SHORT,               4,                  0,  8},
        {GL_SHORT,                        4,                  0,  8},
        {GL_UNSIGNED_INT,                 4,                  1, 16},
        {GL_INT,                          4,                  1, 16},
        {GL_UNSIGNED_INT,                 4,                  0, 16},
        {GL_INT,                          4,                  0, 16},
        {GL_HALF_FLOAT,                   4,                  0,  8},
        {GL_FLOAT,                        4,                  0, 16},
        {GL_DOUBLE,                       4,                  0, 32},

        {GL_UNSIGNED_BYTE,                GL_BGRA,            1,  4},

        {GL_INT_2_10_10_10_REV,           4,                  1,  4},
        {GL_UNSIGNED_INT_2_10_10_10_REV,  4,                  1,  4},
        {GL_INT_2_10_10_10_REV,           GL_BGRA,            1,  4},
        {GL_UNSIGNED_INT_2_10_10_10_REV,  GL_BGRA,            1,  4},

        {GL_UNSIGNED_INT_10F_11F_11F_REV, 3,                  1,  4},
    };
}
