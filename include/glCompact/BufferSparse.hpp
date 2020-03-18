#pragma once
#include "glCompact/BufferInterface.hpp"

namespace glCompact {
    class BufferSparse;
    class BufferSparse : public BufferInterface {
        public:
            BufferSparse           () = default;
            BufferSparse           (bool clientMemoryCopyable, uintptr_t size);
            BufferSparse           (const BufferSparse&  buffer);
            BufferSparse           (      BufferSparse&& buffer);
            BufferSparse& operator=(const BufferSparse&  buffer);
            BufferSparse& operator=(      BufferSparse&& buffer);
            ~BufferSparse();
            void free();

            static uintptr_t getPageSize();
            void commitment(uintptr_t offset, uintptr_t size, bool commit);
        private:
            void create(bool clientMemoryCopyable, uintptr_t size);

            //uintptr_t commitedSize;
            //std::bitset commitMap //to store what pages are commited. So we always can accuratly calcualte what the current commit size is and also can copy this objects
            //uint64* commitMap;
            /*
                x86 only
                GCC?
                __builtin_popcount = int
                __builtin_popcountl = long int
                __builtin_popcountll = long long
            */
    };
}
