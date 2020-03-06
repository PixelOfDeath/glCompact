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
