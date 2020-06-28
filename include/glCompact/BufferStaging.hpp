#pragma once
#include "glCompact/BufferInterface.hpp"

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

            void* getMem() const {return mem;}
        private:
            void* mem = 0;
    };
}
