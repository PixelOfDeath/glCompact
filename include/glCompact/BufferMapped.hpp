#pragma once
#include "glCompact/BufferInterface.hpp"
#include "glCompact/CastAnyPtr.hpp"

namespace glCompact {
    class BufferMapped;
    class BufferMapped : public BufferInterface {
        public:
            BufferMapped           () = default;
            BufferMapped           (uintptr_t size);
            BufferMapped           (const BufferMapped&  buffer);
            BufferMapped           (      BufferMapped&& buffer);
            BufferMapped& operator=(const BufferMapped&  buffer);
            BufferMapped& operator=(      BufferMapped&& buffer);
            ~BufferMapped();
            void free();

            void flushMappedMemoryWrites();
            void flushMappedMemoryWrites(uintptr_t offset, uintptr_t size);

            CastAnyPtr getPtr() const {return mem;}
        private:
            void* mem = 0;
    };
}
