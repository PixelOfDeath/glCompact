#include "glCompact/MultiMalloc11.hpp"

#if __cplusplus >= 201703L //C++17 or higher
    #include <cstdlib> //For aligned_alloc
#elif defined(__GNUC__) || defined(__clang__)
    #include <malloc.h>
#endif

namespace glCompact {
    void* MultiMalloc11::aligned_alloc(std::size_t alignment, std::size_t size) {
        #if __cplusplus >= 201703L //C++17 or higher
            return aligned_alloc(alignment, size);
        #elif defined(__GNUC__) || defined(__clang__)
            return memalign(alignment, size);
        #else
            //Just over allocate and the template will do take care of the alignment!
            return malloc(size + alignment);
        #endif
    }

    MultiMalloc11::~MultiMalloc11() {
        free();
    }

    void MultiMalloc11::free() {
        ::free(basePtr);
        basePtr = 0;
    }
}
