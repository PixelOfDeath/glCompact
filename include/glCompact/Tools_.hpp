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

    #if __cplusplus > 201703L //C++20 (this actually just tests if __cplusplus is higher then C++17)
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

    #if defined(__GNUC__) || defined(__clang__)
        #define PURE_FUNCTION __attribute__((pure))
    #elif _MSC_VER
        //MSVC does not support defining functions as pure
        #define PURE_FUNCTION
    #else
        #warning "PURE_FUNCTION not implemented for this compiler"
        #define PURE_FUNCTION
    #endif

    [[noreturn]] extern void crash(std::string s);
}
