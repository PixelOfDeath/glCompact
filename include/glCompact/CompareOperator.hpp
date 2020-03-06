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
    enum class CompareOperator : int32_t {
      //never          = 0x0200, //GL_NEVER //remove it because it has no use!?
        less           = 0x0201, //GL_LESS
        equal          = 0x0202, //GL_EQUAL
        lessOrEqual    = 0x0203, //GL_LEQUAL
        greater        = 0x0204, //GL_GREATER
        unequal        = 0x0205, //GL_NOTEQUAL
        greaterOrEqual = 0x0206, //GL_GEQUAL
        disabled       = 0x0207  //GL_ALWAYS
    };
}
