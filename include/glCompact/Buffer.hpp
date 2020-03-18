#pragma once
#include "glCompact/BufferInterface.hpp"

namespace glCompact {
    class Buffer;
    class Buffer : public BufferInterface {
        public:
            Buffer           () = default;
            Buffer           (bool clientMemoryCopyable, uintptr_t size);
            Buffer           (bool clientMemoryCopyable, uintptr_t size, const void* data);
            Buffer           (const Buffer&  buffer);
            Buffer           (      Buffer&& buffer);
            Buffer& operator=(const Buffer&  buffer);
            Buffer& operator=(      Buffer&& buffer);
            ~Buffer          ();
            void free();
        private:
            void create(bool clientMemoryCopyable, uintptr_t size, const void* data = 0);
    };
}
