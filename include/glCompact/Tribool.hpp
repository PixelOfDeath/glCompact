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
#pragma once
#include <cstdint> //C++11

namespace glCompact {
    class Tribool {
        public:
            constexpr Tribool():value(2){}
            constexpr Tribool(bool value):value(value){}
            constexpr Tribool(const Tribool& tribool):value(tribool.value){}
            bool isFalse  () const {return value == 0;}
            bool isTrue   () const {return value == 1;}
            bool isUnknown() const {return value == 2;}
        private:
            uint8_t value;
    };
}
