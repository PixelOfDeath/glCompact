#pragma once
#include "glCompact/BufferInterface.hpp"
#include "glCompact/CastAnyPtr.hpp"

namespace glCompact {
    class BufferCpu;
    class BufferCpu : public BufferInterface {
        public:
            BufferCpu           () = default;
            BufferCpu           (uintptr_t size);
            BufferCpu           (const BufferCpu&  buffer);
            BufferCpu           (      BufferCpu&& buffer);
            BufferCpu& operator=(const BufferCpu&  buffer);
            BufferCpu& operator=(      BufferCpu&& buffer);
            ~BufferCpu();
            void free();

            void flushMappedMemoryWrites();
            void flushMappedMemoryWrites(uintptr_t offset, uintptr_t size);

            CastAnyPtr getPtr() const {return mem;}
        private:
            void* mem = 0;
    };
}
