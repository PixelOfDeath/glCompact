#pragma once

//stupid MSVC macros breaking stuff all over the place
#ifdef WIN32
   #undef min
   #undef max
#endif

namespace glCompact {
    //C++11 GCC std::max/std::min are NOT constexpr! And MSVC only uses stupid macros that break stuff.
    template<typename T>
    constexpr T max(T l, T r) {
        return (l >= r) ? l : r;
    }

    template<typename T>
    constexpr T min(T l, T r) {
        return (l <= r) ? l : r;
    }
}
