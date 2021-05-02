#pragma once
#include <cstdint> //C++11

namespace glCompact {
    /** @enum glCompact::Primitive
     * @brief primitive topology
     */
    enum class Primitive : int32_t {
        //vertex shader input
        points              = 0x0000, //GL_POINTS
        lines               = 0x0001, //GL_LINES
        triangles           = 0x0004, //GL_TRIANGLES

        //geometry shader input
        linesAdjacency      = 0x000A, //GL_LINES_ADJACENCY
        trianglesAdjacency  = 0x000C, //GL_TRIANGLES_ADJACENCY

        //tesselation shader
        patches             = 0x000E  //GL_PATCHES
    };
}

