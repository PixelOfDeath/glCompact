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
    enum class BlendEquation : int32_t {
        add             = 0x8006, //GL_FUNC_ADD
        subtract        = 0x800A, //GL_FUNC_SUBTRACT
        reverseSubtract = 0x800B, //GL_FUNC_REVERSE_SUBTRACT
        min             = 0x8007, //GL_MIN
        max             = 0x8008  //GL_MAX
    };
}
