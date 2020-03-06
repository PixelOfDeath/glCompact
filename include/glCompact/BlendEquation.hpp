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
    enum class BlendEquation : int32_t {
        add             = 0x8006, //GL_FUNC_ADD
        subtract        = 0x800A, //GL_FUNC_SUBTRACT
        reverseSubtract = 0x800B, //GL_FUNC_REVERSE_SUBTRACT
        min             = 0x8007, //GL_MIN
        max             = 0x8008  //GL_MAX
    };
}
