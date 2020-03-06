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
//SHOULD NEVER BE INCLUDED IN HPP FILES TO NOT BLEED MACROS INTO OTHER PROJECTS!

#include <string>

#define LOOPINT(v, m_m) for(int v = 0; v < int(m_m); v++)
#define LOOPI(m_m) LOOPINT(i,m_m)
#define LOOPJ(m_m) LOOPINT(j,m_m)
#define LOOPK(m_m) LOOPINT(k,m_m)
#define LOOPL(m_m) LOOPINT(l,m_m)

namespace glCompact {
    /*constexpr int integerPowerOf(int x, int p) {
        int i = 1;
        for (int j = 1; j <= p; j++) i *= x;
        return i;
    }*/

    template<typename T>
    constexpr T align(T value, T alignment) {
        return value + ((value % alignment) ? (alignment - (value % alignment)) : 0);
    }

    std::string fileToString(const std::string& fileName);

    /*namespace glCompact {
        //char* fileToCString(const std::string filename);
    }
    */

    #if __cplusplus > 201703L //C++20
        #define LIKELY_IF  (x) if (x) [[likely]]
        #define UNLIKELY_IF(x) if (x) [[unlikely]]
    #elif defined(__GNUC__) || defined(__clang__)
        #define LIKELY_IF  (x) if (__builtin_expect((x),1))
        #define UNLIKELY_IF(x) if (__builtin_expect((x),0))
    #elif _MSC_VER
        //Until C++20 MSVC does not have this
        #define LIKELY_IF  (x) if (x)
        #define UNLIKELY_IF(x) if (x)
    #else
        #warning "LIKELY_IF/UNLIKELY_IF not implemented for this compiler"
        #define LIKELY_IF  (x) if (x)
        #define UNLIKELY_IF(x) if (x)
    #endif

    extern void crash(std::string s);
}
