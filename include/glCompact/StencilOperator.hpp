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
