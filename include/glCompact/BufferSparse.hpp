#pragma once
#include "glCompact/BufferInterface.hpp"

#include <vector>

namespace glCompact {
    class BufferSparse;
    class BufferSparse : public BufferInterface {
        public:
            BufferSparse           () = default;
            BufferSparse           (bool clientMemoryCopyable, uintptr_t size);
            BufferSparse           (const BufferSparse&  bufferSparse);
            BufferSparse           (      BufferSparse&& bufferSparse);
            BufferSparse& operator=(const BufferSparse&  bufferSparse);
            BufferSparse& operator=(      BufferSparse&& bufferSparse);
            ~BufferSparse() = default;
            void free();

            void setCommitment(uintptr_t offset, uintptr_t size, bool commit);
            uintptr_t getCommitmentSize() const {return commitmentSize;}

            static constexpr uintptr_t pageSize = 65536;
        private:
            uintptr_t commitmentSize = 0;
            std::vector<bool> commitmentMap;

            void setCommitment_(uintptr_t offset, uintptr_t size, bool commit);
            void copyCommitment(const BufferSparse& bufferSparse);
            void copyFromBufferCommitmentRegionOnly(const BufferSparse& bufferSparse);
    };
}
