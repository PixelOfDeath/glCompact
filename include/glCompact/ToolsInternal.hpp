#pragma once
//SHOULD NEVER BE INCLUDED IN HPP FILES TO NOT BLEED MACROS INTO OTHER PROJECTS!

#include <string>
#include <cstddef> //std::max_align_t

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

    template<typename T>
    T alignTo(T value, T alignTo) {
        return (value % alignTo) ? (value + (alignTo - (value % alignTo))) : value;
    }

    //C++11 std::max is NOT constexpr!
    template<typename T>
    constexpr T max(T l, T r) {
        return (l >= r) ? l : r;
    }

    namespace {
        template<bool modifiyPtr, typename T, typename Tinit>
        void multiReNewPlacement(uintptr_t& oldCurrentOffset, uintptr_t& newCurrentOffset, T*& ptr, uintptr_t oldCount, uintptr_t newCount, const Tinit initValue) {
            if (modifiyPtr) {
                T* oldPtr = reinterpret_cast<T*>(oldCount ? alignTo(oldCurrentOffset, alignof(T)) : oldCurrentOffset);
                T* newPtr = reinterpret_cast<T*>(newCount ? alignTo(newCurrentOffset, alignof(T)) : newCurrentOffset);
                uintptr_t i = 0;
                for (; i < std::min(oldCount, newCount); ++i) new (newPtr + i)T(std::move(oldPtr[i]));
                for (; i <                    newCount ; ++i) new (newPtr + i)T(initValue);
                for (; i <                    oldCount ; ++i) oldPtr[i].~T();
                ptr = newPtr;
            }
            if (oldCount) oldCurrentOffset = alignTo(oldCurrentOffset, alignof(T)) + sizeof(T) * oldCount;
            if (newCount) newCurrentOffset = alignTo(newCurrentOffset, alignof(T)) + sizeof(T) * newCount;
        }

        template<bool modifiyPtr, typename T, typename Tinit, typename... Args>
        void multiReNewPlacement(uintptr_t& oldCurrentOffset, uintptr_t& newCurrentOffset, T*& ptr, uintptr_t oldCount, uintptr_t newCount, const Tinit initValue, Args&&... args) {
            multiReNewPlacement<modifiyPtr>(oldCurrentOffset, newCurrentOffset, ptr, oldCount, newCount, initValue);
            multiReNewPlacement<modifiyPtr>(oldCurrentOffset, newCurrentOffset, args...);
        }

        template<typename T, typename, typename, typename>
        constexpr uintptr_t multiReNewGetMaxAlign() {
            return alignof(T);
        }

        template<typename T, typename, typename, typename, typename Arg0, typename... Args>
        constexpr uintptr_t multiReNewGetMaxAlign() {
            return max(alignof(T), multiReNewGetMaxAlign<Arg0, Args...>());
        }
    }

    //Currently this function is limited to alignof(std::max_align_t)
    template<typename T, typename Tinit, typename... Args>
    void multiReNew(T*& ptr, uintptr_t oldCount, uintptr_t newCount, const Tinit initValue, Args&&... args) {
        void* firstPtr = ptr;
        uintptr_t oldCurrentOffset, newCurrentOffset;
        constexpr uintptr_t maxAlign = multiReNewGetMaxAlign<T, uintptr_t, uintptr_t, const Tinit, Args...>();
        static_assert(maxAlign <= alignof(std::max_align_t), "Only support alignment up to alignof(std::max_align_t)");
        multiReNewPlacement<0>(oldCurrentOffset = 0,                   newCurrentOffset = 0,                                   ptr, oldCount, newCount, initValue, args...);
        multiReNewPlacement<1>(oldCurrentOffset = uintptr_t(firstPtr), newCurrentOffset = uintptr_t(malloc(newCurrentOffset)), ptr, oldCount, newCount, initValue, args...);
        free(firstPtr);
    }
}
