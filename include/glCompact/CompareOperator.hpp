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
