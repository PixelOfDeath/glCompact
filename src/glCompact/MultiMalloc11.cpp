#include "glCompact/MultiMalloc11.hpp"

#if __cplusplus >= 201703L //C++17 or higher
    #include <cstdlib> //For aligned_alloc
#elif defined(__GNUC__) || defined(__clang__)
    #include <malloc.h>
#elif _MSC_VER
    #include <malloc.h>
#endif

namespace glCompact {
    void* MultiMalloc11::aligned_alloc(std::size_t alignment, std::size_t size) {
        #if __cplusplus >= 201703L //C++17 or higher
            return aligned_alloc(alignment, size);
        #elif defined(__GNUC__) || defined(__clang__)
            return memalign(alignment, size);
        #elif _MSC_VER
            return _aligned_malloc(alignment, size); //only works with _aligned_free, don't like that!
        #else
            static_assert(0, "Don't know aligned malloc for this system");
        #endif
    }

    MultiMalloc11::~MultiMalloc11() {
        free();
    }

    void MultiMalloc11::free() {
        #if __cplusplus >= 201703L //C++17 or higher
            ::free(basePtr);
        #elif defined(__GNUC__) || defined(__clang__)
            ::free(basePtr);
        #elif _MSC_VER
            _aligned_free(basePtr);
        #endif
        basePtr = 0;
    }
}
