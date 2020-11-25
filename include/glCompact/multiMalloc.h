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
    constexpr multiMallocDescriptor(void* ptr, size_t typeSize, size_t typeAlign, size_t* currentCountPtr, size_t pendingCount) :
        ptr(ptr), typeSize(typeSize), typeAlign(typeAlign), currentCountPtr(currentCountPtr), pendingCount(pendingCount){}
    template<typename T>
    constexpr multiMallocDescriptor(T* ptr, size_t* currentCountPtr, size_t pendingCount) :
        ptr(ptr), typeSize(sizeof(**ptr)), typeAlign(alignof(**ptr)), currentCountPtr(currentCountPtr), pendingCount(pendingCount){}
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
