#pragma once
#include <stddef.h>

struct multiMallocDescriptor {
    void*   ptr;
    size_t  typeSize;
    size_t  typeAlign;
    size_t* currentCountPtr;
    size_t  pendingCount;

    #ifdef __cplusplus
    multiMallocDescriptor() = default;
    template<typename T>
    multiMallocDescriptor(T** ptr, size_t typeSize, size_t typeAlign, size_t* currentCountPtr, size_t pendingCount) :
        ptr(ptr), typeSize(typeSize), typeAlign(typeAlign), currentCountPtr(currentCountPtr), pendingCount(pendingCount) {
            assert(sizeof (T) == typeSize);
            assert(alignof(T) == typeAlign);
    }
    template<typename T>
    constexpr multiMallocDescriptor(T** ptr, size_t* currentCountPtr, size_t pendingCount) :
        ptr(reinterpret_cast<void*>(ptr)), typeSize(sizeof(T)), typeAlign(alignof(T)), currentCountPtr(currentCountPtr), pendingCount(pendingCount){}
    #endif
};

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC extern
#endif

EXTERNC void* multiMalloc          (                      const struct multiMallocDescriptor* md, size_t descriptorSizeInByte);
EXTERNC void* multiReMalloc        (void* currentBasePtr, const struct multiMallocDescriptor* md, size_t descriptorSizeInByte);
EXTERNC void* multiReMallocGrowOnly(void* currentBasePtr, const struct multiMallocDescriptor* md, size_t descriptorSizeInByte);

#undef EXTERNC
