#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstddef> //std::max_align_t
#include <utility>
#include "glCompact/undefMsvcMinMax.hpp"

namespace glCompact {
    namespace {
        template<typename T>
        T alignTo(T value, T alignTo) {
            return (value % alignTo) ? (value + (alignTo - (value % alignTo))) : value;
        }

        //C++11 GCC std::max/std::min are NOT constexpr! And MSVC only uses stupid macros that break stuff.
        template<typename T>
        constexpr T max(T l, T r) {
            return (l >= r) ? l : r;
        }

        template<typename T>
        constexpr T min(T l, T r) {
            return (l <= r) ? l : r;
        }

        template<typename T, typename, typename, typename>
        constexpr uintptr_t MultiMalloc11ReNewGetMaxAlign() {
            return alignof(T);
        }

        template<typename T, typename, typename, typename, typename Arg0, typename... Args>
        constexpr uintptr_t MultiMalloc11ReNewGetMaxAlign() {
            return max(alignof(T), MultiMalloc11ReNewGetMaxAlign<Arg0, Args...>());
        }

        template<bool modifyPtr, typename T, typename Tinit>
        void MultiMalloc11ReNewPlacement(uintptr_t& newCurrentOffset, T*& ptr, uintptr_t oldCount, uintptr_t newCount, const Tinit initValue) {
            if (modifyPtr) {
                T* oldPtr = ptr;
                T* newPtr = reinterpret_cast<T*>(newCount ? alignTo(newCurrentOffset, alignof(T)) : newCurrentOffset);
                uintptr_t i = 0;
                for (; i < min(oldCount, newCount); ++i) new (newPtr + i)T(std::move(oldPtr[i]));
                for (; i <               newCount ; ++i) new (newPtr + i)T(initValue);
                for (; i <               oldCount ; ++i) oldPtr[i].~T();
                ptr = newCount ? newPtr : 0;
            }
            if (newCount) newCurrentOffset = alignTo(newCurrentOffset, alignof(T)) + sizeof(T) * newCount;
        }

        template<bool modifyPtr, typename T, typename Tinit, typename... Args>
        void MultiMalloc11ReNewPlacement(uintptr_t& newCurrentOffset, T*& ptr, uintptr_t oldCount, uintptr_t newCount, const Tinit initValue, Args&&... args) {
            MultiMalloc11ReNewPlacement<modifyPtr>(newCurrentOffset, ptr, oldCount, newCount, initValue);
            MultiMalloc11ReNewPlacement<modifyPtr>(newCurrentOffset, args...);
        }
    }

    class MultiMalloc11 {
        public:
            MultiMalloc11() = default;
            ~MultiMalloc11();
            template<typename T, typename Tinit, typename... Args>
            void reNew(T*& ptr, uintptr_t oldCount, uintptr_t newCount, const Tinit initValue, Args&&... args) {
                uintptr_t newCurrentOffset;
                constexpr uintptr_t maxAlign = MultiMalloc11ReNewGetMaxAlign<T, uintptr_t, uintptr_t, const Tinit, Args...>();
                MultiMalloc11ReNewPlacement<0>(newCurrentOffset = 0,                     ptr, oldCount, newCount, initValue, args...);
                void* newBasePtr = aligned_alloc(maxAlign, newCurrentOffset);
                MultiMalloc11ReNewPlacement<1>(newCurrentOffset = uintptr_t(newBasePtr), ptr, oldCount, newCount, initValue, args...);
                free();
                basePtr = newBasePtr;
            }
            void free();
        private:
            void* basePtr = 0;
            static void* aligned_alloc(std::size_t alignment, std::size_t size);
    };
}
