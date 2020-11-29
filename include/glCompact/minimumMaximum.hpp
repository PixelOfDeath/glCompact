#pragma once

//C++11 GCC std::max/std::min are NOT constexpr! And MSVC has stupid min/max macros that break stuff.

namespace glCompact {
    template<typename T>
    constexpr T maximum(T l, T r) {
        return (l >= r) ? l : r;
    }

    template<typename T, typename...TArgs>
    constexpr T maximum(T l, T r, TArgs...args) {
        return maximum(maximum(l, r), args...);
    }

    template<typename T>
    constexpr T minimum(T l, T r) {
        return (l <= r) ? l : r;
    }

    template<typename T, typename...TArgs>
    constexpr T minimum(T l, T r, TArgs...args) {
        return minimum(minimum(l, r), args...);
    }
}
