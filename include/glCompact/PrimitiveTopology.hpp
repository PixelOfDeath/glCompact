/*
    This file is part of glCompact.
    Copyright (C) 2019-2020 Frederik Uje vom Hofe

    glCompact is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    glCompact is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <https://www.gnu.org/licenses/>.
*/
///@file
#pragma once
#include <cstdint> //C++11

namespace glCompact {
    /** @enum glCompact::PrimitiveTopology
     * @brief primitive topology, at the momenty only used for vertex stage input
     */
    enum class PrimitiveTopology : int32_t {
        pointList                  = 0x0000, //GL_POINTS,
        lineList                   = 0x0001, //GL_LINES,
      //lineLoop                   = 0x0002, //GL_LINE_LOOP, //not supported in vulkan
        lineStrip                  = 0x0003, //GL_LINE_STRIP,
        triangleList               = 0x0004, //GL_TRIANGLES,
        triangleStrip              = 0x0005, //GL_TRIANGLE_STRIP,
        triangleFan                = 0x0006, //GL_TRIANGLE_FAN, //not supported by metal

        //Geometry shader
        lineListWithAdjacency      = 0x000A, //GL_LINES_ADJACENCY,
        lineStripWithAdjacency     = 0x000B, //GL_LINE_STRIP_ADJACENCY,
        triangleListWithAdjacency  = 0x000C, //GL_TRIANGLES_ADJACENCY,
        triangleStripWithAdjacency = 0x000D, //GL_TRIANGLE_STRIP_ADJACENCY,

        //Tesselation shader
        patchList                  = 0x000E  //GL_PATCHES
    };
}
