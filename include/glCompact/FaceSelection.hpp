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
    enum class FaceSelection : int32_t {
        front        = 0x0404, //GL_FRONT,
        back         = 0x0405, //GL_BACK,
        frontAndBack = 0x0408  //GL_FRONT_AND_BACK
    };
}
