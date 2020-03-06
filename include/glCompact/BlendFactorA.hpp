/*
    glCompact
    Copyright (C) 2019-2020 Frederik Uje vom Hofe

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
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
