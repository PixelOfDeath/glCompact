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
