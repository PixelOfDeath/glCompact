#pragma once
#include "glCompact/BufferInterface.hpp"
#include "glCompact/CastAnyPtr.hpp"

namespace glCompact {
    class BufferStaging;
    class BufferStaging : public BufferInterface {
        public:
            BufferStaging           () = default;
            BufferStaging           (uintptr_t size);
            BufferStaging           (const BufferStaging&  buffer);
            BufferStaging           (      BufferStaging&& buffer);
            BufferStaging& operator=(const BufferStaging&  buffer);
            BufferStaging& operator=(      BufferStaging&& buffer);
            ~BufferStaging();
            void free();

            void flushCpuCache();
            void flushCpuCache(uintptr_t offset, uintptr_t size);

            CastAnyPtr getPtr() const {return mem;}
        private:
            void* mem = 0;
    };
}
