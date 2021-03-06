#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "glCompact/multiMalloc.h"

static size_t minimum(size_t a, size_t b) {
    return a < b ? a : b;
}

static size_t maximum(size_t a, size_t b) {
    return a > b ? a : b;
}

static size_t raiseToAlign(size_t value, size_t alignment) {
    assert(alignment && !(alignment & (alignment - 1))); //alignment must be >0 and a power of two value
    return (value + alignment - 1) & ~(alignment - 1);
}

static void* multiReMalloc_(void* currentBasePtr, _Bool growOnly, const struct multiMallocDescriptor* md, size_t descriptorSizeInByte/*, void** initValue*/) {
    const int mdCount = descriptorSizeInByte / sizeof(struct multiMallocDescriptor);

    _Bool changes = 0;
    size_t mallocSize = 0;
    for (int i = 0; i < mdCount; ++i) {
        const struct multiMallocDescriptor* d = &md[i];
        size_t currentCount = currentBasePtr && d->currentCountPtr ? *d->currentCountPtr : 0;
        size_t pendingCount = growOnly ? maximum(currentCount, d->pendingCount) : d->pendingCount;
        if (currentCount != pendingCount) changes = 1;
        if (pendingCount) mallocSize = raiseToAlign(mallocSize, d->typeAlign) + d->typeSize * pendingCount;
    }
    if (!changes) return currentBasePtr;

    void* pendingBasePtr = malloc(mallocSize);
    size_t currentPtr = (size_t)pendingBasePtr;
    for (int i = 0; i < mdCount; ++i) {
        const struct multiMallocDescriptor* d = &md[i];
        size_t currentCount = currentBasePtr && d->currentCountPtr ? *d->currentCountPtr : 0;
        size_t pendingCount = growOnly ? maximum(currentCount, d->pendingCount) : d->pendingCount;
        if (pendingCount) {
            currentPtr = raiseToAlign(currentPtr, d->typeAlign);
            size_t copyCount = currentBasePtr && d->ptr ? minimum(currentCount, pendingCount) : 0;
            size_t nullCount = copyCount < pendingCount ? pendingCount - copyCount : 0;
            memcpy((void*)(currentPtr                          ), *(void**)d->ptr, d->typeSize * copyCount);
            memset((void*)(currentPtr + d->typeSize * copyCount),               0, d->typeSize * nullCount);
            *(void**)d->ptr = (void*)currentPtr;
            currentPtr += d->typeSize * pendingCount;
        } else {
            *(void**)d->ptr = 0;
        }
    }

    //we set the counters last, because different multiMallocDescriptor might reference the same counter
    for (int i = 0; i < mdCount; ++i) {
        const struct multiMallocDescriptor* d = &md[i];
        size_t currentCount = currentBasePtr && d->currentCountPtr ? *d->currentCountPtr : 0;
        size_t pendingCount = growOnly ? maximum(currentCount, d->pendingCount) : d->pendingCount;
        if (d->currentCountPtr) *d->currentCountPtr = pendingCount;
    }
    free(currentBasePtr);
    return pendingBasePtr;
}

void* multiMalloc          (                      const struct multiMallocDescriptor* md, size_t descriptorSizeInByte) {
    return multiReMalloc_(             0, 0, md, descriptorSizeInByte);
}

void* multiReMalloc        (void* currentBasePtr, const struct multiMallocDescriptor* md, size_t descriptorSizeInByte) {
    return multiReMalloc_(currentBasePtr, 0, md, descriptorSizeInByte);
}

void* multiReMallocGrowOnly(void* currentBasePtr, const struct multiMallocDescriptor* md, size_t descriptorSizeInByte) {
    return multiReMalloc_(currentBasePtr, 1, md, descriptorSizeInByte);
}
