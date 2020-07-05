#pragma once
#include "glCompact/BufferInterface.hpp"

namespace glCompact {
    class BufferGpu;
    class BufferGpu : public BufferInterface {
        public:
            BufferGpu           () = default;
            BufferGpu           (bool clientMemoryCopyable, uintptr_t size);
            BufferGpu           (bool clientMemoryCopyable, uintptr_t size, const void* data);
            BufferGpu           (const BufferGpu&  buffer);
            BufferGpu           (      BufferGpu&& buffer);
            BufferGpu& operator=(const BufferGpu&  buffer);
            BufferGpu& operator=(      BufferGpu&& buffer);
            ~BufferGpu();
            void free();
    };
}
